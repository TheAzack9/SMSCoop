#include <sdk.h>
#include <raw_fn.hxx>

#include <SMS/Strategic/HitActor.hxx>
#include <JDrama/JDRGraphics.hxx>
#include <SMS/MapObj/MapObjRail.hxx>

#include "players.hxx"
#include "camera.hxx"
#include "splitscreen.hxx"
#include "yoshi.hxx"

#define MARIO_COUNT 2
bool* sMantaEscapeFromMario = (bool*)0x8040db40;

namespace SMSCoop {
	TMario* marioEnteringGate = nullptr;
	bool hasTriggeredMechaBowserCutscene = false;
	u32 jetcoasterDemoCallbackCamera = 0;
	char canRocketMountMario[2] = {1, 1};
	bool eelFirstDie = false;
	TTakeActor* bossEel = nullptr;

	void resetAi(TMarDirector *director) {
		marioEnteringGate = nullptr;
		hasTriggeredMechaBowserCutscene = false;
		jetcoasterDemoCallbackCamera = 0;
		eelFirstDie = false;
		bossEel = nullptr;
		for(int i = 0; i < 2; ++i) {
			canRocketMountMario[i] = 1;
		}
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
	
	SMS_WRITE_32(SMS_PORT_REGION(0x800fccdc, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x800fcce0, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x800fcce8, 0, 0, 0), 0x60000000);
	
	// Petey
	
	// Fixes Petey (Boss Pakkun) swallow not triggering when petey is not facing both players
	void TBPHeadHit_receiveMessage(JDrama::TPlacement* bpHead, THitActor* sender, u32 message) {
		int closestMario = getClosestMarioId(&bpHead->mTranslation);

		setActiveMario(closestMario);

		receiveMessage__10TBPHeadHitFP9THitActorUl(bpHead, sender, message);

		setActiveMario(getActiveViewport());
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
	void TPopo_perform_override(TSpineEnemy* enemy, u32 performFlags, JDrama::TGraphics* graphics) {
		int marioId = getClosestMarioId(&enemy->mTranslation);
		setActiveMario(marioId);
		setCamera(marioId);
		if(performFlags & 0x1) {
			
			if(enemy->mTarget) {
				int targetMario = getPlayerId((TMario*)enemy->mTarget);
				if(targetMario != marioId) {
					u32* nerveWalkerGraphWander = (u32*)((u32)enemy + 0x70);
					u8* isNotPossessed = (u8*)(*nerveWalkerGraphWander + 0x60);
					*isNotPossessed = true;
				}
			}
			

			
			enemy->mTarget = getMario(marioId);
				
			//if(enemy->mTarget) {
			//	marioId = getPlayerId((TMario*)enemy->mTarget);
			//} else {
			//}
		}
		perform__5TPopoFUlPQ26JDrama9TGraphics(enemy, performFlags, graphics);
		
		setActiveMario(getActiveViewport());
		setCamera(getActiveViewport());
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
	
	// Yoshi goop thingys
	void TSeal_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__5TSealFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bbc94, 0, 0, 0), (u32)(&TSeal_perform_override));
	
	u32 TSeal_receiveMessage_override(JDrama::TPlacement* placement, THitActor* actor, u32 flag) {
		int marioId = getClosestMarioId(&placement->mTranslation);
		TMario* closestMario = getMario(marioId);
		setWaterColorForMario(closestMario);
		u32 result = receiveMessage__5TSealFP9THitActorUl(placement, actor, flag);
		setWaterColorForMario(gpMarioOriginal);
		return result;
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bbd14, 0, 0, 0), (u32)(&TSeal_receiveMessage_override));
	
