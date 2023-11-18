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
}