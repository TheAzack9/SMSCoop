#pragma once

namespace SMSCoop {
	void initPvp(TMarDirector* marDirector);
	void updatePvp(TMarDirector* marDirector);
	void drawPvp();
	bool isPvpLevel();
	void onDeathPvp();
	void touchPlayerPvp(TMario* playerTouching, TMario* playerTouched);
}