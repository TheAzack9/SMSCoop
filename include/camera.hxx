#pragma once
#include <SMS/Camera/PolarSubCamera.hxx>

namespace SMSCoop {
	void setCamera(int i);
	CPolarSubCamera* getCameraById(int i);
	void startDemoCameraCoOp(const char* camera_name, const TVec3f *unk1, s32 unk2, f32 unk3, bool unk4);
}