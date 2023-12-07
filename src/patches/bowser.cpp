#include <sdk.h>
#include <macros.h>
#include <raw_fn.hxx>

#include "players.hxx"
#include "splitscreen.hxx"

// TODO: reset this
bool isBowserFight = false;

namespace SMSCoop {
	// Description: Setup collision for both marios every frame
	// Reason: Allow p2 to collide with platform separately from p1
	void TBathtub_setupCollisions_Override(void* tBathtub) {
		isBowserFight = true;
		for (int i = getPlayerCount()-1; i >= 0; --i) {
			setActiveMario(i);
			setupCollisions___8TBathtubFv(tBathtub);
		}
		setActiveMario(getActiveViewport());
	}
	SMS_PATCH_BL(SMS_PORT_REGION(0x801fb0a4, 0, 0, 0), TBathtub_setupCollisions_Override);
	
	// Description: Disable the remove collision function entierly
	// NB: Might have unknown consequences, should test this further...
	// Reason: Enable collision for p2 in bowser fight
	// NB: Used for e.g shrinking of spring 
	int TMapCollisionBase_remove(void* m_this) {
		if(isBowserFight) {
			return (int) m_this;
		} else {
			return remove__17TMapCollisionBaseFv(m_this);
		}
	}

	// Override vtable
	SMS_WRITE_32(SMS_PORT_REGION(0x803c171c, 0, 0, 0), (u32)(&TMapCollisionBase_remove));
	SMS_WRITE_32(SMS_PORT_REGION(0x803c1764, 0, 0, 0), (u32)(&TMapCollisionBase_remove));
	
	// Description: Switch between which mario is active when updating TBathWaterManager
	// Reason: Allow p2 to collide with bathwater
	void TBathWaterManager_perform_override(void* tBathwaterManager, u32 param_1, void* tGraphics) {
		// is updating and not draw	
		if((param_1 & 1) != 0) {
			int ap = getActiveViewport();
			for(int i = 0; i < getPlayerCount(); ++i) {
				if(i == ap) continue;
				setActiveMario(i);
				perform__17TBathWaterManagerFUlPQ26JDrama9TGraphics(tBathwaterManager, param_1, tGraphics);
			}
			setActiveMario(ap);
			perform__17TBathWaterManagerFUlPQ26JDrama9TGraphics(tBathwaterManager, param_1, tGraphics);
			// Ensure that primary mario is set correct again
		} else {
			perform__17TBathWaterManagerFUlPQ26JDrama9TGraphics(tBathwaterManager, param_1, tGraphics);
		}

	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c27a8, 0, 0, 0), (u32)(&TBathWaterManager_perform_override));
	

	// Fix collision for p2 in bowser fight
	SMS_WRITE_32(SMS_PORT_REGION(0x801fa7c8, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x801fa7d4, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x801fa7e0, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x801fa7ec, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x801fa7f8, 0, 0, 0), 0x60000000);
	SMS_WRITE_32(SMS_PORT_REGION(0x801440c4, 0, 0, 0), 0x60000000);
	

}