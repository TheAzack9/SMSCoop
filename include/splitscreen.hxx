#pragma once

#include <SMS/System/MarDirector.hxx>
namespace SMSCoop {
	int getActiveViewport();
	void setViewport(int player);
	void resetSplitScreen(TMarDirector* director);
	void pushToPerformList(JDrama::TViewObj *obj, u32 flag);
}