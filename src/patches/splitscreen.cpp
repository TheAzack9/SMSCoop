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
#include <SMS/Camera/SunModel.hxx>

#include "camera.hxx"
#include "players.hxx"
#include "talking.hxx"

// FIXME: Support horizontal split screen
// FIXME: 3/4 perspectives?
namespace SMSCoop {
    JDrama::TGraphics* graphicsPointer;
    JDrama::TViewport* gViewport;
    JDrama::TViewport* gScreen2DViewport;

    void* g_sun;
    TSunModel* g_sunModel;
    void* g_sunLensFlare;
    void* g_sunLensGlow;

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

    }

    int getActiveViewport() {
        return perspective;
    }

    // Description: Get's an instance of the 3d viewport
    // FIXME: Get this from name ref instead
    static void TNameRefGe_getNameRef_createViewport(JDrama::TViewport* viewport, JDrama::TRect* rect, const char* name) {
        gViewport = viewport;
        __ct__Q26JDrama9TViewportFRCQ26JDrama5TRectPCc(viewport, rect, name);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802fb52c, 0, 0, 0), TNameRefGe_getNameRef_createViewport);
    
    // Description: Get's an instance of the ui viewport
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
        graphicsPointer->mViewPortSpace.mX1 = 0;
        graphicsPointer->mViewPortSpace.mX2 = 640;
        graphicsPointer->mViewPortSpace.mY1 = 0;
        graphicsPointer->mViewPortSpace.mY2 = 448;
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802fcdc0, 0, 0, 0), TViewport_perform_setViewport);

    // TODO: Rewrite this override EfbCtrlTex_perform instead and store a reference to the EfbCtrlTex that should
    // be changed to correct size. Checking the width is very hacky
    static void setTexCopySrcEfbTex(u16 left, u16 top, u16 wd, u16 ht) {
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
        setActiveMario(0);
        setCamera(0);

        VIWaitForRetrace();

        u32 flagsNoUpdate = 0xffffffff;
        // TODO: Create a custom perform list of things that must update before drawing on p2 screen
        if(g_sun) {
            perform__7TSunMgrFUlPQ26JDrama9TGraphics(g_sun, 0x7, graphicsPointer);
        }

        TMarDirector_movement_game_override(director);
        
        director->mPerformListPreDraw->perform(0xffffffff, graphicsPointer);
        director->mPerformListPostDraw->perform(flagsNoUpdate, graphicsPointer);
        
        if(g_sunModel) {
            calcDispRatioAndScreenPos___9TSunModelFv(g_sunModel);
            //g_sunModel->getZBufValue();
            perform__9TSunModelFUlPQ26JDrama9TGraphics(g_sunModel, 0x7, graphicsPointer);

        }
        if(g_sunLensGlow) {
            perform__9TLensGlowFUlPQ26JDrama9TGraphics(g_sunLensGlow, 0x7, graphicsPointer);
        }
        if(g_sunLensFlare) {
            perform__10TLensFlareFUlPQ26JDrama9TGraphics(g_sunLensFlare, 0x7, graphicsPointer);
        }

        // Update + goop stamp (game freezes with goop stamps otherwise :c)
        director->mPerformListUnk1->perform(0x1000000, graphicsPointer); // Need to do some preparation for goop maps
        director->mPerformListUnk2->perform(0x20f0000, graphicsPointer); // Unsure exactly what happens, but this works without drawing a black square to the screen
        director->mPerformListGX->perform(flagsNoUpdate, graphicsPointer);
        // FIXME should be based on some conditions, check direct in MarDirector
        director->mPerformListSilhouette->testPerform(0xffffffff, graphicsPointer);
        director->mPerformListGXPost->perform(flagsNoUpdate, graphicsPointer);

        setCamera(1);
        setActiveMario(1);
        setViewport(1);
        
        
        GXInvalidateTexAll(); 
        OSReport("Ending p2 screen \n");
        OSReport("---------------------\n");
        OSReport("Content of GXPost\n");
        OSReport("Testing %s \n", director->mPerformListPostDraw->mKeyName);
        for(JGadget::TSingleNodeLinkList::iterator begin = director->mPerformListPostDraw->begin(); 
            begin != director->mPerformListPostDraw->end(); ) {
            
            JDrama::TViewObj* obj = (JDrama::TViewObj*)begin->mData;
            OSReport("   Child: '%s' \n", obj->mKeyName);

            begin = begin->mNext;
        }
        OSReport("---------------------\n");

    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80299d04, 0, 0, 0), processGXInvalidateTexAll);

    // Bowserfight rendering
    // This is all custom, which is very annoying

    // Description: 
    // thusly we need to handle this shit manually...
    void GXSetViewport_bathwater(f32 xOrig,f32 yOrig,f32 wd,f32 ht,f32 nearZ,f32 farZ) {
        f32 xOffset = xOrig;
        if(perspective == 1) {
            xOffset = 320.0f;
        }

        GXSetViewport(xOffset, yOrig, 640.0f / 2.0f, ht, nearZ, farZ);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x801ab0b4, 0, 0, 0), GXSetViewport_bathwater);

    // Description: Set source copy when copying bathwater from efb to screentexture
    // We copy the part of the texture that the active viewport is rendering
    void GXSetTexCopySrc_bathwater(u16 left, u16 top,u16 wd,u16 ht) {
        u16 xOffset = left;
        if(perspective == 1) {
            xOffset = 320;
        }

        GXSetTexCopySrc(xOffset, top, wd, ht);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x801acd98, 0, 0, 0), GXSetTexCopySrc_bathwater);

    static unsigned int SMSGetGameRenderWidth_320() { 
		    return 320;
    }
    // Description: Sets render width for bathwater EFB
    SMS_PATCH_BL(SMS_PORT_REGION(0x801ac970, 0, 0, 0), SMSGetGameRenderWidth_320);
    
    // Description: Set screen area for mist and bathwater reflection to render
    void draw_mist(u32 x, u32 y, u32 wd, u32 ht, u32 unk) {
        u32 xOffset = x;
        if(perspective == 1) {
            xOffset = 320;
        }
        draw_mist__FUsUsUsUsPv(xOffset, y, wd/2, ht, unk);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x801ad6a8, 0, 0, 0), draw_mist);

    // Manta rendering
    
    SMS_PATCH_BL(SMS_PORT_REGION(0x80110128, 0, 0, 0), GXSetTexCopySrc_bathwater);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80110114, 0, 0, 0), SMSGetGameRenderWidth_320);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80110134, 0, 0, 0), SMSGetGameRenderWidth_320);
    SMS_PATCH_BL(SMS_PORT_REGION(0x801101e4, 0, 0, 0), SMSGetGameRenderWidth_320);

    // Fix sun
    void GXPeekZ_override(u16 x, u16 y, u32* z) {
        int offsetX = x / 2;
        if(perspective == 1) {
            offsetX += 320;
        }
        GXPeekZ(offsetX, y, z);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8002eacc, 0, 0, 0), GXPeekZ_override);

    void TSunMgr_perform_override(void* sunMgr, u32 performFlags, JDrama::TGraphics* graphics) {
        g_sun = sunMgr;
        perform__7TSunMgrFUlPQ26JDrama9TGraphics(sunMgr, performFlags, graphics);
    }
	SMS_WRITE_32(SMS_PORT_REGION(0x803ad180, 0, 0, 0), TSunMgr_perform_override);
    void TSunModel_perform_override(TSunModel* sunModel, u32 performFlags, JDrama::TGraphics* graphics) {
        g_sunModel = sunModel;
        calcDispRatioAndScreenPos___9TSunModelFv(g_sunModel);
        g_sunModel->getZBufValue();
        perform__9TSunModelFUlPQ26JDrama9TGraphics(sunModel, performFlags, graphics);
    }
	SMS_WRITE_32(SMS_PORT_REGION(0x803ad1c0, 0, 0, 0), TSunModel_perform_override);
    void TLensGlow_perform_override(void* lensGlow, u32 performFlags, JDrama::TGraphics* graphics) {
        g_sunLensGlow = lensGlow;
        perform__9TLensGlowFUlPQ26JDrama9TGraphics(lensGlow, performFlags, graphics);
    }
	SMS_WRITE_32(SMS_PORT_REGION(0x803ad150, 0, 0, 0), TLensGlow_perform_override);
    void TLensFlare_perform_override(void* lensFlare, u32 performFlags, JDrama::TGraphics* graphics) {
        g_sunLensFlare = lensFlare;
        perform__10TLensFlareFUlPQ26JDrama9TGraphics(lensFlare, performFlags, graphics);
    }
	SMS_WRITE_32(SMS_PORT_REGION(0x803ad128, 0, 0, 0), TLensFlare_perform_override);
}