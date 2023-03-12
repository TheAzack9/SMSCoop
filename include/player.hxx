#pragma once

#include <SMS/System/MarDirector.hxx>

void setupPlayers(TMarDirector *director);
int getPlayerCount();
void setActiveMario(int id);
TMario* getMarioById(int id);
int getClosestMarioId(TVec3f* position);
u8 getPlayerId(TMario* mario);