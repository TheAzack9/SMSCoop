#include <string.h>
#include <sdk.h>
#include <SMS/Player/Mario.hxx>
#include <SMS/System/Application.hxx>

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
static TMarioGamePad* GamePadTwo = (TMarioGamePad*)0x8057748c;
void TMarioGamePad_updateMeaning_override(TMarioGamePad* gamepad) {
	// A bit of a jank way to check if it is player 2 sadly
	if(loadedMarios > 1 && gamepad == GamePadTwo) {
        setActiveMario(1);
        setCamera(1);
	}
	updateMeaning__13TMarioGamePadFv(gamepad);
	if(loadedMarios > 1 && gamepad == GamePadTwo) {
        setActiveMario(0);
        setCamera(0);
	}
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80299a84, 0, 0, 0), TMarioGamePad_updateMeaning_override);
SMS_PATCH_BL(SMS_PORT_REGION(0x802a6024, 0, 0, 0), TMarioGamePad_updateMeaning_override);

// Description: set correct camera before player update to make player move after correct camera
void updatePlayerCoop(TMario *player, bool isRealMario) {
	u8 playerId = getPlayerId(player);
	setCamera(playerId);
}

static TMarioGamePad* GamePads = (TMarioGamePad*)0x8057738c;

// Description: Override load method for mario to set correct gamepad for p2 and load correct model
void loadMario(TMario* mario, JSUMemoryInputStream *input) {
    load__Q26JDrama6TActorFR20JSUMemoryInputStream(mario, input);
	
    marios[loadedMarios] = mario;

    if(loadedMarios == 1) {
        TApplication* app = &gpApplication;
		app->mGamePads[1]->_E0 = 2;
        mario->setGamePad(app->mGamePads[1]);
    }

    setActiveMarioArchive(loadedMarios);
	setActiveMario(0);
    loadedMarios++;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x80276BF0, 0, 0, 0), loadMario);

// Description: A setup function on stage load to reset global fields per level
void setupPlayers(TMarDirector *director) {
    //setActiveMarioArchive(0);
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

