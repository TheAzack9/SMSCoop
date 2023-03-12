/// <summary>
/// Overrides logic for rendering two perspectives to the screen.
/// Thanks to Mr. Brocoli for quite a bit of this code.
/// </summary>

#include <Dolphin/GX.h>
#include <macros.h>
#include <sdk.h>
#include <BetterSMS/stage.hxx>

#include "player.hxx"

static bool IS_SPLITSCREEN_ENABLED = true;
static int ACTIVE_PERSPECTIVE = 0;
static bool IS_VERTICAL_SPLIT = true;

// TODO: Get rid of hard pointers, i think these are in MarDirector
static u8* MapEpisodePost = (u8*)0x803e970f;
static u8* ThpState = (u8*)0x803ec204;
static u8* isThpInit = (u8*)0x803ec206;

static u8* MapArea = (u8*)0x803e970e;

// Description: Checks whether to enable split screen or not
// TODO: Cleanup, probably don't need to check all of this
static bool isSplitscreen() {
    TApplication *app      = &gpApplication;
    TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

    if (app == nullptr || app->mContext != TApplication::CONTEXT_DIRECT_STAGE || gpMarioAddress == nullptr) {
        ACTIVE_PERSPECTIVE = 0;
        return false;
    }

    if (director == nullptr || director->mCurState == TMarDirector::STATE_INTRO_INIT) {
        ACTIVE_PERSPECTIVE = 0;
        return false;
    }
    

	if(*ThpState == 2 && *isThpInit == 0) {
        ACTIVE_PERSPECTIVE = 0;
		return false;
	}

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
		if (IS_VERTICAL_SPLIT) unk += 160;
		else unk += 0x46000 / 4;
	}
    
    GXCopyDisp(unk, one);
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802f92f8, 0, 0, 0), processGXCopyDisp);

// Description: Flips which perspective is active. Should probably not happen here.
static void processGXInvalidateTexAll() { 
    if (isSplitscreen()) {
        ACTIVE_PERSPECTIVE ^= 1;
    } else {
        ACTIVE_PERSPECTIVE = 0;
    }

    GXInvalidateTexAll(); 
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80299d04, 0, 0, 0), processGXInvalidateTexAll);

// Description: Replaces branch that moves EFB (Embedded frabe buffer) to XFB (External frame buffer).
// TL;DR: It copies from memory to screen
// Reason: We must offset the destination such that on one frame it renders P1's perspective and the next it renders P2's perspective
static void processGXSetDispCopySrc(u16 param_1, u16 param_2, u16 param_3, u16 param_4) {
    if (isSplitscreen()) {
        if(IS_VERTICAL_SPLIT) {
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
	if(IS_VERTICAL_SPLIT) {
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
	if(!IS_VERTICAL_SPLIT) {
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
