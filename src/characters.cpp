/// <summary>
/// Overrides logic for loading a player model. Also moves common logic to the common.szs archive to reduce memory size of modules.
/// This allows us to load different skins for each player.
/// Thanks to JoshuaMK for writing and helping with this.
/// </summary>

#include <Dolphin/types.h>
#include <Dolphin/printf.h>
#include <Dolphin/DVD.h>

#include <JSystem/JGadget/Vector.hxx>
#include <JSystem/JKernel/JKRDecomp.hxx>
#include <JSystem/JKernel/JKRDvdRipper.hxx>
#include <JSystem/JKernel/JKRHeap.hxx>
#include <JSystem/JKernel/JKRMemArchive.hxx>
#include <JSystem/JKernel/JKRDvdFile.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>
#include <SMS/System/Application.hxx>
#include <SMS/assert.h>
#include <SMS/macros.h>
#include <sdk.h>
#include <string.h>

namespace SMSCoop {

    struct CharacterInfo {
        const char* archivePath = "/data/mario.arc";
        const char* uiIconPath = "/game_6/timg/mario_icon.bti";
        const char* uiTextPath = "/game_6/timg/mario_text.bti";
        const char* guideIcon = "/guide/timg/guide_mario.bti";
        const char* guideCursor1 = "/guide/timg/guide_cursor_1.bti";
        const char* guideCursor2 = "/guide/timg/guide_cursor_2.bti";
    };

    static void* sCharacterArcs[2];
    static CharacterInfo infos[2];
    void* arcBufMario;

    void setSkinForPlayer(
        int id,
        char const* archivePath = "/data/mario.arc",
        char const* uiIconPath = "/game_6/timg/mario_icon.bti",
        char const* uiTextPath = "/game_6/timg/mario_text.bti",
        char const* guideIcon = "/guide/timg/guide_mario.bti",
        char const* guideCursor1 = "/guide/timg/guide_cursor_1.bti",
        char const* guideCursor2 = "/guide/timg/guide_cursor_2.bti"
    ) {
        if(id < 0 || id >= 2) {
            OSReport("Tried to set player skin outside max player bounds of 2\n");
        }
        infos[id].archivePath = archivePath;
        infos[id].uiIconPath = uiIconPath;
        infos[id].uiTextPath = uiTextPath;
        infos[id].guideIcon = guideIcon;
        infos[id].guideCursor1 = guideCursor1;
        infos[id].guideCursor2 = guideCursor2;
    }

    const char* getUiIconPath(int id) {
        if(id < 0 || id >= 2) {
            OSReport("Tried to get ui icon path, outside max bounds\n");
        }
        return infos[id].uiIconPath;
    }
    
    const char* getUiTextPath(int id) {
        if(id < 0 || id >= 2) {
            OSReport("Tried to get ui text path, outside max bounds\n");
        }
        return infos[id].uiTextPath;
    }
    
    const char* getGuideIcon(int id) {
        if(id < 0 || id >= 2) {
            OSReport("Tried to get guide icon path, outside max bounds\n");
        }
        return infos[id].guideIcon;
    }

    const char* getGuideCursor1(int id) {
        if(id < 0 || id >= 2) {
            OSReport("Tried to get guide cursor 1 path, outside max bounds\n");
        }
        return infos[id].guideCursor1;
    }

    const char* getGuideCursor2(int id) {
        if(id < 0 || id >= 2) {
            OSReport("Tried to get guide cursor 2 path, outside max bounds\n");
        }
        return infos[id].guideCursor2;
    }

	void unmountActiveMarioArchive() {
        JKRMemArchive *archive = reinterpret_cast<JKRMemArchive *>(JKRFileLoader::getVolume("mario"));
        archive->unmountFixed();
    }

    void setActiveMarioArchive(int id) {
        if(id >= 2 || id < 0) return;
        JKRMemArchive *archive = reinterpret_cast<JKRMemArchive *>(JKRFileLoader::getVolume("mario"));
        archive->unmountFixed();
        arcBufMario = sCharacterArcs[id];
        archive->mountFixed(arcBufMario, UNK_0);
    }

