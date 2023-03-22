#include <Dolphin/types.h>
#include <Dolphin/CARD.h>
#include <Dolphin/math.h>
#include <Dolphin/OS.h>
#include <Dolphin/string.h>

#include <JSystem/J2D/J2DOrthoGraph.hxx>
#include <JSystem/J2D/J2DTextBox.hxx>

#include <SMS/System/Application.hxx>

#include <BetterSMS/game.hxx>
#include <BetterSMS/module.hxx>
#include <BetterSMS/stage.hxx>
#include <BetterSMS/loading.hxx>
#include <BetterSMS/settings.hxx>
#include <BetterSMS/player.hxx>
#include "characters.hxx"
#include "player.hxx"

static void initModule() {
    OSReport("Initializing Module...\n");

    //// Register callbacks
    BetterSMS::setDebugMode(true);
    BetterSMS::Stage::registerInitCallback("setupPlayersCoop", setupPlayers);
    BetterSMS::Stage::registerUpdateCallback("updateCoop", updateCoop);
    BetterSMS::Stage::registerInitCallback("initCharacterArchivesCoop", initCharacterArchives);
}

static void deinitModule() {
    OSReport("Deinitializing Module...\n");

    //// Cleanup callbacks
    BetterSMS::Stage::deregisterInitCallback("setupPlayersCoop");
    BetterSMS::Stage::deregisterUpdateCallback("updateCoop");
    BetterSMS::Stage::deregisterInitCallback("initCharacterArchivesCoop");
}

// Definition block
KURIBO_MODULE_BEGIN("SMS Coop", "theAzack9", "v1.0") {
    // Set the load and unload callbacks to our registration functions
    KURIBO_EXECUTE_ON_LOAD { initModule(); }
    KURIBO_EXECUTE_ON_UNLOAD { deinitModule(); }
}
KURIBO_MODULE_END()
