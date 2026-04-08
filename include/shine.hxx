#pragma once

#include <SMS/System/MarDirector.hxx>

namespace SMSCoop {
	void updateShineTimer(TMarDirector* marDirector);
	void resetShineLogic(TMarDirector* marDirector);
	bool isShineGot();
	int getMarioThatPickedShine();

	void setShineCutscene(bool setIsShineCutscene);
}