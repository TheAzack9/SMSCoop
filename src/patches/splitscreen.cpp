#include <sdk.h>
#include <types.h>
#define NTSCU
#include <raw_fn.hxx>

#include <SMS/System/Application.hxx>
#include <JSystem/JDrama/JDRGraphics.hxx>
#include <JSystem/JDrama/JDRRect.hxx>
#include <JSystem/JDrama/JDRViewport.hxx>
#include <JUtility/JUTTexture.hxx>
#include <Dolphin/GX.h>
#include <VI.h>

#include "camera.hxx"
#include "players.hxx"

// FIXME: Support horizontal split screen
// FIXME: 3/4 perspectives?
namespace SMSCoop {
    JDrama::TGraphics* graphicsPointer;
    JDrama::TViewport* gViewport;
    JDrama::TViewport* gScreen2DViewport;

    int perspective = 1;
    
    // Description: Set active viewport for player
    static void setViewport(int player) {
        perspective = player;
        if(player == 0) {
            gViewport->mViewportRect.mX1 = 0;
            gViewport->mViewportRect.mX2 = 320;
        } else {
            gViewport->mViewportRect.mX1 = 320;
            gViewport->mViewportRect.mX2 = 640;
        }
        gViewport->perform(0x88, graphicsPointer);
        
        if(gScreen2DViewport) {
            if(player == 0) {
                gScreen2DViewport->mViewportRect.mX1 = 0;
                gScreen2DViewport->mViewportRect.mX2 = 320;
            } else {
                gScreen2DViewport->mViewportRect.mX1 = 320;
                gScreen2DViewport->mViewportRect.mX2 = 640;
            }
            gScreen2DViewport->perform(0x88, graphicsPointer);
        }
        /*
        TMarDirector* director = (TMarDirector*)gpApplication.mDirector;
    constexpr static const char *screen2DKey ="\x53\x63\x72\x65\x65\x6e\x20\x32\x44";
        auto *nameref = TMarNameRefGen::getInstance()->getRootNameRef();
        u16 keycode = JDrama::TNameRef::calcKeyCode(screen2DKey);
        JDrama::TViewport* viewport2D = (JDrama::TViewport*)director->mPerformListGXPost->searchF(keycode, screen2DKey);
        OSReport("Screen 2D %X\n", director->mPerformListGXPost->searchF(keycode, screen2DKey));*/
    }

    int getActiveViewport() {
        return perspective;
    }

