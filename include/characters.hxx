#pragma once

namespace SMSCoop {
	void setActiveMarioArchive(int id);
	void unmountActiveMarioArchive();
	
	void initCharacterArchives(TMarDirector *director);
	void setSkinForPlayer(int id, char const* path);
}