    SMS_WRITE_32(SMS_PORT_REGION(0x802A6C4C, 0, 0, 0), 0x60000000);  // Prevent early archive init
    SMS_WRITE_32(SMS_PORT_REGION(0x802A7148, 0, 0, 0), 0x48000058);
    SMS_WRITE_32(SMS_PORT_REGION(0x802A71A8, 0, 0, 0), 0x60000000);

    // TODO: This should be based on a configurable list that is exposed in the module
    void initCharacterArchives(TMarDirector *director) {
        void* marioArchive = SMSLoadArchive("/data/mario.arc", nullptr, 0, nullptr);
        for(int i = 0; i < 2; ++i) {
            sCharacterArcs[i] = nullptr;
            if(strcmp(infos[i].archivePath, "/data/mario.arc") == 0) {
                OSReport("Loading default mario skin %s\n", infos[i].archivePath);
                sCharacterArcs[i]= marioArchive;
                continue;
            }
            OSReport("Loading skin %s\n", infos[i].archivePath);
        
            // Load skins (if exists)
            void* playerArchive = SMSLoadArchive(infos[i].archivePath, nullptr, 0, nullptr);
            if(playerArchive != 0) {
                sCharacterArcs[i]= playerArchive;
            } else {
                sCharacterArcs[i]= marioArchive;
            }
        }

        arcBufMario = sCharacterArcs[0];

        JKRMemArchive *archive = reinterpret_cast<JKRMemArchive *>(JKRFileLoader::getVolume("mario"));
        if (!archive)
            archive = new JKRMemArchive(arcBufMario, 0, UNK_0);

    }
    static void getGlobalOrLocalResFmt(char *dst, size_t size, const char *local_path,
                                    const char *specifier, const char *global_path) {
        auto* first_file = JKRFileLoader::findFirstFile("/common/mario/01_waterboost");
        if (!first_file) {
            snprintf(dst, size, local_path, specifier);
            return;
        }
        delete first_file;
        snprintf(dst, size, global_path, specifier);
    }

    static void *getGlobalOrLocalRes(const char *local_path, const char *global_path) {
        auto* first_file = JKRFileLoader::findFirstFile("/common/mario/01_waterboost");
        if (!first_file) {
            return JKRFileLoader::getGlbResource(local_path);
        }
        delete first_file;
        return JKRFileLoader::getGlbResource(global_path);
    }