	// strollin stu / hamu kuri
	void THamuKuri_perform_override(TSpineEnemy* enemy, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId;
			if(enemy->mTarget) {
				marioId = getPlayerId((TMario*)enemy->mTarget);
			} else {
				marioId = getClosestMarioId(&enemy->mTranslation);
			}
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__11TSmallEnemyFUlPQ26JDrama9TGraphics(enemy, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b0a4c, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// TFireHamuKuri / smoldering flaming stu
	SMS_WRITE_32(SMS_PORT_REGION(0x803aff9c, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// Sambo head / pokey head
	SMS_WRITE_32(SMS_PORT_REGION(0x803b9e2c, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// Gesso / blooper
	SMS_WRITE_32(SMS_PORT_REGION(0x803af8cc, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// Kaze / wind
	SMS_WRITE_32(SMS_PORT_REGION(0x803bceac, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// Kumokun / yellow blue coin spider
	SMS_WRITE_32(SMS_PORT_REGION(0x803bdbd4, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// TKiller / bullet bills / Kuri
	SMS_WRITE_32(SMS_PORT_REGION(0x803b98e0, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// TDoroHaneKuri / swipin stus? (thicc flyings ones)
	SMS_WRITE_32(SMS_PORT_REGION(0x803b06bc, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// TDoroHaneKuri / flying stus? (smol flyings ones)
	SMS_WRITE_32(SMS_PORT_REGION(0x803afdd4, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// TElecNokoNoko / Electric koopas
	SMS_WRITE_32(SMS_PORT_REGION(0x803b30b8, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// THaneHamuKuri / smol flying stus
	SMS_WRITE_32(SMS_PORT_REGION(0x803b0884, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// THaneHamuKuri2 / smol flying stus pt 2?
	SMS_WRITE_32(SMS_PORT_REGION(0x803b04f4, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// TRocket / pinna rockets
	SMS_WRITE_32(SMS_PORT_REGION(0x803bcbc0, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// TBossDangoHamuKuri / Stackin stus base
	SMS_WRITE_32(SMS_PORT_REGION(0x803b0164, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// TDebuTelesa / sleepy boo (/ fat boo aparently)
	SMS_WRITE_32(SMS_PORT_REGION(0x803bf1f8, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// TMoePuku / fire fish
	SMS_WRITE_32(SMS_PORT_REGION(0x803b4b18, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// TBubble / bubbles from king boo
	SMS_WRITE_32(SMS_PORT_REGION(0x803b80f0, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// TIgaiga / goop balls
	SMS_WRITE_32(SMS_PORT_REGION(0x803b6b68, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	// TBombhei / bob-ombs
	SMS_WRITE_32(SMS_PORT_REGION(0x803b8658, 0, 0, 0), (u32)(&THamuKuri_perform_override));
	

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
	// Wire traps / Things on wire
	SMS_WRITE_32(SMS_PORT_REGION(0x803bc8bc, 0, 0, 0), (u32)(&TSamboFlower_perform_override));
	// Birds / TAnimalBird
	SMS_WRITE_32(SMS_PORT_REGION(0x803abe98, 0, 0, 0), (u32)(&TSamboFlower_perform_override));
	// Boats / TFruitsBoat
	SMS_WRITE_32(SMS_PORT_REGION(0x803bab94, 0, 0, 0), (u32)(&TSamboFlower_perform_override));

	
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
	
	// Fix fire trap
	void TLampTrapIron_control_override(JDrama::TPlacement* lampTrapIron) {
		int marioId = getClosestMarioId(&lampTrapIron->mTranslation);
		setActiveMario(marioId);
		setCamera(marioId);
		control__13TLampTrapIronFv(lampTrapIron);
		setActiveMario(getActiveViewport());
		setCamera(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d7fa8, 0, 0, 0), (u32)(&TLampTrapIron_control_override));

	void TLampTrapIronHit_perform_override(JDrama::TPlacement* lampTrapIronHit, u32 flags, JDrama::TGraphics* graphics) {
		int marioId = getClosestMarioId(&lampTrapIronHit->mTranslation);
		setActiveMario(marioId);
		setCamera(marioId);
		perform__16TLampTrapIronHitFUlPQ26JDrama9TGraphics(lampTrapIronHit, flags, graphics);
		setActiveMario(getActiveViewport());
		setCamera(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d8064, 0, 0, 0), (u32)(&TLampTrapIronHit_perform_override));
	
	// Fix spike trap
	void TLampTrapSpike_control_override(JDrama::TPlacement* lampTrapSpike) {
		int marioId = getClosestMarioId(&lampTrapSpike->mTranslation);
		setActiveMario(marioId);
		setCamera(marioId);
		control__14TLampTrapSpikeFv(lampTrapSpike);
		setActiveMario(getActiveViewport());
		setCamera(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d81b0, 0, 0, 0), (u32)(&TLampTrapSpike_control_override));

	void TLampTrapSpikeHit_perform_override(JDrama::TPlacement* lampTrapSpikeHit, u32 flags, JDrama::TGraphics* graphics) {
		int marioId = getClosestMarioId(&lampTrapSpikeHit->mTranslation);
		setActiveMario(marioId);
		setCamera(marioId);
		perform__17TLampTrapSpikeHitFUlPQ26JDrama9TGraphics(lampTrapSpikeHit, flags, graphics);
		setActiveMario(getActiveViewport());
		setCamera(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d826c, 0, 0, 0), (u32)(&TLampTrapSpikeHit_perform_override));

	// Fix fire
	void TEffectObjBase_moveObj_override(JDrama::TPlacement* effectObjBase) {
		int marioId = getClosestMarioId(&effectObjBase->mTranslation);
		setActiveMario(marioId);
		setCamera(marioId);
		moveObject__14TEffectObjBaseFv(effectObjBase);
		setActiveMario(getActiveViewport());
		setCamera(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803ae1bc, 0, 0, 0), (u32)(&TEffectObjBase_moveObj_override));

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
	
	// Fix boats
	TMario* marioControllingSwing = nullptr;
	void TSwingBoard_control(void* swingBoard) {
		if(marioControllingSwing) {
			int controllingMario = getPlayerId(marioControllingSwing);
			setActiveMario(controllingMario);

			if(!marioIsOn__11TMapObjBaseCFv(swingBoard) || !marioControllingSwing->mFludd->isEmitting()) {
				marioControllingSwing = nullptr;
			}
		}

		if(!marioControllingSwing) {
			for(int i = 0; i < getPlayerCount(); ++i) {
				TMario* mario = getMario(i);
				setActiveMario(i);
				if(marioIsOn__11TMapObjBaseCFv(swingBoard) && mario->mFludd->isEmitting()) {
					marioControllingSwing = mario;
					break;
				}
			}
		}
		control__11TSwingBoardFv(swingBoard);

		setActiveMario(getActiveViewport());

	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d57b4, 0, 0, 0), (u32)(&TSwingBoard_control));
	
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
	//void TKazekun_perform_override(TSpineEnemy* kazekun, u32 performFlags, JDrama::TGraphics* graphics) {
	//	if(performFlags & 0x1) {
	//		int marioId;
	//		if(kazekun->mTarget) {
	//			marioId = getPlayerId((TMario*)kazekun->mTarget);
	//		} else {
	//			marioId = getClosestMarioId(&kazekun->mTranslation);
	//		}
	//		setActiveMario(marioId);
	//		setCamera(marioId);
	//	}
	//	perform__11TSmallEnemyFUlPQ26JDrama9TGraphics(kazekun, performFlags, graphics);
	//	
	//	if(performFlags & 0x1) {
	//		setActiveMario(getActiveViewport());
	//		setCamera(getActiveViewport());
	//	}
	//}
	//SMS_WRITE_32(SMS_PORT_REGION(0x803bceac, 0, 0, 0), (u32)(&TKazekun_perform_override));

	void TKazekun_updateSquareToMario_override(TSpineEnemy* kaze) {
		int marioId = getClosestMarioId(&kaze->mTranslation);
		setActiveMario(marioId);
		setCamera(marioId);
		kaze->mTarget = gpMarioOriginal;
		updateSquareToMario__11TSpineEnemyFv(kaze);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8010d86c, 0, 0, 0), TKazekun_updateSquareToMario_override);

	// Poi hana / cataquaks
	void TPoiHana_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__8TPoiHanaFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	// Sleeping
	SMS_WRITE_32(SMS_PORT_REGION(0x803b7028, 0, 0, 0), (u32)(&TPoiHana_perform_override));
	// Blue
	SMS_WRITE_32(SMS_PORT_REGION(0x803b7398, 0, 0, 0), (u32)(&TPoiHana_perform_override));
	// Red
	SMS_WRITE_32(SMS_PORT_REGION(0x803b71e0, 0, 0, 0), (u32)(&TPoiHana_perform_override));
	
	// ChuHana / Plungelos (gelato 2)
	void TChuuHana_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__9TChuuHanaFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b661c, 0, 0, 0), (u32)(&TChuuHana_perform_override));
	
	// Mirror in gelato 2
	void TLeanMirror_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__11TMapObjBaseFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		 
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803cf7b0, 0, 0, 0), (u32)(&TLeanMirror_perform_override));
	

	// TabePuku 
	void TTabePuku_behaveToWater_override(JDrama::TPlacement* placement, THitActor* actor) {
		int marioId = getClosestMarioId(&placement->mTranslation);
		setActiveMario(marioId);

		behaveToWater__9TChuuHanaFP9THitActor(placement, actor);
		
		setActiveMario(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b6728, 0, 0, 0), (u32)(&TTabePuku_behaveToWater_override));
	
	// TabePuku / Fish that drags you down
	void TTabePuku_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__9TTabePukuFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bf474, 0, 0, 0), (u32)(&TTabePuku_perform_override));
	
	// CoasterKiller / Rollercoaster bullet bill
	void TCoasterKiller_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__14TCoasterKillerFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bedf8, 0, 0, 0), (u32)(&TCoasterKiller_perform_override));
	
	// BathtubKiller / Bowser bullet bill
	void TBathtubKiller_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__14TBathtubKillerFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bebb0, 0, 0, 0), (u32)(&TBathtubKiller_perform_override));
	
	// TCannon / cannon
	void TCannon_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__7TCannonFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b8944, 0, 0, 0), (u32)(&TCannon_perform_override));

	
	// TBossTelesa / King boo
	void TBossTeleasa_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__11TBossTelesaFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b7b3c, 0, 0, 0), (u32)(&TBossTeleasa_perform_override));
	
	
	
	// TMapObjBall / Durians
	void TMapObjBall_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__14TMapObjGeneralFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d2db4, 0, 0, 0), (u32)(&TMapObjBall_perform_override));
	
	// TMapObjBall / Durians
	void TResetFruit_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__11TResetFruitFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d2bc4, 0, 0, 0), (u32)(&TResetFruit_perform_override));

	// TTelesa
	void TTelesa_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__7TTelesaFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b35c0, 0, 0, 0), (u32)(&TTelesa_perform_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b3778, 0, 0, 0), (u32)(&TTelesa_perform_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b3930, 0, 0, 0), (u32)(&TTelesa_perform_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b3ae8, 0, 0, 0), (u32)(&TTelesa_perform_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b3ca0, 0, 0, 0), (u32)(&TTelesa_perform_override));

	// TElecCarapace / Electrokoopa shell
	void TElecCarapace_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__13TElecCarapaceFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b2f70, 0, 0, 0), (u32)(&TElecCarapace_perform_override));
	
	// Hanking electrokoopas
	void TAmiNoko_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__8TAmiNokoFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bb2f0, 0, 0, 0), (u32)(&TAmiNoko_perform_override));
	
	// snooza koopas (pinna 4)
	void TTamaNoko_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__9TTamaNokoFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b772c, 0, 0, 0), (u32)(&TTamaNoko_perform_override));

	// TDangoHamuKuri / Stackin stus
	void TDangoHamuKuri_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__14TDangoHamuKuriFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b032c, 0, 0, 0), (u32)(&TDangoHamuKuri_perform_override));
	
	// Fire chain chomp (pianta 1)
	void TFireWanwan_perform_override(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__11TFireWanwanFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b3fa8, 0, 0, 0), (u32)(&TFireWanwan_perform_override));
	
	// Boss chain chomp (pianta 1)
	void TBossWanwan_perform_override(TSpineEnemy* enemy, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&enemy->mTranslation);
			setActiveMario(marioId);
			setCamera(marioId);
		}

		perform__11TBossWanwanFUlPQ26JDrama9TGraphics(enemy, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b6198, 0, 0, 0), (u32)(&TBossWanwan_perform_override));

	// Description: Makes merrygoround check all players
	int merryGoRoundMarioCheck = 0;
	void TMerrygoround_control_override(void* merrygoround) {
		setActiveMario(merryGoRoundMarioCheck);
		control__13TMerrygoroundFv(merrygoround);
		setActiveMario(getActiveViewport());
		merryGoRoundMarioCheck = (merryGoRoundMarioCheck + 1) % getPlayerCount();
		
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d0b38, 0, 0, 0), (u32)(&TMerrygoround_control_override));

	// Description: Makes merrygoround check all players
	void TChangeStageMerrygoround_touchPlayer_override(void* changeStageMerrygoround, THitActor* player) {
		int playerId = getPlayerId((TMario*)player);
		setActiveMario(playerId);
		touchPlayer__24TChangeStageMerrygoroundFP9THitActor(changeStageMerrygoround, player);
		setActiveMario(getActiveViewport());
		
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803d0a54, 0, 0, 0), (u32)(&TChangeStageMerrygoround_touchPlayer_override));
	
	// Fix brick blocks
	THitActor* currentBrickBlockSender;
	void TBrickBlock_receiveMessage_override(void* brickBlock, THitActor* sender, u32 message) {
		currentBrickBlockSender = sender;
		receiveMessage__11TBrickBlockFP9THitActorUl(brickBlock, sender, message);
		currentBrickBlockSender = nullptr;
		setActiveMario(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803cb9f8, 0, 0, 0), (u32)(&TBrickBlock_receiveMessage_override));

	bool TMapObjBase_marioHeadAttack_brickBlock_override(TMapObjBase* mapObj) {
		if(currentBrickBlockSender) {
			int playerId = getPlayerId((TMario*)currentBrickBlockSender);
			setActiveMario(playerId);
		}

		return mapObj->marioHeadAttack();
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801c3500, 0, 0, 0), TMapObjBase_marioHeadAttack_brickBlock_override);
	
