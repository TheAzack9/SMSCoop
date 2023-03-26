/// <summary>
/// Overrides logic for rendering two perspectives to the screen.
/// Thanks to Mr. Brocoli for quite a bit of this code.
/// </summary>

#include <Dolphin/GX.h>
#include <Dolphin/VI.h>
#include <macros.h>
#include <sdk.h>
#include <raw_fn.hxx>
#include <BetterSMS/stage.hxx>
#include <BetterSMS/loading.hxx>
#include <BetterSMS/settings.hxx>

#include "player.hxx"
#include "camera.hxx"
#include "settings.hxx"

//#define EXPERIMENTAL_RENDERING

extern SMSCoop::SplitScreenSetting gSplitScreenSetting;

namespace SMSCoop {
    static bool IS_SPLITSCREEN_ENABLED = true;
    static int ACTIVE_PERSPECTIVE = 0;

    // TODO: Get rid of hard pointers, i think these are in MarDirector
    static u8* MapEpisodePost = (u8*)0x803e970f;
    static u8* ThpState = (u8*)0x803ec204;
    static u8* isThpInit = (u8*)0x803ec206;

    static u8* MapArea = (u8*)0x803e970e;

    bool g_isLoading = false;

    void setLoading(bool isLoading) {
        g_isLoading = isLoading; 
    }

    bool isVerticalSplit() {
        return gSplitScreenSetting.getInt() == SplitScreenSetting::VERTICAL;
    }


    // Description: Checks whether to enable split screen or not
    // TODO: Cleanup, probably don't need to check all of this
    bool isSplitscreen() {
        TApplication *app      = &gpApplication;
        TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

        if(g_isLoading || gSplitScreenSetting.getInt() == SplitScreenSetting::NONE) {
            ACTIVE_PERSPECTIVE = 0;
            return false;
        }

        if (app == nullptr || app->mContext != TApplication::CONTEXT_DIRECT_STAGE ) {
            ACTIVE_PERSPECTIVE = 0;
            return false;
        }

        if (director == nullptr || (director->mCurState == TMarDirector::STATE_GAME_STARTING && getPlayerCount() <= 1)) {
            ACTIVE_PERSPECTIVE = 0;
            return false;
        }

	    if(*ThpState == 2 && *isThpInit == 0) {
            ACTIVE_PERSPECTIVE = 0;
		    return false;
	    }

        // intro / option level
        if (director->mAreaID == 15) {
            ACTIVE_PERSPECTIVE = 0;
            return false;
        }

    
	    if(*MapArea == 0x0f) {
            ACTIVE_PERSPECTIVE = 0;
		    return false;
	    }

	    if(*MapEpisodePost == 0xff) {
            ACTIVE_PERSPECTIVE = 0;
		    return false;
	    }

        return IS_SPLITSCREEN_ENABLED;
    }

    // Description: Checks whether to split the view or not. This is to fix e.g shine select
    // TODO: Cleanup, probably don't need to check all of this
    int shouldFlip() {
        return false;
	    if(*MapEpisodePost == 0xff) {
		    ACTIVE_PERSPECTIVE = 0;
		    return true;
	    }

	    if(*ThpState == 2 && *isThpInit == 0) {
		    ACTIVE_PERSPECTIVE = 0;
		    return true;
	    }

	    return !isSplitscreen();
    }

    // Description: Get's the player id of current perspectve
    int getActivePerspective() {
        if(ACTIVE_PERSPECTIVE && isSplitscreen() && getPlayerCount() > 1) return 1;
        return 0;
    }

