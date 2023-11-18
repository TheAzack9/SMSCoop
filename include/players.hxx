#pragma once

#include <SMS/Player/Mario.hxx>

namespace SMSCoop {
	int getPlayerCount();
	void setActiveMario(int id);
	TMario* getMario(int id);
}