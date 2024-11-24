#pragma once

#include <SMS/System/Application.hxx>
#include <SMS/Player/Mario.hxx>

namespace SMSCoop {
	int isSingleplayerLevel();
	int isSingleCameraLevel();
	bool isFocusedCamera();
	u8 getFocusedPlayer();
	void setFocusedPlayer(u8 playerId);
	int getPlayerCount();
	void setActiveMario(int id);
	bool isMarioSet();
	TMario* getMario(int id);
	u8 getPlayerId(TMario* mario);
	int getClosestMarioId(TVec3f* position);
	void setPlayerSkin(TMarDirector* director);
}