    // Description: Replaces branch that moves EFB (Embedded frame buffer) to XFB (External frame
    // buffer). TL;DR: It copies from memory to screen Reason: We must offset the destination such that
    // on one frame it renders P1's perspective and the next it renders P2's perspective
    static void processGXCopyDisp(int *unk, char one) {
	    if (isSplitscreen() && ACTIVE_PERSPECTIVE == shouldFlip()) {
		    if (isVerticalSplit()) unk += 160;
		    else unk += 0x46000 / 4;
	    }
    
        GXCopyDisp(unk, one);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802f92f8, 0, 0, 0), processGXCopyDisp);

    #ifdef EXPERIMENTAL_RENDERING
    bool doRetrace = true;
    #define VIGetRetraceCount_TEST         ((int (*)(...))0x803504EC)
    void waitForRetraceOverride(JDrama::TVideo* video, u16 param_1) {
        //OSReport("RETRACE_COUNT %d wut %d Wut2 %d\n", VIGetRetraceCount_TEST(), video->mRetraceCount, param_1);
            //video->mRetraceCount = VIGetRetraceCount();
        if(doRetrace) {
            waitForRetrace__Q26JDrama6TVideoFUs(video, param_1);
            //VIWaitForRetrace();
        } else {
            //waitForRetrace__Q26JDrama6TVideoFUs(video, param_1);
        }
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802f80ec, 0, 0, 0), waitForRetraceOverride);



    void gxFlushOverride(JDrama::TVideo* video, u16 param_1) {
        if(doRetrace) {
            GXFlush();
        }
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802f8148, 0, 0, 0), gxFlushOverride);

    #endif
    JDrama::TGraphics* graphicsPointer;
    void setGraphics(JDrama::TGraphics* graphics) {
        graphicsPointer = graphics;
    }

    #ifndef EXPERIMENTAL_RENDERING
    // Description: This fixes some objects not being finished rendering on the correct frame. We basically just move the vsync to happen at the end of the game loop
    // instead of when displaying the result. This even fixes a minor rendering glitch in sunshine that no casual would ever notice lmao
    static bool hasRetraced = false;
    void waitForRetraceOverride(JDrama::TVideo* video, u16 param_1) {
        if(!hasRetraced) waitForRetrace__Q26JDrama6TVideoFUs(video, param_1);
        hasRetraced = false;
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802f80ec, 0, 0, 0), waitForRetraceOverride);
    #endif

    // Description: Flips which perspective is active. Should probably not happen here.
    static void processGXInvalidateTexAll() { 
        if (isSplitscreen()) {
            ACTIVE_PERSPECTIVE ^= 1;
    #ifndef EXPERIMENTAL_RENDERING
            waitForRetrace__Q26JDrama6TVideoFUs(gpApplication.mDisplay->mVideo, gpApplication.mDisplay->mRetraceCount);
            hasRetraced = true;
            //GXInvalidateTexAll(); 
    #else
            doRetrace = true;
            //OSThread * gxCurrentThread = OSGetCurrentThread();

        
            //OSDisableInterrupts();
            ////OSSleepThread(&retraceQueue);
            //OSRestoreInterrupts(true);
            /*VIFlush();
            VIWaitForRetrace();*/
            //VIWaitForRetrace();
            //VIFlush();
            //VIWaitForRetrace();
            //VISetNextFrameBuffer(gpApplication.mDisplay->mVideo->mNextFB);
            //VISetBlack(0);
            //gpApplication.mDisplay->mRetraceCount = 1;
            //OSSleepThread(&gxCurrentThread->mQueueJoin);
            gpApplication.mDisplay->endRendering();
            gpApplication.mDisplay->startRendering();

            doRetrace = false;
            /*setCameraTemp(1);
            setActiveMario(1);*/
            CPolarSubCamera* camera = getCameraById(1);
            TMarDirector* director = (TMarDirector*)gpApplication.mDirector;
            //PSMTXCopy(camera->mMatrixTRS, *(Mtx*)((u32)graphicsPointer + 0xb4));
        
            //director->mPerformListCalcAnim->testPerform(0x1, graphicsPointer);
            director->mPerformListUIElements->testPerform(0xffffffff & ~2, graphicsPointer);
            director->mPerformListPreDraw->testPerform(0xFFFFFFFf , graphicsPointer);
            director->mPerformListPostDraw->testPerform(0xffffffff, graphicsPointer);
            director->mPerformListUnk1->testPerform(0xffffffff, graphicsPointer);
            director->mPerformListUnk2->testPerform(0xffffffff, graphicsPointer);
            director->mPerformListGX->testPerform(0xffffffff, graphicsPointer);
            // Unsure what this draws (underground?)
            director->mPerformListSilhouette->testPerform(0xffffffff, graphicsPointer);
            director->mPerformListGXPost->testPerform(0xffffffff, graphicsPointer);
            //setCameraTemp(0);
            //setActiveMario(0);
            ///* silhouette
		          //  if ((dVar12 < (double)*(float *)(MarioUtil::gpSilhouetteManager + 0x48)) ||
            //(*(short *)&Camera::gpCamera->field_0x2c8 != -1)) {
            //(**(code **)(*this->field23_0x20 + 0x20))(this->field23_0x20,0xffffffff,local_140);
            //}
            //*/
            //director->mPerformListGXPost->testPerform(0xffffffff, graphicsPointer);
            //if(marDirector->mGameState & 0x4000) {
            //	return;
            //}
            //OSReport("PERFORM MARIO %X\n", marDirector->mGameState);
            ACTIVE_PERSPECTIVE ^= 1;
    #endif
        } else {
            ACTIVE_PERSPECTIVE = 0;
        }
    
        GXInvalidateTexAll(); 
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80299d04, 0, 0, 0), processGXInvalidateTexAll);

    #ifdef EXPERIMENTAL_RENDERING
    int ViewFrustumClipCheck(double param_1, JDrama::TGraphics* param_2, TVec3f* param_3) {
        return 1;
        int theResult = ViewFrustumClipCheck__FPQ26JDrama9TGraphicsP3Vecf(param_1, param_2, param_3);
        if(theResult) return theResult;
        for(int i = 0; i < getPlayerCount(); ++i) {
            CPolarSubCamera* camera = getCameraById(i);
            SetViewFrustumClipCheckPerspective__Fffff(camera->mProjectionFovy, camera->mProjectionAspect, 10.0, 100000.0);
            PSMTXCopy(camera->mMatrixTRS, *(Mtx*)((u32)param_2 + 0xb4));
        
            theResult  = ViewFrustumClipCheck__FPQ26JDrama9TGraphicsP3Vecf(param_1, param_2, param_3);
            if(theResult) break;

        }
    
        CPolarSubCamera* camera = getCameraById(0);
        PSMTXCopy(camera->mMatrixTRS, *(Mtx*)((u32)param_2 + 0xb4));
        return theResult;
    }

    SMS_PATCH_BL(SMS_PORT_REGION(0x8000752c, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80007684, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80009eb8, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80034078, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x8003d0e4, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x8004a274, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x8004a2cc, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80069368, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x800693c0, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80069438, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x8007fdb4, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x8007fe0c, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80135c30, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x8020a320, 0, 0, 0), ViewFrustumClipCheck);
    SMS_PATCH_BL(SMS_PORT_REGION(0x8021b144, 0, 0, 0), ViewFrustumClipCheck);
    #endif

    // Description: Replaces branch that moves EFB (Embedded frabe buffer) to XFB (External frame buffer).
    // TL;DR: It copies from memory to screen
    // Reason: We must offset the destination such that on one frame it renders P1's perspective and the next it renders P2's perspective
    static void processGXSetDispCopySrc(u16 param_1, u16 param_2, u16 param_3, u16 param_4) {
        if (isSplitscreen()) {
            if(isVerticalSplit()) {
                param_3 = 320;
            } else {
                param_4 = 224;
            }
        }

        GXSetDispCopySrc(param_1, param_2, param_3, param_4);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802f92cc, 0, 0, 0), processGXSetDispCopySrc);

    // Description: RenderWidth of the SMS screen
    // We need to Update this to half width in order for shaders to render properly
    // This is due to some shaders using the previous frame and thusly this value to render
    // NOTE: This is only called when creating the textures and must be changed before level load
    // FIXME: Switch between full screen and half screen when in level vs cutscenes.

    // TODO: Fix widescreen bug here
    static unsigned int SMSGetGameRenderWidth() { 
	    if(isSplitscreen() && isVerticalSplit()) {
		    return 320;
	    }
	    return 640;
    }
    SMS_PATCH_B(SMS_PORT_REGION(0x802a8bd0, 0, 0, 0), SMSGetGameRenderWidth);

    static unsigned int SMSGetGameRenderWidth2() { 
        return 640;
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802a50c0, 0, 0, 0), SMSGetGameRenderWidth2);
    SMS_PATCH_BL(SMS_PORT_REGION(0x802a4f68, 0, 0, 0), SMSGetGameRenderWidth2);
    SMS_PATCH_BL(SMS_PORT_REGION(0x802b6e1c, 0, 0, 0), SMSGetGameRenderWidth2);
    SMS_PATCH_BL(SMS_PORT_REGION(0x802a6604, 0, 0, 0), SMSGetGameRenderWidth2);
    SMS_PATCH_BL(SMS_PORT_REGION(0x802a6528, 0, 0, 0), SMSGetGameRenderWidth2);
    SMS_PATCH_BL(SMS_PORT_REGION(0x802a64c0, 0, 0, 0), SMSGetGameRenderWidth2);

    static unsigned int SMSGetGameVideoWidth() { 
        return 660;
    }
    SMS_PATCH_B(SMS_PORT_REGION(0x802a8ca8, 0, 0, 0), SMSGetGameVideoWidth);


    static unsigned int SMSGetGameRenderHeight() {
	    if(isSplitscreen() && !isVerticalSplit()) {
		    return 224;
	    }
	    return 448;
    }
    SMS_PATCH_B(SMS_PORT_REGION(0x802a8bc8, 0, 0, 0), SMSGetGameRenderHeight);

    static unsigned int SMSGetGameRenderHeight2() {
	    return 448;
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802a50c8, 0, 0, 0), SMSGetGameRenderHeight2);
    SMS_PATCH_BL(SMS_PORT_REGION(0x802a4f70, 0, 0, 0), SMSGetGameRenderHeight2);
    SMS_PATCH_BL(SMS_PORT_REGION(0x802b6e08, 0, 0, 0), SMSGetGameRenderHeight2);
    SMS_PATCH_BL(SMS_PORT_REGION(0x802a65fc, 0, 0, 0), SMSGetGameRenderHeight2);
    SMS_PATCH_BL(SMS_PORT_REGION(0x802a6520, 0, 0, 0), SMSGetGameRenderHeight2);
    SMS_PATCH_BL(SMS_PORT_REGION(0x802a64b8, 0, 0, 0), SMSGetGameRenderHeight2);
}