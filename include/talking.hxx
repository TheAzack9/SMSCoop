#pragma once

#include <SMS/System/MarDirector.hxx>

namespace SMSCoop {
	void resetTalking(TMarDirector *director);
	void updateTalking(TMarDirector *director);
	void TMarDirector_movement_game_override(TMarDirector* marDirector);
	bool isTalking();
	int getTalkingPlayer();
}