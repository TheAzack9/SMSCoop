#pragma once

#include <SMS/Player/Mario.hxx>

namespace SMSCoop {
	int getPlayerCount();
	void setActiveMario(int id);
	TMario* getMario(int id);
	u8 getPlayerId(TMario* mario);
	int getClosestMarioId(TVec3f* position);
}