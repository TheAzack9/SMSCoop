#pragma once

#include <SMS/System/MarDirector.hxx>
#include <SMS/System/Application.hxx>

namespace SMSCoop {
	void setupPlayers(TMarDirector *director);
	int getPlayerCount();
	void setActiveMario(int id);
	TMario* getMarioById(int id);
	int getClosestMarioId(TVec3f* position);
	u8 getPlayerId(TMario* mario);
	void updateCoop(TMarDirector* mardirector);
	bool cleanupPlayersCoop(TApplication* application);
	void setWaterColorForMario(int id);
}