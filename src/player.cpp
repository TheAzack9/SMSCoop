#include <string.h>
#include <sdk.h>
#include <SMS/Player/Mario.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/Manager/ModelWaterManager.hxx>
#include <JSystem/JDrama/JDRGraphics.hxx>

#include <BetterSMS/player.hxx>
#include <BetterSMS/stage.hxx>

#include "characters.hxx"
#include "camera.hxx"

static u8 loadedMarios = 0;
static TMario* marios[2];

int getPlayerCount() {
	return loadedMarios;
}


// Description: Sets the global instance of mario and all access parameters
// Note: Cannot be set every other frame because game logic depends on one static global instance so swapping breaks the game.
// A lot of how mario works is connected to the global mario and must be manually changed to work with multiple players
void setActiveMario(int id) {
	TMario* mario = marios[id];
	gpMarioOriginal = mario;
	SMS_SetMarioAccessParams__Fv();
}

u8 getPlayerId(TMario* mario) {
	for(u8 i = 0; i < loadedMarios; ++i) {
		if(mario == marios[i]) return i;
	}
	return 0;
}

// Description: Overrides mario loading to allow for loading a luigi game object in level
// TODO: Make this optional based on setting
int TMarioStrCmp_Override(const char* nameRef, void* str) {
    if(strcmp(nameRef, "Luigi") == 0) {
        return 0;
    }

    return  strcmp(nameRef, "Mario");
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8029d7d8, 0, 0, 0), TMarioStrCmp_Override);

// Description: Override the controller update to ensure that correct mario is checked.
// Note: Certain things like the y-cam is for some reason tied directly to the controller update.
// IMPROVEMENT: I should just re-implement the entire loop at 0x80299a24 which is what calls this instead
void TMarioGamePad_updateMeaning_override(TMarioGamePad* gamepad) {
    TApplication* app = &gpApplication;
	// A bit of a jank way to check if it is player 2 sadly
	if(loadedMarios > 1 && gamepad == app->mGamePads[1]) {
        setActiveMario(1);
        setCamera(1);
	}
	updateMeaning__13TMarioGamePadFv(gamepad);
	if(loadedMarios > 1 && gamepad == app->mGamePads[1]) {
        setActiveMario(0);
        setCamera(0);
	}
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80299a84, 0, 0, 0), TMarioGamePad_updateMeaning_override);
SMS_PATCH_BL(SMS_PORT_REGION(0x802a6024, 0, 0, 0), TMarioGamePad_updateMeaning_override);

// Description: set correct camera and player before player update to make player move after correct camera
#define perform__6TMarioFUlPQ26JDrama9TGraphics         ((int (*)(...))0x8024D2A8)
void TMario_perform_coop(TMario* mario, u32 param_1, JDrama::TGraphics* param_2) {
	u8 playerId = getPlayerId(mario);
	setActiveMario(playerId);
	setCamera(playerId);
	perform__6TMarioFUlPQ26JDrama9TGraphics(mario, param_1, param_2);
	setActiveMario(0);
	setCamera(0);
}


static TMarioGamePad* GamePads = (TMarioGamePad*)0x8057738c;

// VTable for TMario
static u32* marioVTable = (u32*)0x803dd660;

// Description: Override load method for mario to set correct gamepad for p2 and load correct model
void loadMario(TMario* mario, JSUMemoryInputStream *input) {
	// Override vtable to get full controll over before and after update of player
	// TODO: This should be possible in plain better SMS with register callbacks, 
	// but since you can only register a callback before the update is it not currently possible to clean up state.
	marioVTable[8] = (u32)(&TMario_perform_coop);

    load__Q26JDrama6TActorFR20JSUMemoryInputStream(mario, input);
	
    marios[loadedMarios] = mario;

    if(loadedMarios == 1) {
        TApplication* app = &gpApplication;
		app->mGamePads[1]->_E0 = 2;
        mario->setGamePad(app->mGamePads[1]);
		mario->mController = app->mGamePads[1];
    }

    setActiveMarioArchive(loadedMarios);
	setActiveMario(0);
    loadedMarios++;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80276BF0, 0, 0, 0), loadMario);

// Description: A setup function on stage load to reset global fields per level
void setupPlayers(TMarDirector *director) {
    for(int i = 0; i < 2; ++i) {
        marios[loadedMarios] = nullptr;
    }
	loadedMarios = 0;
}

// Description: Sets initial fields on load for player and makes player active. 
// Note: This makes player spawn at level exits
// TODO: Fix wrong level exit animation
void SetMario(TMarDirector* director) {
	for (int i = loadedMarios-1; i >= 0; i--) {
		setActiveMario(i);
		director->setMario();
	}
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802983f8, 0, 0, 0), SetMario);
SMS_PATCH_BL(SMS_PORT_REGION(0x80298428, 0, 0, 0), SetMario);
SMS_PATCH_BL(SMS_PORT_REGION(0x802984d8, 0, 0, 0), SetMario);

// Description: Increases distance from gpMarioOriginal(player 1) where water can exist to a very high number. 
// This is to fix water for second player when they are far away from the "main" player
void TModelWaterManager_perform_move(TModelWaterManager* waterManager) {
	// OPTIMIZATION: Do this on load of water manager instead of every time it tries to move the water
	*(f32*)(((u32*)waterManager) + 0x5e08/4) = 1000000.0f;
	waterManager->move();
}
SMS_PATCH_BL(SMS_PORT_REGION(0x8027bedc, 0, 0, 0), TModelWaterManager_perform_move);

// Fix for luigi shadow
// Basically there is a check whether shadow has been rendered
// This removes that check
// Optimization: Check for shadow individually between players
SMS_WRITE_32(SMS_PORT_REGION(0x80231834, 0, 0, 0), 0x60000000);
