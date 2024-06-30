#pragma once

namespace SMSCoop {
	void setActiveMarioArchive(int id);
	void unmountActiveMarioArchive();
	
	void initCharacterArchives(TMarDirector *director);
	void setSkinForPlayer(int id, char const* path, bool hasAnimations, int voiceType, int moveType);
	bool hasCustomAnimations(int id);
	int getVoiceType(int id);
	int getMoveType(int id);
}