    // Description: Get's an instance of the screen viewport
    // FIXME: Get this from name ref instead
    static void TNameRefGe_getNameRef_createViewport(JDrama::TViewport* viewport, JDrama::TRect* rect, const char* name) {
        gViewport = viewport;
        __ct__Q26JDrama9TViewportFRCQ26JDrama5TRectPCc(viewport, rect, name);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802fb52c, 0, 0, 0), TNameRefGe_getNameRef_createViewport);
    
    // Description: Get's an instance of the screen viewport
    // FIXME: Get this from name ref instead
    static void Create_screen2D_Viewport_initECDisplay(JDrama::TViewport* viewport, JDrama::TRect* rect, const char* name) {
        gScreen2DViewport = viewport;
        __ct__Q26JDrama9TViewportFRCQ26JDrama5TRectPCc(viewport, rect, name);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8029b4b4, 0, 0, 0), Create_screen2D_Viewport_initECDisplay);
    
    // Description: Sets the viewport, mostly used to get graphics pointer pt
    // FIXME: Get Graphics pointer in a better way
    static void TViewport_perform_setViewport(JDrama::TGraphics* graphics, JDrama::TRect& rect, f32 far, f32 near) { 
        graphicsPointer = graphics;
        graphics->setViewport(rect, far, near);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802fcdc0, 0, 0, 0), TViewport_perform_setViewport);

    //void TViewport_Screen2D_Rect(JUTRect* rect) {
    //    rect->mY1 = 0;
    //    rect->mY2 = 448;
    //    if(perspective == 0) {
    //        rect->mX1 = 0;
    //        rect->mX2 = 320;
    //    } else {
    //        rect->mX1 = 320;
    //        rect->mX2 = 640;
    //    }
    //}
    //SMS_PATCH_BL(SMS_PORT_REGION(0x8029b4a4, 0, 0, 0), TViewport_Screen2D_Rect);

    // TODO: Rewrite this override EfbCtrlTex_perform instead and store a reference to the EfbCtrlTex that should
    // be changed to correct size. Checking the width is very hacky
    static void setTexCopySrcEfbTex(u16 left, u16 top, u16 wd, u16 ht) {
        //OSReport("TexCopySrcEFB SHIT %d %d %d %d\n", left, top, wd, ht);

        if(wd == 640 && perspective == 0) {
            left = 0;
            wd = 320;
        }
        if(wd == 640 && perspective == 1) {
            left = 320;
            wd = 320;
        }
        GXSetTexCopySrc(left, top, wd, ht);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802f8d48, 0, 0, 0), setTexCopySrcEfbTex);

    // Description: Override TScreenTexture to be half the width for split screen
    // Note: ScreenTexture is already half size for memory optimization? (Done in sunshine)
    static void TScreenTexture_load_ct_JUTTexture(JUTTexture* texture, int width, int height, u32 fmt) {
        __ct__10JUTTextureFii9_GXTexFmt(texture, 640/2 / 2, 448/2, fmt);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8022d4d0, 0, 0, 0), TScreenTexture_load_ct_JUTTexture);

    // Trying to figure out what each perform flag does
    // TViewObj also has dynamic check with 0xc (display flag)
    // 0x1 = update
    // 0x2 = update 2? (or ui update?)
    // 0x4 = Some kind of FX?
    // 0x8 = draw?
    // 0x10 = after effects
    // 0x14 = camera
    // 0x20 = colors / lights
    // 0x88 = viewport
    // 0x80 = efb textures (/ draw buffers)
    // 0x200 = ? (TSmJ3dAct + M3UModel)  (crashes when run twice)
    // 0x400 = DrawBufObj setting global flags
    // 0x1000 = TViewObj filteR?
    // 0x2000 = TViewObj filteR?
    // 0x1000000 = = (TMario)
    // 0x2000000 = = (TMario) + related to updating goop stamping
    // 0x4000000 = = (TMario)
    // 0x8000000 = = (TMario)
    // 0x10000000 = = (TMario)
    // 0x40000000 = = (TMario)
    // 0x20000000 = = (TMario)
    // 0x80000000 = = (TMario)
    
    // Description: Before doing GXInvalidate, render other players perspective
    static void processGXInvalidateTexAll() { 
        //OSReport("---------------------\n");
        //OSReport("Starting p2 screen \n");
        TMarDirector* director = (TMarDirector*)gpApplication.mDirector;
        GXInvalidateTexAll(); 
        
        
        
        setViewport(0);
        setCamera(0);
        setActiveMario(0);

        //VIWaitForRetrace();

        u32 flagsNoUpdate = 0xffffffff & ~0x1 & ~0x200;
        

        //director->mPerformListUIElements->testPerform(0xffffffff, graphicsPointer);
        director->mPerformListPreDraw->perform(0xffffffff, graphicsPointer);
        director->mPerformListPostDraw->perform(flagsNoUpdate, graphicsPointer);
        u32 flags = 0x1 | 0x2000000; // Update + goop stamp (freezes with goop stamps otherwise :c)
        director->mPerformListUnk1->perform(flags, graphicsPointer);
        director->mPerformListUnk2->perform(flags, graphicsPointer);
        director->mPerformListGX->perform(flagsNoUpdate, graphicsPointer);
        // TODO, based on some conditions
        director->mPerformListSilhouette->testPerform(0xffffffff, graphicsPointer);
        director->mPerformListGXPost->perform(flagsNoUpdate, graphicsPointer);

        setCamera(1);
        setActiveMario(1);
        setViewport(1);
        
        
        GXInvalidateTexAll(); 
       /* OSReport("Ending p2 screen \n");
        OSReport("---------------------\n");
        OSReport("Content of GXPost\n");
        OSReport("Testing %s \n", director->mPerformListGXPost->mKeyName);
        for(JGadget::TSingleNodeLinkList::iterator begin = director->mPerformListGXPost->begin(); 
            begin != director->mPerformListGXPost->end(); ) {
            
            JDrama::TViewObj* obj = (JDrama::TViewObj*)begin->mData;
            OSReport("   Child: '%s' \n", obj->mKeyName);

            begin = begin->mNext;
        }
        OSReport("---------------------\n");*/

    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80299d04, 0, 0, 0), processGXInvalidateTexAll);

    //static void performListUiElements(TPerformList* performList, u32 flags, JDrama::TGraphics* graphics) {
    //    OSReport("Rendering ui \n");
    //    //performList->testPerform(flags, graphics);
    //}
    //// Disable ui, we render this custom later
    //SMS_PATCH_BL(SMS_PORT_REGION(0x80299b98, 0, 0, 0), performListUiElements);

    static void performListUiElements2(TPerformList* performList, u32 flags, JDrama::TGraphics* graphics) {
        /*TMarDirector* director = (TMarDirector*)gpApplication.mDirector;
        director->mPerformListUIElements->perform(~0x2, graphicsPointer);
        OSReport("Drawing p1 stuff %X \n");*/
        performList->testPerform(flags, graphics);
    }
    //// Disable ui, we render this custom later
    //SMS_PATCH_BL(SMS_PORT_REGION(0x80299c48, 0, 0, 0), performListUiElements2);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80299d00, 0, 0, 0), performListUiElements2);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x80299d00, 0, 0, 0), performListUiElements);
    //
    //static void performListUiElementss(TPerformList* performList, u32 flags, JDrama::TGraphics* graphics) {
    //    OSReport("PRE DRAW FLAG %X \n", flags);
    //    performList->testPerform(flags, graphics);

    //    
    //    
    //    TMarDirector* director = (TMarDirector*)gpApplication.mDirector;
    //    
    //    for(int i = 0; i < 4; ++i) {
    //        u32 flags = 0;
    //        if(i == 3) {
    //            flags = 2;
    //        }
    //        director->mPerformListUIElements->testPerform(~flags, graphicsPointer);
    //    }
    //}
    //// Disable ui, we render this custom later
    //SMS_PATCH_BL(SMS_PORT_REGION(0x80299c20, 0, 0, 0), performListUiElementss);

}