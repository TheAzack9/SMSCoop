#pragma once

namespace SMSCoop {
	void setActiveMarioArchive(int id);
	void unmountActiveMarioArchive();
	
	void initCharacterArchives(TMarDirector *director);
	void setSkinForPlayer(
        int id,
        char const* archivePath = "/data/mario.arc",
        char const* uiIconPath = "/game_6/timg/mario_icon.bti",
        char const* uiTextPath = "/game_6/timg/mario_text.bti",
        char const* guideIcon = "/guide/timg/guide_mario.bti",
        char const* guideCursor1 = "/guide/timg/guide_cursor_1.bti",
        char const* guideCursor2 = "/guide/timg/guide_cursor_2.bti"
	);

    const char* getUiIconPath(int id);
    const char* getUiTextPath(int id);
    const char* getGuideIcon(int id);
    const char* getGuideCursor1(int id);
    const char* getGuideCursor2(int id);
}