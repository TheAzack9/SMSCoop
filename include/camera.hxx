#pragma once

#include <SMS/Camera/PolarSubCamera.hxx>

namespace SMSCoop {
	void setCamera(int i);
	CPolarSubCamera* getCameraById(int i);
	int getActivePerspective();
}
