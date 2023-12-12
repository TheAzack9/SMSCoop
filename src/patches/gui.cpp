#include <sdk.h>
#include <macros.h>
#include <SMS/GC2D/GCConsole2.hxx>
#include <raw_fn.hxx>

#include "players.hxx"
#include "camera.hxx"
#include "splitscreen.hxx"

namespace SMSCoop {
	
	TGCConsole2* consoles[2];
	
	TGCConsole2* getConsoleForPlayer(int id) {
		return consoles[1 - id];
	}

	// Create all instances of TGCConsole2
	void TGCConsole2_constructor(TGCConsole2* console, char* param_1) {

		consoles[0] = console;
		__ct__11TGCConsole2FPCc(console, param_1);

		for(int i = 1; i < 2; ++i) {
			TGCConsole2* otherConsole = new TGCConsole2();
			__ct__11TGCConsole2FPCc(otherConsole, param_1);
			consoles[i] = otherConsole;
		}

	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8029db48, 0, 0, 0), TGCConsole2_constructor);

	// Load all instances of TGCConsole2
	void TGCConsole2_load(TGCConsole2* tgcConsole2, JSUMemoryInputStream* param_1) {
		char buffer[64];

		for(int i = 0; i < 2; ++i) {
			load__11TGCConsole2FR20JSUMemoryInputStream(consoles[i], param_1);

			// TODO: Not hard coded
			if(i == 1) {
				{
					J2DPicture* marioIcon =
						reinterpret_cast<J2DPicture*>(consoles[i]->mMainScreen->search('m_ic'));
					snprintf(buffer, 64, "/game_6/timg/%s_icon.bti", "luigi");

					auto* timg = reinterpret_cast<ResTIMG*>(JKRFileLoader::getGlbResource(buffer));
					if (timg)
						marioIcon->changeTexture(timg, 0);
				}

				{
					J2DPicture *marioName =
						reinterpret_cast<J2DPicture *>(consoles[i]->mMainScreen->search('m_tx'));

					snprintf(buffer, 64, "/game_6/timg/%s_text.bti", "luigi");

					auto *timg = reinterpret_cast<ResTIMG *>(JKRFileLoader::getGlbResource(buffer));
					if (timg)
						marioName->changeTexture(timg, 0);
				}
			}
		}
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803c0314, 0, 0, 0), (u32)(&TGCConsole2_load));

	// Load all instances of TGCConsole2
	void TGCConsole2_loadAfter(TGCConsole2* tgcConsole2) {
		for(int i = 0; i < 2; ++i) {
			loadAfter__11TGCConsole2Fv(consoles[i]);
		}
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803c031c, 0, 0, 0), (u32)(&TGCConsole2_loadAfter));

	void TGCConsole2_perform_override(TGCConsole2* tgcConsole2, u32 param_1, JDrama::TGraphics* graphics) {
		int p = getActiveViewport();

		// We still need to update ui for other players for animations to play correctly
		// param_1 & 0x8 = drawing, so we remove that flag
		for(int i = 0; i < getPlayerCount(); ++i) {
			if(i == p) continue;
			setActiveMario(i);
			setCamera(i);
			perform__11TGCConsole2FUlPQ26JDrama9TGraphics(consoles[i], param_1 & (~8), graphics);
		}

		// Update and draw current ui
		setActiveMario(p);
		setCamera(p);
		perform__11TGCConsole2FUlPQ26JDrama9TGraphics(consoles[p], param_1, graphics);
	}
	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803c0324, 0, 0, 0), (u32)(&TGCConsole2_perform_override));

	u8 pausingPlayer = 0;
	static bool checkIfPauseMenu() {
		TMarDirector *director;
		SMS_FROM_GPR(31, director);
		
		if(director->mAreaID == 15) return false;

		bool openMenu = false;

		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* mario = getMario(i);
			u32 attributes = *(u32*)&mario->mAttributes;

			if((attributes & 0x1000) != 0) continue;
			if((mario->mState & 0x800) != 0) continue;
			const JUTGamePad::CButton &buttons = mario->mController->mButtons;
			if((buttons.mFrameInput & TMarioGamePad::Z) != 0) continue;
			if((buttons.mFrameInput & TMarioGamePad::START) == 0) continue;
			openMenu = true;
			pausingPlayer = i;
			//OSReport("Menu was opened by %d \n", pausingPlayer);
		}

		return openMenu;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80297A48, 0, 0, 0), checkIfPauseMenu);
	SMS_WRITE_32(SMS_PORT_REGION(0x80297A4C, 0, 0, 0), 0x28030000);
	SMS_WRITE_32(SMS_PORT_REGION(0x80297a64, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x80297a6c, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x80297a78, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x80297a88, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x80297a9c, 0, 0, 0), 0x60000000);

	void TPauseMenu2_perform_override(u32* pauseMenu, u32 performFlags, JDrama::TGraphics* graphics) {
		
        TApplication *app      = &gpApplication;
        TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

		//u32 state = *(u32*)&director->mGamePads[pausingPlayer]->mState;

		director->mGamePads[pausingPlayer]->mState = director->mGamePads[0]->mState;
		*(TMarioGamePad**)(pauseMenu + 0x10c / 4) = director->mGamePads[pausingPlayer];

		perform__11TPauseMenu2FUlPQ26JDrama9TGraphics(pauseMenu, performFlags, graphics);
		*(TMarioGamePad**)(pauseMenu + 0x10c / 4) = director->mGamePads[0];
		//*(u32*)&director->mGamePads[pausingPlayer]->mState = state;
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c0570, 0, 0, 0), (u32)(&TPauseMenu2_perform_override));
	
	void TCardSave_perform_override(u32* cardSave, u32 performFlags, JDrama::TGraphics* graphics) {
		if(isSingleplayerLevel()) {
			perform__9TCardSaveFUlPQ26JDrama9TGraphics(cardSave, performFlags, graphics);
			return;
		}
        TApplication *app      = &gpApplication;
        TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

		//u32 state = *(u32*)&director->mGamePads[pausingPlayer]->mState;
		
		director->mGamePads[pausingPlayer]->mState = director->mGamePads[0]->mState;
		*(TMarioGamePad**)(cardSave + 0x270 / 4) = director->mGamePads[pausingPlayer];

		perform__9TCardSaveFUlPQ26JDrama9TGraphics(cardSave, performFlags, graphics);
		*(TMarioGamePad**)(cardSave + 0x270 / 4) = director->mGamePads[0];
		//*(u32*)&director->mGamePads[pausingPlayer]->mState = state;
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c07c0, 0, 0, 0), (u32)(&TCardSave_perform_override));
}