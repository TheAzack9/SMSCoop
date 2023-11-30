#include <sdk.h>
#define NTSCU
#include <raw_fn.hxx>

#include <SMS/Strategic/HitActor.hxx>
#include <JDrama/JDRGraphics.hxx>

#include "players.hxx"
#include "camera.hxx"
#include "splitscreen.hxx"

namespace SMSCoop {
	TMario* marioEnteringGate = nullptr;

	void resetAi(TMarDirector *director) {
		marioEnteringGate = nullptr;
	}

	TMario* getMarioEnteringGate() {
		return marioEnteringGate;
	}
	
	// Npc interactions
	void TBaseNPC_perform_override(THitActor* npc, u32 perform_flags, JDrama::TGraphics* graphics) {
		int marioId = getClosestMarioId(&npc->mTranslation);
		setActiveMario(marioId);
		setCamera(marioId);
		perform__8TBaseNPCFUlPQ26JDrama9TGraphics(npc, perform_flags, graphics);
		setActiveMario(getActiveViewport());
		setCamera(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8468, 0, 0, 0), (u32)(&TBaseNPC_perform_override));
	

	// Fixes M gates entry
	void TModelGate_perform(THitActor* modelGate, u32 performFlags, JDrama::TGraphics* graphics) {
		int cm = getClosestMarioId(&modelGate->mTranslation);
		setActiveMario(cm);
		setCamera(cm);
		perform__10TModelGateFUlPQ26JDrama9TGraphics(modelGate, performFlags, graphics);
		
		setCamera(getActiveViewport());
		setActiveMario(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d3fbc, 0, 0, 0), (u32)(&TModelGate_perform));
	SMS_WRITE_32(SMS_PORT_REGION(0x803d3fbc, 0, 0, 0), (u32)(&TModelGate_perform));
	
