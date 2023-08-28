#pragma once

#include <BetterSMS/module.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>

namespace SMSCoop {
    inline void setCamera(int i) {
        auto _setCamera =
            BetterSMS::getExportedFunctionPointer<void (*)(int)>("setCamera__7SMSCoopFi");
        _setCamera(i);
    }
    inline CPolarSubCamera *getCameraById(int i) {
        auto _getCameraById = BetterSMS::getExportedFunctionPointer<CPolarSubCamera *(*)(int)>(
            "getCameraById__7SMSCoopFi");
        return _getCameraById(i);
    }
    inline int getActivePerspective() {
        auto _getActivePerspective =
            BetterSMS::getExportedFunctionPointer<int (*)()>("getActivePerspective__7SMSCoopFv");
        return _getActivePerspective();
    }
    inline void setSkinForPlayer(int id, char const *path) {
        auto _setSkinForPlayer = BetterSMS::getExportedFunctionPointer<void (*)(int, char const *)>(
            "setSkinForPlayer__7SMSCoopFiPCc");
        _setSkinForPlayer(id, path);
    }
    inline TMario *getMarioById(int id) {
        auto _getMarioById =
            BetterSMS::getExportedFunctionPointer<TMario *(*)(int)>("getMarioById__7SMSCoopFi");
        return _getMarioById(id);
    }
    inline int getClosestMarioId(TVec3f *position) {
        auto _getClosestMarioId = BetterSMS::getExportedFunctionPointer<int (*)(TVec3f *)>(
            "getClosestMarioId__7SMSCoopFPQ29JGeometry8TVec3<f>");
        return _getClosestMarioId(position);
    }
    inline u8 getPlayerId(TMario *mario) {
        auto _getPlayerId = BetterSMS::getExportedFunctionPointer<u8 (*)(TMario *)>(
            "getPlayerId__7SMSCoopFP6TMario");
        return _getPlayerId(mario);
    }
    inline int getPlayerCount() {
        auto _getPlayerCount =
            BetterSMS::getExportedFunctionPointer<int (*)()>("getPlayerCount__7SMSCoopFv");
        return _getPlayerCount();
    }
    inline void setActiveMario(int id) {
        auto _setActiveMario =
            BetterSMS::getExportedFunctionPointer<void (*)(int)>("setActiveMario__7SMSCoopFi");
        _setActiveMario(id);
    }
}  // namespace SMSCoop