    static void getGlobalPlayerBoneAnim(char *dst, size_t size, const char *local_path, const char *specifier) {
        getGlobalOrLocalResFmt(dst, size, local_path, specifier, "/common/mario/bck/ma_%s.bck");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80246B2C, 0, 0, 0), getGlobalPlayerBoneAnim);

    static void getGlobalPlayerSoundAnim(char *dst, size_t size, const char *local_path,
                                         const char *specifier) {
        getGlobalOrLocalResFmt(dst, size, local_path, specifier, "/common/mario/bas/ma_%s.bas");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80246B60, 0, 0, 0), getGlobalPlayerSoundAnim);

    static void getGlobalPlayerTobiKomiAnim(MActorAnmData *data, const char *local_path) {
        auto* first_file = JKRFileLoader::findFirstFile("/common/mario/04_tobikomi");
        if (!first_file) {
            data->init(local_path, nullptr);
            return;
        }
        delete first_file;
        data->init("common/mario/04_tobikomi", nullptr);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8027211C, 0, 0, 0), getGlobalPlayerTobiKomiAnim);

    static void *getGlobalPlayerTobiKomiMdl(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/04_tobikomi/04_tobikomi.bmd");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80272160, 0, 0, 0), getGlobalPlayerTobiKomiMdl);

    static void getGlobalPlayerWaterAnim(MActorAnmData *data, const char *local_path) {
        auto* first_file = JKRFileLoader::findFirstFile("/common/mario/01_waterboost");
        if (!first_file) {
            data->init(local_path, nullptr);
            return;
        }
        delete first_file;
        data->init("common/mario/01_waterboost", nullptr);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802721F0, 0, 0, 0), getGlobalPlayerWaterAnim);

    static void *getGlobalPlayerTexPattern(const char *local_path) {
        char global_path[60];
        snprintf(global_path, 60, "%s%s", "/common", local_path);

        return getGlobalOrLocalRes(local_path, global_path);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80246be4, 0, 0, 0), getGlobalPlayerTexPattern);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80247528, 0, 0, 0), getGlobalPlayerTexPattern);

    static void *getGlobalPlayerWaterBoostMdl(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/01_waterboost/01_waterboost.bmd");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80272220, 0, 0, 0), getGlobalPlayerWaterBoostMdl);

    static void *getGlobalPlayerWaterRefTex(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/timg/waterref.bti");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80280330, 0, 0, 0), getGlobalPlayerWaterRefTex);

    static void *getGlobalPlayerWaterMaskTex(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/timg/waterMask.bti");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80280364, 0, 0, 0), getGlobalPlayerWaterMaskTex);

    static void *getGlobalPlayerWaterSpecTex(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/timg/waterSpec.bti");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8028039c, 0, 0, 0), getGlobalPlayerWaterSpecTex);

    static void *getGlobalPlayerWaterJumpingTex(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/timg/waterJumping.bti");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802803D4, 0, 0, 0), getGlobalPlayerWaterJumpingTex);

    static void *getGlobalPlayerSplashTex(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/timg/splash.bti");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8026707C, 0, 0, 0), getGlobalPlayerSplashTex);


    static void *getGlobalPlayerWaterShadowYukaMdl(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/bmd/water_shadow_yuka.bmd");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8028040C, 0, 0, 0), getGlobalPlayerWaterShadowYukaMdl);

    static void *getGlobalPlayerWaterShadowKabeMdl(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/bmd/water_shadow_kabe.bmd");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80280420, 0, 0, 0), getGlobalPlayerWaterShadowKabeMdl);

    static void *getGlobalPlayerWaterMaskMdl(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/bmd/watermask.bmd");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80280434, 0, 0, 0), getGlobalPlayerWaterMaskMdl);

    static void *getGlobalPlayerWaterHideYukaSMdl(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/bmd/water_hide_yuka_s.bmd");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80280448, 0, 0, 0), getGlobalPlayerWaterHideYukaSMdl);

    static void *getGlobalPlayerWaterHideKabeSMdl(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/bmd/water_hide_kabe_s.bmd");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8028045C, 0, 0, 0), getGlobalPlayerWaterHideKabeSMdl);

    static void *getGlobalPlayerWaterDiverHelmMdl(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/watergun2/body/diver_helm.bmd");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802423B0, 0, 0, 0), getGlobalPlayerWaterDiverHelmMdl);

    static void getGlobalPlayerWaterGunAnim(MActorAnmData *data, const char *local_path) {
        auto* first_file = JKRFileLoader::findFirstFile("/common/mario/01_waterboost");
        if (!first_file) {
            data->init(local_path, nullptr);
            return;
        }
        delete first_file;

        char buffer[64];
        snprintf(buffer, 64, "/common/%s", local_path+1);  // Replace /mario/ with /common/

        data->init(buffer, nullptr);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8026A6E8, 0, 0, 0), getGlobalPlayerWaterGunAnim);

    static void *getGlobalPlayerWaterGunMdl(const char *local_path) {
        auto* first_file = JKRFileLoader::findFirstFile("/common/mario/01_waterboost");
        if (!first_file) {
            return JKRFileLoader::getGlbResource(local_path);
        }
        delete first_file;
        char buffer[64];
        snprintf(buffer, 64, "/common/%s", local_path+1);  // Replace /mario/ with /common/

        return JKRFileLoader::getGlbResource(buffer);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8026A71C, 0, 0, 0), getGlobalPlayerWaterGunMdl);

    static void getGlobalPlayerWaterGunBodyAnim(MActorAnmData *data, const char *local_path) {
        auto* first_file = JKRFileLoader::findFirstFile("/common/mario/01_waterboost");
        if (!first_file) {
            data->init(local_path, nullptr);
            return;
        }
        delete first_file;
        data->init("common/mario/watergun2/body", nullptr);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8026A550, 0, 0, 0), getGlobalPlayerWaterGunBodyAnim);

    static void *getGlobalPlayerWaterGunBodyMdl(const char *local_path) {
        return getGlobalOrLocalRes(local_path, "/common/mario/watergun2/body/wg_mdl1.bmd");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8026A578, 0, 0, 0), getGlobalPlayerWaterGunBodyMdl);
}