#pragma once

#include <SMS/System/MarDirector.hxx>

void setupPlayers(TMarDirector *director);
int getPlayerCount();
void setActiveMario(int id);
void updatePlayerCoop(TMario *player, bool isRealMario);