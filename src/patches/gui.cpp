#include <sdk.h>
#include <macros.h>
#include <SMS/GC2D/GCConsole2.hxx>
#include <raw_fn.hxx>

#include "players.hxx"
#include "camera.hxx"
#include "splitscreen.hxx"
#include "settings.hxx"
#include "pvp.hxx"

extern SMSCoop::PlayerTypeSetting gPlayer1TypeSetting;
extern SMSCoop::PlayerTypeSetting gPlayer2TypeSetting;

namespace SMSCoop {
	
	TGCConsole2* consoles[2];
	bool gIsInGuide = false;

	JUTTexture* guidePictures[7];
	
	TGCConsole2* getConsoleForPlayer(int id) {
		return consoles[1 - id];
	}

	bool isInGuide() {
		return gIsInGuide && gpMarDirector->mGuide->mState != 9  && gpMarDirector->mGuide->mState != 8;
	}

	// Create all instances of TGCConsole2
	void TGCConsole2_constructor(TGCConsole2* console, char* param_1) {

		consoles[0] = console;
		gIsInGuide = false;
		__ct__11TGCConsole2FPCc(console, param_1);

		for(int i = 1; i < 2; ++i) {
			TGCConsole2* otherConsole = new TGCConsole2();
			__ct__11TGCConsole2FPCc(otherConsole, param_1);
			consoles[i] = otherConsole;
		}

	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8029db48, 0, 0, 0), TGCConsole2_constructor);

	const char* getSkinNameForPlayer(int i) {
		int playerType = gPlayer1TypeSetting.getInt();
		if(i == 1) playerType = gPlayer2TypeSetting.getInt();

		if(isPvpLevel() && i == 0) return "mario";
		if(isPvpLevel() && i == 1) return "luigi";
		
		if(playerType == PlayerTypeSetting::LUIGI) return "luigi";
		if(playerType == PlayerTypeSetting::SHADOW_MARIO) return "kagemario";
		return "mario";
	}

	// Load all instances of TGCConsole2
	void TGCConsole2_load(TGCConsole2* tgcConsole2, JSUMemoryInputStream* param_1) {
		char buffer[64];

		bool isSingleCamera = isSingleplayerLevel();

		for(int i = 0; i < 2; ++i) {
			load__11TGCConsole2FR20JSUMemoryInputStream(consoles[i], param_1);

			if(isSingleCamera) {
				const char* type = getSkinNameForPlayer(0);

				{
					J2DPicture* marioIcon =
						reinterpret_cast<J2DPicture*>(consoles[i]->mMainScreen->search('m_ic'));
					snprintf(buffer, 64, "/game_6/timg/%s_icon.bti", type);

					auto* timg = reinterpret_cast<ResTIMG*>(JKRFileLoader::getGlbResource(buffer));
					if (timg)
						marioIcon->changeTexture(timg, 0);
				}

				{
					J2DPicture *marioName =
						reinterpret_cast<J2DPicture *>(consoles[i]->mMainScreen->search('m_tx'));

					snprintf(buffer, 64, "/game_6/timg/%s_text.bti", type);

					auto *timg = reinterpret_cast<ResTIMG *>(JKRFileLoader::getGlbResource(buffer));
					if (timg)
						marioName->changeTexture(timg, 0);
				}
			} else {
				const char* type1 = getSkinNameForPlayer(0);
				const char* type2 = getSkinNameForPlayer(1);
			
				{
					J2DPicture* marioIcon =
						reinterpret_cast<J2DPicture*>(consoles[i]->mMainScreen->search('m_ic'));
					snprintf(buffer, 64, "/game_6/timg/%s_%s_icon.bti", type1, type2);

					auto* timg = reinterpret_cast<ResTIMG*>(JKRFileLoader::getGlbResource(buffer));
					if (timg) {
						marioIcon->changeTexture(timg, 0);
					}
				}

				{
					J2DPicture *marioName =
						reinterpret_cast<J2DPicture *>(consoles[i]->mMainScreen->search('m_tx'));

					snprintf(buffer, 64, "/game_6/timg/%s_%s_text.bti", type1, type2);

					auto *timg = reinterpret_cast<ResTIMG *>(JKRFileLoader::getGlbResource(buffer));
					if (timg) {
						marioName->changeTexture(timg, 0);
					}
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
			consoles[i]->mMainScreen->mRect.mX2 = 200;
			consoles[i]->mMainScreen->mRect.mY2 = 200;
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
	static int checkIfPauseMenu() {
		TMarDirector *director;
		SMS_FROM_GPR(31, director);

		if(director->mCurState != 10) {
			gIsInGuide = false;
		}
		
		if(director->mAreaID == 15) return 0;



		int menuToOpen = 0;

		for(int i = 0; i < getPlayerCount(); ++i) {
			TMario* mario = getMario(i);
			u32 attributes = *(u32*)&mario->mAttributes;

			// Do not pause if airborn and not in water
			if((attributes & 0x1000) == 0 && (mario->mState & 0x800) != 0) continue;
			const JUTGamePad::CButton &buttons = mario->mController->mButtons;
			if((buttons.mFrameInput & TMarioGamePad::Z) != 0 && !isPvpLevel()) {
				menuToOpen = 10;

				int playerType = gPlayer1TypeSetting.getInt();
				if(i == 1) playerType = gPlayer2TypeSetting.getInt();

				TGuide* guide = director->mGuide;
				J2DPicture* cursora =
					reinterpret_cast<J2DPicture*>(guide->mScreen->search('cu_a'));

				// Mario
				u32 ids[10] = {'mi00', 'mi01', 'mi02', 'mi03', 'mi04', 'mi05', 'mi06', 'mi07', 'mi08', 'mi09'};
				if(playerType == PlayerTypeSetting::MARIO) {
					if(guidePictures[0]) {
						for(int j = 0; j < 10; ++j) {
							J2DPicture* marioIcon =
								reinterpret_cast<J2DPicture*>(guide->mScreen->search(ids[j]));
							marioIcon->mTextures[0] = guidePictures[0];
						}
					}

					if(guidePictures[1]) cursora->mTextures[0] = guidePictures[1];
					if(guidePictures[2]) cursora->mTextures[1] = guidePictures[2];
				}
				else if(playerType == PlayerTypeSetting::LUIGI) {
					if(guidePictures[3]) {
						OSReport("Luigi time \n");
						for(int j = 0; j < 10; ++j) {
							J2DPicture* marioIcon =
								reinterpret_cast<J2DPicture*>(guide->mScreen->search(ids[j]));
							marioIcon->mTextures[0] = guidePictures[3];
						}
					}
					
					if(guidePictures[1]) cursora->mTextures[0] = guidePictures[1];
					if(guidePictures[2]) cursora->mTextures[1] = guidePictures[2];
				} else if(playerType == PlayerTypeSetting::SHADOW_MARIO) {
					if(guidePictures[4]) {
						for(int j = 0; j < 10; ++j) {
							J2DPicture* marioIcon =
								reinterpret_cast<J2DPicture*>(guide->mScreen->search(ids[j]));
							marioIcon->mTextures[0] = guidePictures[4];
						}
					}
					if(guidePictures[5]) cursora->mTextures[0] = guidePictures[5];
					if(guidePictures[6]) cursora->mTextures[1] = guidePictures[6];
				}

				pausingPlayer = i;
				gIsInGuide = true;
				break;
			}
			if((buttons.mFrameInput & TMarioGamePad::START) != 0) {
				menuToOpen = 5;
				pausingPlayer = i;
			}
			//OSReport("Menu was opened by %d \n", pausingPlayer);
		}

		return menuToOpen;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80297A48, 0, 0, 0), checkIfPauseMenu);
	SMS_WRITE_32(SMS_PORT_REGION(0x80297A4C, 0, 0, 0), 0x28030000);
	SMS_WRITE_32(SMS_PORT_REGION(0x80297a54, 0, 0, 0), 0x607d0000); // 0x607d0000
	SMS_WRITE_32(SMS_PORT_REGION(0x80297a58, 0, 0, 0), 0x4800059c);
	SMS_WRITE_32(SMS_PORT_REGION(0x80297a78, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x80297a88, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x80297a9c, 0, 0, 0), 0x60000000);

	void TPauseMenu2_perform_override(u32* pauseMenu, u32 performFlags, JDrama::TGraphics* graphics) {
		
        TApplication *app      = &gpApplication;
        TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

		//u32 state = *(u32*)&director->mGamePads[pausingPlayer]->mState;

		director->mGamePads[pausingPlayer]->mState = director->mGamePads[0]->mState;
		*(TMarioGamePad**)(pauseMenu + 0x10c / 4) = director->mGamePads[pausingPlayer];

		// Tried to make pause only appear on person who paused's screen
		//u32 overrideFlags = performFlags;
		//if(getMario(pausingPlayer) != gpMarioOriginal) {
		//	overrideFlags &= ~0x8;
		//	if(performFlags & 0x8 && gpMarDirector->mCurState == 5) {
		//		J2DFillBox(0, 0, 640, 640, {0, 0, 0, 150});
		//	}
		//}


		perform__11TPauseMenu2FUlPQ26JDrama9TGraphics(pauseMenu, performFlags, graphics);


		*(TMarioGamePad**)(pauseMenu + 0x10c / 4) = director->mGamePads[0];
		//*(u32*)&director->mGamePads[pausingPlayer]->mState = state;
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c0570, 0, 0, 0), (u32)(&TPauseMenu2_perform_override));


	void TGuide_perform_override(u32* guide, u32 performFlags, JDrama::TGraphics* graphics) {
        TApplication *app      = &gpApplication;
        TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);

		director->mGamePads[pausingPlayer]->mState = director->mGamePads[0]->mState;
		*(TMarioGamePad**)(guide + 0xc0 / 4) = director->mGamePads[pausingPlayer];

		perform__6TGuideFUlPQ26JDrama9TGraphics(guide, performFlags, graphics);


		*(TMarioGamePad**)(guide + 0xc0 / 4) = director->mGamePads[0];
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c1068, 0, 0, 0), (u32)(&TGuide_perform_override));

	void TGuide_load_override(TGuide* guide, JSUMemoryInputStream* memStream) {
		//guide->load(memStream);
		
		load__6TGuideFR20JSUMemoryInputStream(guide, memStream);
		for(int i = 0; i < 7; ++i) {
			guidePictures[i] = new JUTTexture();
		}
		bool isSingleCamera = isSingleplayerLevel();

		int player1Type = gPlayer1TypeSetting.getInt();
		int player2Type = gPlayer2TypeSetting.getInt();
		bool hasLuigi = player1Type == PlayerTypeSetting::LUIGI || player2Type == PlayerTypeSetting::LUIGI;
		bool hasKageMario = player1Type == PlayerTypeSetting::SHADOW_MARIO || player2Type == PlayerTypeSetting::SHADOW_MARIO;
			
		guidePictures[0]->storeTIMG(reinterpret_cast<ResTIMG*>(JKRFileLoader::getGlbResource("/guide/timg/guide_mario.bti")));
		guidePictures[1]->storeTIMG(reinterpret_cast<ResTIMG*>(JKRFileLoader::getGlbResource("/guide/timg/guide_cursor_1.bti")));
		guidePictures[2]->storeTIMG(reinterpret_cast<ResTIMG*>(JKRFileLoader::getGlbResource("/guide/timg/guide_cursor_2.bti")));
		if(hasLuigi) {
			guidePictures[3]->storeTIMG(reinterpret_cast<ResTIMG*>(JKRFileLoader::getGlbResource("/guide/timg/guide_luigi.bti")));
		}
		if(hasKageMario) {
			guidePictures[4]->storeTIMG(reinterpret_cast<ResTIMG*>(JKRFileLoader::getGlbResource("/guide/timg/guide_kagemario.bti")));
			guidePictures[5]->storeTIMG(reinterpret_cast<ResTIMG*>(JKRFileLoader::getGlbResource("/guide/timg/guide_cursor_kagemario_1.bti")));
			guidePictures[6]->storeTIMG(reinterpret_cast<ResTIMG*>(JKRFileLoader::getGlbResource("/guide/timg/guide_cursor_kagemario_2.bti")));
				
		}


	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c1058, 0, 0, 0), (u32)(&TGuide_load_override));

	
	void TCardSave_perform_override(u32* cardSave, u32 performFlags, JDrama::TGraphics* graphics) {
		if(isSingleCameraLevel()) {
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