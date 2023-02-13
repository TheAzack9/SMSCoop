#include <BetterSMS/stage.hxx>
#include <Dolphin/GX.h>
#include <Dolphin/OS.h>
#include <JSystem/JKernel/JKRDvdRipper.hxx>
#include <SMS/raw_fn.hxx>
#include <macros.h>
#include <sdk.h>
#include <string.h>

#include "TLuigi.hxx"

static void *marioArcBuf = NULL;
static void *luigiArcBuf = NULL;

static void *JKRDvdRipper_loadToMainRAM_Mario(const char *path, u8 *param_2, JKRExpandSwitch expand,
                                              u32 param_4, JKRHeap *heap,
                                              JKRDvdRipper::EAllocDirection direction, u32 param_7,
                                              int *param_8) {

    luigiArcBuf = JKRDvdRipper::loadToMainRAM("/data/luigi.szs", param_2, expand, param_4, heap,
                                              direction, param_7, param_8);
    marioArcBuf = JKRDvdRipper::loadToMainRAM("/data/mario.szs", param_2, expand, param_4, heap,
                                              direction, param_7, param_8);

    return marioArcBuf;
}
SMS_PATCH_BL(SMS_PORT_REGION(0x802a716c, 0, 0, 0), JKRDvdRipper_loadToMainRAM_Mario);
SMS_PATCH_BL(SMS_PORT_REGION(0x802a719c, 0, 0, 0), JKRDvdRipper_loadToMainRAM_Mario);

static int loadedMarios  = 0;
static int updatedMarios = 0;
static int modelToLoad   = 0;
static bool isLuigiModel = false;

static TMario *marios[2];
static int MarioOnYoshi[2];

static bool isLuigiObj = false;
//
//static int TMarioStrCmp_Override(const char *nameRef, void *str) {
//    if (strcmp(nameRef, "Luigi") == 0) {
//        isLuigiObj = true;
//        return 0;
//    }
//
//    return strcmp(nameRef, "Mario");
//}
//SMS_PATCH_BL(SMS_PORT_REGION(0x8029d7d8, 0, 0, 0), TMarioStrCmp_Override);

//
//static void TMario_Constructor(TMario* mario) {
//
//    if (isLuigiObj) {
//
//        TMario *luigi = new (*((char*)mario)) TMario;
//        isLuigiObj = false;
//        return;
//    }
//    TMario *mario2 = new (*((char *)mario)) TLuigi;
//}
//SMS_PATCH_BL(SMS_PORT_REGION(0x8029d7f8, 0, 0, 0), TMario_Constructor);


namespace Coop {

    void initPlayer(TMario *mario, bool isMario) {

        // TMario* mario2 = (TMario*)SearchObjByRef("MarMP2");
        // if(this == mario2) return;
        marios[loadedMarios] = mario;
        loadedMarios++;

        if (marios[1] != NULL) {
            /*  GamePadTwo->u2 = 2;
              Mario_SetGamePad(marios[1], &(GamePads[1]));*/
        }
        // TODO
        // initCameraForMario(mario, loadedMarios - 1);
    }

    void onStageInit(TMarDirector *tApplication) {

        loadedMarios = 0;
        modelToLoad  = 0;
        marios[0]    = NULL;
        marios[1]    = NULL;
    }

}  // namespace Coop

TLuigi::TLuigi() {

}

void TLuigi::initModel() {

    //if (this == marios[1]) {

    //    JKRMemArchive *marioVolume =
    //        reinterpret_cast<JKRMemArchive *>(JKRFileLoader::getVolume("mario"));
    //    marioVolume->unmountFixed();
    //    marioVolume->mountFixed(luigiArcBuf, UNK_0);
    //    isLuigiModel = true;
    //}
    TMario::initModel();
}
//
//void os_DCFlushRange_TMario_initModel(void *param_1, size_t param_2) {
//    if (isLuigiModel) {
//        JKRMemArchive *marioVolume =
//            reinterpret_cast<JKRMemArchive *>(JKRFileLoader::getVolume("mario"));
//        marioVolume->unmountFixed();
//        marioVolume->mountFixed(marioArcBuf, UNK_0);
//        isLuigiModel = false;
//    }
//
//    DCFlushRange(param_1, param_2);
//}
//SMS_PATCH_BL(SMS_PORT_REGION(0x802468b4, 0, 0, 0), os_DCFlushRange_TMario_initModel);