	// Fixes M gates entry
	int TMario_receiveMessage_enterGate(TMario* mario, THitActor* shineGate, u32 msg) {
		if(marioEnteringGate != nullptr) return 0;
		mario->dropObject();
		int result = mario->receiveMessage(shineGate, msg);
		if(result == 1) {
			marioEnteringGate = mario;
		}
		return result;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801eb28c, 0, 0, 0), TMario_receiveMessage_enterGate);

	// Fix goop moving towards mario
	void TBiancoGateKeeper_perform_override(TLiveActor* bgk, u32 perform_flags, JDrama::TGraphics* graphics) {
		int marioId = getClosestMarioId(&bgk->mTranslation);
		setActiveMario(marioId);
		setCamera(marioId);
		perform__17TBiancoGateKeeperFUlPQ26JDrama9TGraphics(bgk, perform_flags, graphics);
		setActiveMario(getActiveViewport());
		setCamera(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bb73c, 0, 0, 0), (u32)(&TBiancoGateKeeper_perform_override));

	
	// Petey
	
	// Fixes Petey (Boss Pakkun) swallow not triggering when petey is not facing both players
	void TBPHeadHit_receiveMessage(JDrama::TPlacement* bpHead, THitActor* sender, u32 message) {
		int currentMario = getPlayerId(gpMarioOriginal);
		int closestMario = getClosestMarioId(&bpHead->mTranslation);

		setActiveMario(closestMario);

		receiveMessage__10TBPHeadHitFP9THitActorUl(bpHead, sender, message);

		setActiveMario(currentMario);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b4870, 0, 0, 0), (u32)(&TBPHeadHit_receiveMessage));
	
	// Fixes Petey (Boss Pakkun) look towards mario
	void TBossPakkun_perform_override(JDrama::TPlacement* bp, u32 performFlags, JDrama::TGraphics* graphics) {
		int marioId = getClosestMarioId(&bp->mTranslation);
		setActiveMario(marioId);
		setCamera(marioId);
		perform__11TBossPakkunFUlPQ26JDrama9TGraphics(bp, performFlags, graphics);
		
		setActiveMario(getActiveViewport());
		setCamera(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b45f4, 0, 0, 0), (u32)(&TBossPakkun_perform_override));

	// Popo / poinks in bianco 5
	void TPopo_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__5TPopoFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803ba578, 0, 0, 0), (u32)(&TPopo_perform_override));

	// Skeeter / amenbo
	void TAmenbo_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__7TAmenboFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803be6dc, 0, 0, 0), (u32)(&TAmenbo_perform_override));

	
	// strollin stu / hamu kuri
	void THamuKuri_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__11TSmallEnemyFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b0a4c, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// Sambo head / pokey head
	SMS_WRITE_32(SMS_PORT_REGION(0x803b9e2c, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// Gesso / blooper
	SMS_WRITE_32(SMS_PORT_REGION(0x803af8cc, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// Kaze / wind
	SMS_WRITE_32(SMS_PORT_REGION(0x803bceac, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// Kumokun / yellow blue coin spider
	SMS_WRITE_32(SMS_PORT_REGION(0x803bdbd4, 0, 0, 0), (u32)(&THamuKuri_perform_override));

	
	// gooble / name kuri
	void TNameKuri_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__9TNameKuriFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b17d0, 0, 0, 0), (u32)(&TNameKuri_perform_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b1a40, 0, 0, 0), (u32)(&TNameKuri_perform_override));

	
	// spawn name kuri's
	void TConductor_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__10TConductorFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803ad978, 0, 0, 0), (u32)(&TConductor_perform_override));
	
	// kukus / plurp / coo coo
	void TKukku_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__6TKukkuFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803be3c8, 0, 0, 0), (u32)(&TKukku_perform_override));

	
	// SamboFlower / pokey heads when underground
	void TSamboFlower_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__11TSpineEnemyFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803ba340, 0, 0, 0), (u32)(&TSamboFlower_perform_override));
	// Wire traps
	SMS_WRITE_32(SMS_PORT_REGION(0x803bc8bc, 0, 0, 0), (u32)(&TSamboFlower_perform_override));

	
	// HanaSambo / pokeys
	void THanaSambo_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__10THanaSamboFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803ba09c, 0, 0, 0), (u32)(&THanaSambo_perform_override));
	
	
	// Pokey head flying away in right direction
	void TSamboHead_behaveToWater_override(JDrama::TPlacement* placement, THitActor* actor) {
		int marioId = getClosestMarioId(&placement->mTranslation);
		setActiveMario(marioId);

		behaveToWater__10TSamboHeadFP9THitActor(placement, actor);
		
		setActiveMario(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b9f38, 0, 0, 0), (u32)(&TSamboHead_behaveToWater_override));

	
	
	// YumboHead / Seed pod things
	void TYumboHead_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__6TYumboFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bd3b4, 0, 0, 0), (u32)(&TYumboHead_perform_override));

	
	// Pakkun / piranha plant (goop shooting)
	void TPakkun_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__7TPakkunFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b1e50, 0, 0, 0), (u32)(&TPakkun_perform_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b2148, 0, 0, 0), (u32)(&TPakkun_perform_override));

	
	// EMario / shadow mario
	void TEMario_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__7TEMarioFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803ae424, 0, 0, 0), (u32)(&TEMario_perform_override));
	
	// EnemyMario / shadow mario
	void TEnemyMario_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__11TEnemyMarioFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803aef44, 0, 0, 0), (u32)(&TEnemyMario_perform_override));

	// Fix rotten leaf boats
	TMario* marioControllingRottenLeaf = nullptr;
	void TLeafBoatRotten_control(void* leafBoatRotten) {
		if(marioControllingRottenLeaf) {
			int controllingMario = getPlayerId(marioControllingRottenLeaf);
			setActiveMario(controllingMario);

			if(!marioIsOn__11TMapObjBaseCFv(leafBoatRotten) || !marioControllingRottenLeaf->mFludd->isEmitting()) {
				marioControllingRottenLeaf = nullptr;
			}
		}

		if(!marioControllingRottenLeaf) {
			for(int i = 0; i < getPlayerCount(); ++i) {
				TMario* mario = getMario(i);
				setActiveMario(i);
				if(marioIsOn__11TMapObjBaseCFv(leafBoatRotten) && mario->mFludd->isEmitting()) {
					marioControllingRottenLeaf = mario;
					break;
				}
			}
		}
		
		control__15TLeafBoatRottenFv(leafBoatRotten);

		setActiveMario(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803cc8b0, 0, 0, 0), (u32)(&TLeafBoatRotten_control));

	// Fix leaf boats
	TMario* marioControllingLeaf = nullptr;
	void TLeafBoat_control(void* leafBoat) {
		if(marioControllingLeaf) {
			int controllingMario = getPlayerId(marioControllingLeaf);
			setActiveMario(controllingMario);

			if(!marioIsOn__11TMapObjBaseCFv(leafBoat) || !marioControllingLeaf->mFludd->isEmitting()) {
				marioControllingLeaf = nullptr;
			}
		}

		if(!marioControllingLeaf) {
			for(int i = 0; i < getPlayerCount(); ++i) {
				TMario* mario = getMario(i);
				setActiveMario(i);
				if(marioIsOn__11TMapObjBaseCFv(leafBoat) && mario->mFludd->isEmitting()) {
					marioControllingLeaf = mario;
					break;
				}
			}
		}
		
		control__9TLeafBoatFv(leafBoat);

		setActiveMario(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803cca14, 0, 0, 0), (u32)(&TLeafBoat_control));

	// Fix boats
	TMario* marioControllingBoat = nullptr;
	void TMuddyBoat_control(void* muddyBoat) {
		if(marioControllingBoat) {
			int controllingMario = getPlayerId(marioControllingBoat);
			setActiveMario(controllingMario);

			if(!marioIsOn__11TMapObjBaseCFv(muddyBoat) || !marioControllingBoat->mFludd->isEmitting()) {
				marioControllingBoat = nullptr;
			}
		}

		if(!marioControllingBoat) {
			for(int i = 0; i < getPlayerCount(); ++i) {
				TMario* mario = getMario(i);
				setActiveMario(i);
				if(marioIsOn__11TMapObjBaseCFv(muddyBoat) && mario->mFludd->isEmitting()) {
					marioControllingBoat = mario;
					break;
				}
			}
		}
		control__10TMuddyBoatFv(muddyBoat);

		setActiveMario(getActiveViewport());

	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d166c, 0, 0, 0), (u32)(&TMuddyBoat_control));

	
	// BeeHive / not the bees
	void TBeeHive_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__8TBeeHiveFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803ac064, 0, 0, 0), (u32)(&TBeeHive_perform_override));

	
	// 1up
	void TMushroom1Up_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__12TMushroom1upFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d6d9c, 0, 0, 0), (u32)(&TMushroom1Up_perform_override));

	// Ricco time
	TMario* marioToAttack = nullptr;
	void TBossGesso_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId;
			if(!marioToAttack) {
				marioId = getClosestMarioId(&placement->mTranslation);
			} else {
				marioId = getPlayerId(marioToAttack);
			}
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__10TBossGessoFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b2b1c, 0, 0, 0), (u32)(&TBossGesso_perform_override));

	// Attempt to make boss blooper focus on a player
	//void TBossGesso_changeAttackMode(JDrama::TPlacement* bossGesso, int mode) {
	//	OSReport("Setting attack mode %d\n", mode);
	//	if(mode != 0) {
	//		marioToAttack = gpMarioOriginal;
	//	} else {
	//		marioToAttack = nullptr;
	//	}
	//	changeAttackMode__10TBossGessoFi(bossGesso, mode);
	//}
 //   SMS_PATCH_BL(SMS_PORT_REGION(0x80074f58, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80075ec0, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8007684c, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8007685c, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80076ad8, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80076c68, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80076ce8, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x800770ac, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80077130, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8007725c, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x800772c0, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80077484, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80077504, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8007756c, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x800775ec, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x800777bc, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80077990, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80077e00, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x8007803c, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x800780d0, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x800780e0, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x800782f8, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x800783dc, 0, 0, 0), TBossGesso_changeAttackMode);
	//SMS_PATCH_BL(SMS_PORT_REGION(0x80078e80, 0, 0, 0), TBossGesso_changeAttackMode);
	
	// kaze / wind
	void TKazekun_perform_override(TSpineEnemy* kazekun, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId;
			if(kazekun->mTarget) {
				marioId = getPlayerId((TMario*)kazekun->mTarget);
			} else {
				marioId = getClosestMarioId(&kazekun->mTranslation);
			}
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__11TSmallEnemyFUlPQ26JDrama9TGraphics(kazekun, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bceac, 0, 0, 0), (u32)(&TKazekun_perform_override));

	void TKazekun_updateSquareToMario_override(TSpineEnemy* kaze) {
		int marioId = getClosestMarioId(&kaze->mTranslation);
		setActiveMario(marioId);
		setCamera(marioId);
		kaze->mTarget = gpMarioOriginal;
		updateSquareToMario__11TSpineEnemyFv(kaze);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8010d86c, 0, 0, 0), TKazekun_updateSquareToMario_override);
}