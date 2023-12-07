#pragma once

#include <SMS/System/Application.hxx>
#include <SMS/Player/Mario.hxx>

namespace SMSCoop {
	int isSingleplayerLevel();
	int getPlayerCount();
	void setActiveMario(int id);
	TMario* getMario(int id);
	u8 getPlayerId(TMario* mario);
	int getClosestMarioId(TVec3f* position);
}