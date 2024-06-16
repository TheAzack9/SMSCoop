#pragma once

#include <SMS/System/MarDirector.hxx>
namespace SMSCoop {

    struct ASTex {
        u32 _pad[4];
        JUTTexture* texture;
    };

    ASTex* getScreenTextureForPlayer(int playerId);

	int getActiveViewport();
	void setViewport(int player);
	void resetSplitScreen(TMarDirector* director);
	void pushToPerformList(JDrama::TViewObj *obj, u32 flag);
}