	// Fix moving blocks and things in secret
	bool TRailMapObj_checkMarioRiding_override(TRailMapObj* mapObj) {
		for(int i = 0; i < getPlayerCount(); ++i) {
			setActiveMario(i);
			if(mapObj->checkMarioRiding()) {
				setActiveMario(getActiveViewport());
				return true;
			}
		}
		setActiveMario(getActiveViewport());
		return false;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801f0f14, 0, 0, 0), TRailMapObj_checkMarioRiding_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x801f00c0, 0, 0, 0), TRailMapObj_checkMarioRiding_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x801f095c, 0, 0, 0), TRailMapObj_checkMarioRiding_override);
	SMS_PATCH_BL(SMS_PORT_REGION(0x801dfc30, 0, 0, 0), TRailMapObj_checkMarioRiding_override);

	// Pinna rail region 
	// Fix pinna rail y-cam
	void TCameraBck_setFrame(void* cameraBck, f32 param_1) {
		//OSReport("Setting cameraBck frame to %f \n", param_1 + 1238.0 * jetcoasterDemoCallbackCamera);
		setFrame__10TCameraBckFf(cameraBck, param_1 + 1238.0 * jetcoasterDemoCallbackCamera);
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80025c68, 0, 0, 0), TCameraBck_setFrame);

	u32 Camera_JetCoasterDemoCallback(CPolarSubCamera* camera, u32 param_1) {
		for(int i = 0; i < getPlayerCount(); ++i) {
			CPolarSubCamera* camera = getCameraById(i);
			setCamera(i);
			jetcoasterDemoCallbackCamera = i;
			// Add ctrl->mCurFrame = 1238; to anim
			JetCoasterDemoCallBack__FUlUl(camera, param_1);
		}
		jetcoasterDemoCallbackCamera = 0;
		setCamera(getActiveViewport());
		return 1;
	}

	void fireStartDemoCameraMechaBowser(TMarDirector* marDirector, char* param_1, TVec3f* param_2, u32 param_3, f32 param_4, bool param_5, void* param_6, u32 param_7, CPolarSubCamera* camera, void* param_9) {

		if(!hasTriggeredMechaBowserCutscene) {
			fireStartDemoCamera__12TMarDirectorFPCcPCQ29JGeometry8TVec3_f(marDirector, param_1, param_2, param_3, param_4, param_5, &Camera_JetCoasterDemoCallback, param_7, camera, param_9);
		}
		hasTriggeredMechaBowserCutscene = true;
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x80025bcc, 0, 0, 0), fireStartDemoCameraMechaBowser);

	
	void TRocket_attackToMario(JDrama::TActor* rocket) {
		int cm = getClosestMarioId(&rocket->mTranslation);
		if(canRocketMountMario[cm] == 0) return;
		setActiveMario(cm);

		TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
		char* canHaveRocket = (char*)((int)liveManager + 0x60);
		*canHaveRocket = canRocketMountMario[cm];

		attackToMario__7TRocketFv(rocket);

		canRocketMountMario[cm] = *canHaveRocket;

		setActiveMario(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bcd08, 0, 0, 0), (u32)(&TRocket_attackToMario));

	void TRocket_bind(JDrama::TActor* rocket, int __fd, void* __addr, void* __len) {
		int cm = getClosestMarioId(&rocket->mTranslation);
		setActiveMario(cm);
	
		TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
		char* canHaveRocket = (char*)((int)liveManager + 0x60);
		*canHaveRocket = canRocketMountMario[cm];

		bind__7TRocketFv(rocket);
		canRocketMountMario[cm] = *canHaveRocket;
		setActiveMario(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bcc6c, 0, 0, 0), (u32)(&TRocket_bind));

	void TRocket_calcRootMatrix(JDrama::TActor* rocket) {
		int cm = getClosestMarioId(&rocket->mTranslation);
		setActiveMario(cm);
		calcRootMatrix__7TRocketFv(rocket);
		setActiveMario(getActiveViewport());
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bcc60, 0, 0, 0), (u32)(&TRocket_calcRootMatrix));

	void TRocket_setDeadAnm(JDrama::TActor* rocket) {
		int cm = getClosestMarioId(&rocket->mTranslation);
		TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
		char* canHaveRocket = (char*)((int)liveManager + 0x60);
		*canHaveRocket = canRocketMountMario[cm];

		setDeadAnm__7TRocketFv(rocket);

		canRocketMountMario[cm] = *canHaveRocket;
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bccf4, 0, 0, 0), (u32)(&TRocket_setDeadAnm));

	u32 TNerveRocketFly_execute(TNerveBase<JDrama::TActor>* nerveRocketFly, TSpineBase<JDrama::TActor>* rocketSpineBase) {
		JDrama::TActor* rocket = rocketSpineBase->mTarget;
		int cm = getClosestMarioId(&rocket->mTranslation);
		TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
		char* canHaveRocket = (char*)((int)liveManager + 0x60);
		*canHaveRocket = canRocketMountMario[cm];

		u32 result = execute__15TNerveRocketFlyCFP24TSpineBase_1(nerveRocketFly, rocketSpineBase);
	
		canRocketMountMario[cm] = *canHaveRocket;
		return result;
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bcb8c, 0, 0, 0), (u32)(&TNerveRocketFly_execute));

	u32 TNerveRocketPossessedNozzle_execute(TNerveBase<JDrama::TActor>* nerveRocketFly, TSpineBase<JDrama::TActor>* rocketSpineBase) {
		JDrama::TActor* rocket = rocketSpineBase->mTarget;
		int cm = getClosestMarioId(&rocket->mTranslation);
		TLiveManager* liveManager = (TLiveManager*)*(int*)((int)rocket + 0x70);
		char* canHaveRocket = (char*)((int)liveManager + 0x60);
		*canHaveRocket = canRocketMountMario[cm];
	
		TApplication *app      = &gpApplication;
		TMarDirector *director = reinterpret_cast<TMarDirector *>(app->mDirector);
		auto* p1Gamepad = director->mGamePads[0];
		director->mGamePads[0] = director->mGamePads[cm];

		u32 result =  execute__27TNerveRocketPossessedNozzleCFP24TSpineBase_1(nerveRocketFly, rocketSpineBase);
	
		director->mGamePads[0] = p1Gamepad;
		canRocketMountMario[cm] = *canHaveRocket;
		return result;
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803bcb9c, 0, 0, 0), (u32)(&TNerveRocketPossessedNozzle_execute));

	// End pinna rail region
	
	// Eel
	// Send message to closest mario

	void TBEelTears_perform_override(THitActor* tear, u32 flags, JDrama::TGraphics* graphics) {
		int originalMario = getPlayerId(gpMarioOriginal);
		int marioId = getClosestMarioId(&tear->mTranslation);
		setActiveMario(marioId);
		setCamera(marioId);
		
		perform__10TBEelTearsFUlPQ26JDrama9TGraphics(tear, flags, graphics);
		
		setActiveMario(originalMario);
		setCamera(originalMario);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b960c, 0, 0, 0), (u32)(&TBEelTears_perform_override));
	SMS_WRITE_32(SMS_PORT_REGION(0x803b94a4, 0, 0, 0), (u32)(&TBEelTears_perform_override));


	void SMS_SendMessageToBothMario(THitActor* eel, u32 msg) {
		for(int i = 0; i < getPlayerCount(); ++i) {
			getMario(i)->receiveMessage(eel, msg);
		}
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x800d0ebc, 0, 0, 0), SMS_SendMessageToBothMario);
	SMS_PATCH_BL(SMS_PORT_REGION(0x800d0f64, 0, 0, 0), SMS_SendMessageToBothMario);
	SMS_PATCH_BL(SMS_PORT_REGION(0x800d0f80, 0, 0, 0), SMS_SendMessageToBothMario);
	
	u32 TNerveBossEelDie_execute(TNerveBase<JDrama::TActor>* eel, TSpineBase<TTakeActor>* eelSpineBase) {
		bossEel = eelSpineBase->mTarget;
		if(!eelFirstDie) {
			SMS_SendMessageToBothMario((THitActor*)eelSpineBase->mTarget, 0xe);
			for(int i = 0; i < getPlayerCount(); ++i) {
				TMario* mario = getMario(i);
				mario->mWaterHealth = mario->mMaxWaterHealth;
			}


			eelFirstDie = true;
		}

		return execute__16TNerveBossEelDieCFP24TSpineBase_1(eel, eelSpineBase);
	}
	
	SMS_WRITE_32(SMS_PORT_REGION(0x803b8d98, 0, 0, 0), (u32)(&TNerveBossEelDie_execute));
	
	u32 TNerveBossEelEat_execute(TNerveBase<JDrama::TActor>* eel, TSpineBase<TTakeActor>* eelSpineBase) {
		bossEel = eelSpineBase->mTarget;
		return execute__16TNerveBossEelEatCFP24TSpineBase_1(eel, eelSpineBase);
	
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803b8da8, 0, 0, 0), (u32)(&TNerveBossEelEat_execute));
	
	SMS_WRITE_32(SMS_PORT_REGION(0x8025d7d8, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x8025d7dc, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x8025d7e0, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x8025d7e4, 0, 0, 0), 0x7fe3fb78);
	
	Mtx44* getHolderTakingMtx(TMario* mario) {
		if(mario->mHolder) {
			return mario->mHolder->getTakingMtx();
		}
		if(bossEel) {
			return bossEel->getTakingMtx();
		}
		return mario->getTakingMtx(); // Idk what else to put here lmao
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8025d7e8, 0, 0, 0), getHolderTakingMtx);

	// End eel

	bool mantaEscapesFrom[MARIO_COUNT];
	// Fixes manta escape
	void TBossMantaManager_updateMantaEscape(void* bossMantaManager) {
		
		bool shouldEscape = true;
		for(int i = 0; i < getPlayerCount(); ++i) {
			setActiveMario(i);
			updateMantaEscape__17TBossMantaManagerFv(bossMantaManager);
			mantaEscapesFrom[i] = *sMantaEscapeFromMario;

			if(!(*sMantaEscapeFromMario)) {
				shouldEscape = false;
			}
		}

		*sMantaEscapeFromMario = shouldEscape;
		setActiveMario(getActiveViewport());
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x8010fe18, 0, 0, 0), TBossMantaManager_updateMantaEscape);
	
	void TBossManta_perform(JDrama::TPlacement* placement, u32 performFlags, JDrama::TGraphics* graphics) {
		if(performFlags & 0x1) {
			int marioId = getClosestMarioId(&placement->mTranslation);
			if(mantaEscapesFrom[marioId]){
				for(int i = 0; i < getPlayerCount(); ++i) {
					if(!mantaEscapesFrom[i]) marioId = i;
				}
			}
			setActiveMario(marioId);
			setCamera(marioId);
		}
		perform__11TSpineEnemyFUlPQ26JDrama9TGraphics(placement, performFlags, graphics);
		
		if(performFlags & 0x1) {
			setActiveMario(getActiveViewport());
			setCamera(getActiveViewport());
		}
	}
	// TBossManta / Manta
	SMS_WRITE_32(SMS_PORT_REGION(0x803bd1bc, 0, 0, 0), (u32)(&TBossManta_perform));
}