#pragma once

namespace SMSCoop {
	void updateShineTimer(TMarDirector* marDirector);
	bool isShineGot();
	void resetShineLogic(TMarDirector* marDirector);
	int getMarioThatPickedShine();
	void setShineCutscene(bool setIsShineCutscene);
}