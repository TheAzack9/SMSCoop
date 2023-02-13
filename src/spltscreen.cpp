#include <Dolphin/GX.h>
#include <macros.h>
#include <sdk.h>
#include <BetterSMS/stage.hxx>

static bool IS_SPLITSCREEN_ENABLED = true;
static int ACTIVE_PERSPECTIVE = 0;


static bool isSplitscreen() {
    TApplication *app      = &gpApplication;
    TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

    if (app->mContext != TApplication::CONTEXT_DIRECT_STAGE || !gpMarioAddress)
        return false;

    if (director->mCurState == TMarDirector::STATE_INTRO_INIT) {
        return false;
    }

    if (director->mAreaID == 15)
        return false;
    return IS_SPLITSCREEN_ENABLED;
}

// Description: Replaces branch that moves EFB (Embedded frame buffer) to XFB (External frame
// buffer). TL;DR: It copies from memory to screen Reason: We must offset the destination such that
// on one frame it renders P1's perspective and the next it renders P2's perspective
static void processGXCopyDisp(int *unk, char one) {
    // TODO: Fix horizontalness and verticalness
    /* if (t2 == shouldFlip()) {
        if (isHorizontal == 1)
            unk += 0x46000 / 4;
        else
            unk += 160;
    }*/

    if (ACTIVE_PERSPECTIVE == 0 && isSplitscreen()) {
        unk += 160;
    }
    
    GXCopyDisp(unk, one);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802f92f8, 0, 0, 0), processGXCopyDisp);

static void processGXInvalidateTexAll() { 
    if (isSplitscreen()) {
        ACTIVE_PERSPECTIVE ^= 1;
        
    }
    GXInvalidateTexAll(); 
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80299d04, 0, 0, 0), processGXInvalidateTexAll);

static void processGXSetDispCopySrc(u16 param_1, u16 param_2, u16 param_3, u16 param_4) {
    if (isSplitscreen()) {
        param_3 = 320;
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
unsigned int SMSGetGameRenderWidth() { return 320; }
SMS_PATCH_B(SMS_PORT_REGION(0x802a8bd0, 0, 0, 0), SMSGetGameRenderWidth);

unsigned int SMSGetGameRenderWidth2() { return 640; }
SMS_PATCH_BL(SMS_PORT_REGION(0x802a50c0, 0, 0, 0), SMSGetGameRenderWidth2);
SMS_PATCH_BL(SMS_PORT_REGION(0x802a4f68, 0, 0, 0), SMSGetGameRenderWidth2);
SMS_PATCH_BL(SMS_PORT_REGION(0x802b6e1c, 0, 0, 0), SMSGetGameRenderWidth2);
SMS_PATCH_BL(SMS_PORT_REGION(0x802a6604, 0, 0, 0), SMSGetGameRenderWidth2);
SMS_PATCH_BL(SMS_PORT_REGION(0x802a6528, 0, 0, 0), SMSGetGameRenderWidth2);
SMS_PATCH_BL(SMS_PORT_REGION(0x802a64c0, 0, 0, 0), SMSGetGameRenderWidth2);

unsigned int SMSGetGameVideoWidth() { return 660; }
SMS_PATCH_B(SMS_PORT_REGION(0x802a8ca8, 0, 0, 0), SMSGetGameVideoWidth);