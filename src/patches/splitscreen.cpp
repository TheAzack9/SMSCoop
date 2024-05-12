#include <sdk.h>
#include <types.h>
#define NTSCU
#include <raw_fn.hxx>
#include <string.h>

#include <SMS/System/Application.hxx>
#include <JSystem/JDrama/JDRGraphics.hxx>
#include <JSystem/JDrama/JDRRect.hxx>
#include <JSystem/JDrama/JDRViewport.hxx>
#include <SMS/Strategic/ObjHitCheck.hxx>
#include <JUtility/JUTTexture.hxx>
#include <Dolphin/GX.h>
#include <VI.h>
#include <SMS/Camera/SunModel.hxx>
#include <SMS/Enemy/Conductor.hxx>
#include <SMS/Camera/CubeManagerBase.hxx>
#include <JDrama/JDRViewObjPtrListT.hxx>
#include <SMS/MarioUtil/DrawUtil.hxx>
#include <JSystem/JDrama/JDRViewObjPtrListT.hxx>

#include <Dolphin/MTX.h>
#include <BetterSMS/libs/global_list.hxx>

#include "camera.hxx"
#include "players.hxx"
#include "talking.hxx"
#include "yoshi.hxx"

static u32* gpSilhouetteManager = (u32*)0x8040E090;
static u32* gpQuestionManager = (u32*)0x8040e088;

// FIXME: Support horizontal split screen
// FIXME: 3/4 perspectives?
namespace SMSCoop {
    JDrama::TGraphics* graphicsPointer;
    JDrama::TViewport* gViewport;
    JDrama::TViewport* gScreen2DViewport;

    struct BufferObj {
        BufferObj(JDrama::TViewObj* obj, u32 flag) : m_obj(obj), m_flag(flag) {
        }
        JDrama::TViewObj* m_obj;
        u32 m_flag;
    };

    static TPerformList* g_objectsToUpdate;
    static BetterSMS::TGlobalList<BufferObj> g_performListBuffer;

    void* g_sun;
    TSunModel* g_sunModel;
    void* g_sunLensFlare;
    void* g_sunLensGlow;

    int perspective = 0;
    
    // Description: Set active viewport for player
    void setViewport(int player) {
        perspective = player;
        if(player == 0) {
            gViewport->mViewportRect.mX1 = 0;
            gViewport->mViewportRect.mX2 = 320;
        } else {
            gViewport->mViewportRect.mX1 = 320;
            gViewport->mViewportRect.mX2 = 640;
        }
        gViewport->perform(0x88, graphicsPointer);
        
        if(gScreen2DViewport) {
            if(player == 0) {
                gScreen2DViewport->mViewportRect.mX1 = 0;
                gScreen2DViewport->mViewportRect.mX2 = 320;
            } else {
                gScreen2DViewport->mViewportRect.mX1 = 320;
                gScreen2DViewport->mViewportRect.mX2 = 640;
            }
            gScreen2DViewport->perform(0x88, graphicsPointer);
        }

    }

    int getActiveViewport() {
        // Can be removed once mario is p1 again
        if(isSingleplayerLevel()) return 0;
        return perspective;
    }

    void resetSplitScreen(TMarDirector* director) {
        void* g_sun = nullptr;
        TSunModel* g_sunModel = nullptr;
        void* g_sunLensFlare = nullptr;
        void* g_sunLensGlow = nullptr;
        g_objectsToUpdate = nullptr;
        //g_objectsToUpdate->Erase(g_objectsToUpdate->begin(), g_objectsToUpdate->end());
    }

    void pushToPerformList(JDrama::TViewObj *obj, u32 flag) {
        if(g_objectsToUpdate) {
            g_objectsToUpdate->push_back(obj, flag);
        } else {
            g_performListBuffer.push_back(BufferObj(obj, flag));
        }
    }

    int searchF_performList_movement(void* perfListGroup, u32 keyCode, char* name) {
        u16 mkeyCode = JDrama::TNameRef::calcKeyCode("Player 2 PfLst");
        g_objectsToUpdate = (TPerformList*)searchF__Q26JDrama55TNameRefPtrListT_Q(perfListGroup, mkeyCode, "Player 2 PfLst");

        for(auto& obj : g_performListBuffer) {
            g_objectsToUpdate->push_back(obj.m_obj, obj.m_flag);
        }
        g_performListBuffer.clear();
        
        return searchF__Q26JDrama55TNameRefPtrListT_Q(perfListGroup, keyCode, name);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802b9404, 0, 0, 0), searchF_performList_movement);

    // Description: Get's an instance of the 3d viewport
    // FIXME: Get this from name ref instead
    static void TNameRefGe_getNameRef_createViewport(JDrama::TViewport* viewport, JDrama::TRect* rect, const char* name) {
        gViewport = viewport;
        __ct__Q26JDrama9TViewportFRCQ26JDrama5TRectPCc(viewport, rect, name);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802fb52c, 0, 0, 0), TNameRefGe_getNameRef_createViewport);
    
    // Description: Get's an instance of the ui viewport
    // FIXME: Get this from name ref instead
    static void Create_screen2D_Viewport_initECDisplay(JDrama::TViewport* viewport, JDrama::TRect* rect, const char* name) {
        gScreen2DViewport = viewport;
        __ct__Q26JDrama9TViewportFRCQ26JDrama5TRectPCc(viewport, rect, name);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8029b4b4, 0, 0, 0), Create_screen2D_Viewport_initECDisplay);
    //
    // Description: Update mirror camera to be corret player
    static void TMirrorCamera_constructor_override(JDrama::TViewObj* mirrorCamera, char* name) {
        __ct__13TMirrorCameraFPCc(mirrorCamera, name);    
        pushToPerformList(mirrorCamera, 0x14);
    }

    static void TSky_load_override(JDrama::TViewObj* sky, void* memStream) {
        load__4TSkyFR20JSUMemoryInputStream(sky, memStream);
        pushToPerformList(sky, 0x2);
    }
	SMS_WRITE_32(SMS_PORT_REGION(0x803c2020, 0, 0, 0), (u32)(&TSky_load_override));

    // Description: Sets the viewport, mostly used to get graphics pointer pt
    // FIXME: Get Graphics pointer in a better way
    static void TViewport_perform_setViewport(JDrama::TGraphics* graphics, JDrama::TRect& rect, f32 far, f32 near) { 
        graphicsPointer = graphics;
        graphics->setViewport(rect, far, near);
        graphicsPointer->mViewPortSpace.mX1 = 0;
        graphicsPointer->mViewPortSpace.mX2 = 640;
        graphicsPointer->mViewPortSpace.mY1 = 0;
        graphicsPointer->mViewPortSpace.mY2 = 448;
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802fcdc0, 0, 0, 0), TViewport_perform_setViewport);

    // TODO: Rewrite this override EfbCtrlTex_perform instead and store a reference to the EfbCtrlTex that should
    // be changed to correct size. Checking the width is very hacky
    static void setTexCopySrcEfbTex(u16 left, u16 top, u16 wd, u16 ht) {
        if(!isSingleplayerLevel()) {
            if(wd == 640 && perspective == 0) {
                left = 0;
                wd = 320;
            }
            if(wd == 640 && perspective == 1) {
                left = 320;
                wd = 320;
            }
        }

        GXSetTexCopySrc(left, top, wd, ht);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802f8d48, 0, 0, 0), setTexCopySrcEfbTex);

    // Description: Override TScreenTexture to be half the width for split screen
    // Note: ScreenTexture is already half size for memory optimization? (Or perhaps blurring. Done by sunshine devs)
    static void TScreenTexture_load_ct_JUTTexture(JUTTexture* texture, int width, int height, u32 fmt) {
        if(isSingleplayerLevel()) {
            __ct__10JUTTextureFii9_GXTexFmt(texture, width, height, fmt);
        } else {
            __ct__10JUTTextureFii9_GXTexFmt(texture, 640/2 / 2, 448/2, fmt);
        }
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8022d4d0, 0, 0, 0), TScreenTexture_load_ct_JUTTexture);

    

    // Description: Add a perform to other players, fixing reflection camera.
	void TMirrorModelManager_load_override(JDrama::TViewObj* mirrorModelManager, void* memStream) {
        load__19TMirrorModelManagerFR20JSUMemoryInputStream(mirrorModelManager, memStream);
		pushToPerformList(mirrorModelManager, 0x3);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803c19ac, 0, 0, 0), (u32)&TMirrorModelManager_load_override);

    
	void TMirrorActor_load_override(JDrama::TViewObj* mirrorActor, void* memStream) {
        load__Q26JDrama8TNameRefFR20JSUMemoryInputStream(mirrorActor, memStream);
		pushToPerformList(mirrorActor, 0x3);
	}
	SMS_WRITE_32(SMS_PORT_REGION(0x803da5e0, 0, 0, 0), (u32)&TMirrorActor_load_override);
 //   void TMario_calcView_override(TMario* mario, JDrama::TGraphics* graphics) {
 //       Mtx j3dSysCopy;
 //       PSMTXCopy(*j3dSysPtr, j3dSysCopy);
 //       PSMTXCopy(getCameraById(0)->mTRSMatrix, *j3dSysPtr);
 //       mario->calcView(graphics);
 //       PSMTXCopy(j3dSysCopy, *j3dSysPtr);
 //   }
 //   SMS_PATCH_BL(SMS_PORT_REGION(0x8024d5a0, 0, 0, 0), TMario_calcView_override);
	//SMS_WRITE_32(SMS_PORT_REGION(0x802446ec, 0, 0, 0), 0x60000000);
    
    //void TMapBaseObj_perform_liveActor_override(TLiveActor* actor, u32 flags, JDrama::TGraphics* graphics) {
    //    //if((flags & 0x200) == 0) {
    //        //performOnlyDraw__10TLiveActorFUlPQ26JDrama9TGraphics(actor, flags, graphics);
    //        perform__10TLiveActorFUlPQ26JDrama9TGraphics(actor, flags, graphics);
    //    //}
    //}
    //SMS_PATCH_BL(SMS_PORT_REGION(0x801aff50, 0, 0, 0), TMapBaseObj_perform_liveActor_override);
    // Trying to figure out what each perform flag does
    // TViewObj also has dynamic check with 0xc (display flag)
    // 0x1 = update game logic
    // 0x2 = update animation (i think)
    // 0x4 = Some kind of FX?
    // 0x8 = draw?
    // 0x10 = after effects
    // 0x14 = camera
    // 0x20 = colors / lights
    // 0x88 = viewport
    // 0x80 = efb textures (/ draw buffers)
    // 0x200 = ? (TSmJ3dAct + M3UModel)  (crashes when run twice)
    // 0x400 = DrawBufObj setting global flags
    // 0x1000 = TViewObj filteR?
    // 0x2000 = TViewObj filteR?
    // 0x1000000 = = (TMario)
    // 0x2000000 = = (TMario) + related to updating goop stamping
    // 0x4000000 = = (TMario)
    // 0x8000000 = = (TMario)
    // 0x10000000 = = (TMario)
    // 0x40000000 = = (TMario)
    // 0x20000000 = = (TMario)
    // 0x80000000 = = (TMario)
    
    // TODO: Add option for setting far clip multiplier to reduce lag
    void testing(f32 fov, f32 aspect, f32 near, f32 far) {
        SetViewFrustumClipCheckPerspective(fov, aspect, near, far);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8021b0b0, 0, 0, 0), testing);

    void* gx_GXInit_alloc_override(u32 size, int flags, JKRHeap* heap) {
        return JKRHeap::alloc(0x40000, flags, heap);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802a7488, 0, 0, 0), gx_GXInit_alloc_override);

    void gx_GXInit_override(void* heap, u32 size) {
        GXInit(heap, 0x40000);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x802a7490, 0, 0, 0), gx_GXInit_override);
    
    void printList(JGadget::TList<TLiveActor *>& list, int type = 0) {
        
        auto it2 = list.begin();
        auto end2 = list.end();
        while(it2 != end2) {
            JDrama::TViewObj* obj2 = reinterpret_cast<JDrama::TViewObj*>(*it2);
            if(type == 1) { // Alone actors (whatever that means)
                //TLiveActor* liveActor = reinterpret_cast<TLiveActor*>(*it2);
                //liveActor->mStateFlags.asFlags.mCullModel = true;
                obj2->mPerformFlags |= 0x4;
            } else if (type == 2) { // TLiveManager

                //TMario* currentMario = getMario(0);
                //Vec marioPos;
                //marioPos.x = currentMario->mTranslation.x;
                //marioPos.y = currentMario->mTranslation.y + 75.0;
                //marioPos.z = currentMario->mTranslation.z;
                ////gpCubeArea->mCurrentCube = gpCubeArea->getInCubeNo(marioPos);
                //gpCubeFastA->mCurrentCube = gpCubeFastA->getInCubeNo(marioPos);
                //gpCubeFastB->mCurrentCube = gpCubeFastB->getInCubeNo(marioPos);
                //gpCubeFastC->mCurrentCube = gpCubeFastC->getInCubeNo(marioPos);
                ////OSReport("Testing %X %X %X %X \n", (u32)&gpCubeFastA->mCurrentCube - (u32)gpCubeFastA, gpCubeFastA->mCurrentCube, gpCubeFastB->mCurrentCube, gpCubeFastC->mCurrentCube);

                TLiveManager* manager = reinterpret_cast<TLiveManager*>(*it2);
                //manager->perform(0x2, graphicsPointer);
                /*CPolarSubCamera* camera = getCameraById(0);
                PSMTXCopy(camera->mTRSMatrix, *(Mtx*)((u32)graphicsPointer + 0xb4));
                setCamera(0);*/
                //PSMTXIdentity(*(Mtx*)((u32)graphicsPointer + 0xb4));
                u32 functionAddress = *(u32*)((*(u32*)manager) + 0x48);
                // Cheeky way of detecting if EnemyManager instance by using unused restoreDrawBuffer as an anchor
                if(functionAddress != 0x80006bc4) {
                    u32 functionAddress = *(u32*)((*(u32*)manager) + 0x30);
                    ((int (*)(...))functionAddress)(manager, graphicsPointer);
                } else {
                    u32 functionAddress = *(u32*)((*(u32*)manager) + 0x44);
                    ((int (*)(...))functionAddress)(manager, graphicsPointer);
                }



                //OSReport("Function ptr %X\n", *(u32*)((*(u32*)manager) + 0x30));
                //((int (*)(...))((*(u32*)(*(u32*)manager) + 0x30)))(graphicsPointer);

                //manager->clipActors(graphicsPointer); // FIXME: Unsure why this has a lot lower clip range than in primary render, problem is that i am calling the function directly, should use vt to look up function...
                //manager->clipActorsAux(graphicsPointer, 1000.0f, manager->_3c);
                manager->setFlagOutOfCube();
                //setCamera(0);
             /*   camera = getCameraById(0);
                PSMTXCopy(camera->mTRSMatrix, *(Mtx*)((u32)graphicsPointer + 0xb4));*/
                /*SetViewFrustumClipCheckPerspective(camera->mProjectionFovy, camera->mProjectionAspect, 10.0f, 100000.0);
                */
                u32 objCount = manager->_14;
                u32 id = 0;
                while(id < objCount) {
                    TLiveActor* actor = *reinterpret_cast<TLiveActor**>(manager->_18 + id);

                    if(actor->mStateFlags.asU32 & 0x1 || actor->mObjectID == 0x4000003b || actor->mActorData == nullptr) {
                        id++;
                        continue;
                    }
                    //OSReport("Testing type %s\n", actor->mKeyName);

                    J3DModel* model = (J3DModel*)getModel__10TLiveActorCFv(actor);
                    actor->perform(0x42, graphicsPointer);

                    //char unknown = *(char*)((*(u32*)((u32)model + 0x84)) + 0x30);
                    ////OSReport("Testing %X, %X, %X, %X\n", (u32)model, unknown, model->_74, (u32)&model);
                    //actor->calcRootMatrix();
                    //actor->mActorData->calc();
                    //if(model != nullptr) {
                    //    if((actor->mStateFlags.asU32 & 0x204) == 0) {
                    //        if(unknown == 0) {
                    //            //model->viewCalc();
                    //            SMS_ShowAllShapePacket__FP8J3DModel(model);
                    //        }
                    //    } else {
                    //        if(unknown != 0) {
                    //            SMS_HideAllShapePacket__FP8J3DModel(model);
                    //        }
                    //    }
                    //}
                    id++;
                }

                /*manager->clipActorsAux(graphicsPointer, 0.1, 100.0f);*/
            }
            //OSReport("  Child %s %X %X %X\n", obj2->mKeyName, obj2->mPerformFlags, obj2->getType(), *(u32*)obj2);
            it2++;
        }
    }

    bool isRenderingOtherPerspectives = false;



    // Description: Before doing GXInvalidate, render other players perspective
    static void processGXInvalidateTexAll() { 
        //OSReport("---------------------\n");
        //OSReport("Starting p2 screen \n");
        //OSReport("Mario frame start\n");
        TMarDirector* director = (TMarDirector*)gpApplication.mDirector;
        GXInvalidateTexAll(); 
        if(!isSingleplayerLevel()) {
            //if(*gpSilhouetteManager) {
            //    u8* silhouetteAlpha = (u8*)(*gpSilhouetteManager + 0x48);
            //    //*silhouetteAlpha = 128;
            //}
            //u8* silhouetteAlpha = (u8*)(*gpSilhouetteManager + 0x48 / 4);
            //OSReport("Silhouette alpha %X %d\n", *gpSilhouetteManager, *silhouetteAlpha);


            setViewport(0);
            setActiveMario(0);
            setCamera(0);
            isRenderingOtherPerspectives = true;
            setWaterColorForMario(gpMarioOriginal);
                CPolarSubCamera* camera = getCameraById(0);
                //PSMTXCopy(camera->mTRSMatrix, *(Mtx*)((u32)graphicsPointer + 0xb4));
                // Set camera properties to TGraphics
                camera->perform(0x14, graphicsPointer);
                TMario* currentMario = getMario(0);
                Vec marioPos;
                marioPos.x = currentMario->mTranslation.x;
                marioPos.y = currentMario->mTranslation.y + 75.0;
                marioPos.z = currentMario->mTranslation.z;
     /*       u32 retraceCount = VIGetRetraceCount() / 2;
            do {
                OSSleepThread(&retraceQueue);
            } while(retraceCount);*/

            //VIWaitForRetrace();

            // TODO: Create a custom perform list of things that must update before drawing on p2 screen
            //if(g_sunModel) {
            //    calcDispRatioAndScreenPos___9TSunModelFv(g_sunModel);
            //    //g_sunModel->getZBufValue();
            //    perform__9TSunModelFUlPQ26JDrama9TGraphics(g_sunModel, 0x7, graphicsPointer);
            //}

            //if(g_sun) {
            //    perform__7TSunMgrFUlPQ26JDrama9TGraphics(g_sun, 0x7, graphicsPointer);
            //}

            TMarDirector_movement_game_override(director);

         //   s32 cubeNo = getInCubeNo__16TCubeManagerBaseCFRC3Vec(gpCubeArea, gpMarioPos);
	        //*(u32*)((u32)gpCubeArea + 0x1c) = cubeNo;
         //   cubeNo = getInCubeNo__16TCubeManagerBaseCFRC3Vec(gpCubeFastA, gpMarioPos);
	        //*(u32*)((u32)gpCubeFastA + 0x1c) = cubeNo;
         //   cubeNo = getInCubeNo__16TCubeManagerBaseCFRC3Vec(gpCubeFastB, gpMarioPos);
	        //*(u32*)((u32)gpCubeFastB + 0x1c) = cubeNo;
         //   cubeNo = getInCubeNo__16TCubeManagerBaseCFRC3Vec(gpCubeFastC, gpMarioPos);
	        //*(u32*)((u32)gpCubeFastC + 0x1c) = cubeNo;
            
                gpCubeArea->mCurrentCube = gpCubeArea->getInCubeNo(marioPos);
                gpCubeFastA->mCurrentCube = gpCubeFastA->getInCubeNo(marioPos);
                gpCubeFastB->mCurrentCube = gpCubeFastB->getInCubeNo(marioPos);
                gpCubeFastC->mCurrentCube = gpCubeFastC->getInCubeNo(marioPos);
            // Simulate quarter frames lmao.
            // Fixes Cube streams
            for(int i = 0; i < 4; ++i) {
                g_objectsToUpdate->perform(0x3, graphicsPointer);
            }


            //OSReport("\n\n\n\n");
        
            //
            //auto it = director->mPerformListCalcAnim->begin();
            //auto end = director->mPerformListCalcAnim->end();
            //while(it != end) {
            //    JDrama::TViewObj* viewObj = reinterpret_cast<JDrama::TViewObj*>(it->mData);
            //    ///*OSReport*/("Testing %s %X %X %X\n", viewObj->mKeyName, viewObj->mPerformFlags, viewObj->getType(), *(u32*)viewObj);
            //    if(*(u32*)viewObj == 0x803c0f5c || *(u32*)0x803C0F5C) { // JDrama::TViewObjPtrListT
            //        JDrama::TViewObjPtrListT<JDrama::TViewObj>* list = reinterpret_cast<JDrama::TViewObjPtrListT<JDrama::TViewObj>*>(viewObj);
            //        //OSReport("JDrama::TViewObjPtrListT\n");
            //        auto it2 = list->mViewObjList.begin();
            //        auto end2 = list->mViewObjList.end();
            //        while(it2 != end2) {
            //            JDrama::TViewObj* obj2 = reinterpret_cast<JDrama::TViewObj*>(*it2);
            //            //OSReport("  Child %s %X %X %X\n", obj2->mKeyName, obj2->mPerformFlags, obj2->getType(), *(u32*)obj2);
            //            it2++;
            //        }
            //    }
            //    if(*(u32*)viewObj == 0x803da444 || *(u32*)0x803DA444) { // TIdxGroupObj (also a TViewObjPtrList)
            //        TIdxGroupObj* list = reinterpret_cast<TIdxGroupObj*>(viewObj);
            //        auto it2 = list->mViewObjList.begin();
            //        auto end2 = list->mViewObjList.end();
            //        //OSReport("TIdxGroupObj\n");
            //        while(it2 != end2) {
            //            JDrama::TViewObj* obj2 = reinterpret_cast<JDrama::TViewObj*>(*it2);
            //            //OSReport("  Child %s %X %X %X\n", obj2->mKeyName, obj2->mPerformFlags, obj2->getType(), *(u32*)obj2);
            //            it2++;
            //        }
            //    }
            //    if(*(u32*)viewObj == 0x803ad958) { // TConductor
            //        TConductor* conductor = reinterpret_cast<TConductor*>(viewObj);
            //        //OSReport("Conductor\n");
            //            /*OSReport("  Conductor first head  %X %X %X %X\n", (u32)conductor->_30.mIterator, (u32)conductor->_30.mNext, (u32)conductor->_30.mPrevious, (u32)conductor->_30.mValue);*/
            //        
            //        /*conductor->perform(0x1001, graphicsPointer);
            //        conductor->perform(0x2001, graphicsPointer);
            //        conductor->perform(0x1, graphicsPointer);
            //        conductor->perform(0x3001, graphicsPointer);*/
            //        //conductor->perform(0x2, graphicsPointer);
            //        //OSReport("Conductor manager _10\n");
            //        printList(conductor->_10, 2);
            //        ////OSReport("Conductor manager _20\n");
            //        //printList(conductor->_20);
            //        ////OSReport("Conductor manager _30\n");
            //        //printList(conductor->_30, 1);
            //        ////OSReport("Conductor manager _40\n");
            //        //printList(conductor->_40);
            //        ////OSReport("Conductor manager _50\n");
            //        //printList(conductor->_50);
            //        ////OSReport("Conductor manager _60\n");
            //        //printList(conductor->_60);
            // /*       OSReport("Conductor enemy manager _70\n");
            //        printList(conductor->_70);*/
            //    }
            //    it = it->mNext;
            //}
            /*director->mPerformListCalcAnim->perform(0x1001, graphicsPointer);
            director->mPerformListCalcAnim->perform(0x2001, graphicsPointer);
            director->mPerformListCalcAnim->perform(0x1, graphicsPointer);
            director->mPerformListCalcAnim->perform(0x3001, graphicsPointer);
            director->mPerformListCalcAnim->perform(0x2, graphicsPointer);*/
            //director->mPerformListCalcAnim->perform(0x2, graphicsPointer);
            
            //director->mPerformListSilhouette->perform(0xffffffff, graphicsPointer);
            director->mPerformListPreDraw->perform(0xffffffff, graphicsPointer);
            director->mPerformListPostDraw->perform(0xffffffff, graphicsPointer);
            //


            //}
            //if(g_sunLensGlow) {
            //    perform__9TLensGlowFUlPQ26JDrama9TGraphics(g_sunLensGlow, 0x7, graphicsPointer);
            //}
            //if(g_sunLensFlare) {
            //    perform__10TLensFlareFUlPQ26JDrama9TGraphics(g_sunLensFlare, 0x7, graphicsPointer);
            //}

            // Update + goop stamp (game freezes with goop stamps otherwise :c)
            director->mPerformListUnk1->perform(0x1000000, graphicsPointer); // Need to do some preparation for goop maps
            director->mPerformListUnk2->perform(0x20f0000, graphicsPointer); // Unsure exactly what happens, but this works without drawing a black square to the screen
            director->mPerformListGX->perform(0xffffffff, graphicsPointer);
            
            // FIXME should be based on some conditions, check direct in MarDirector
            director->mPerformListSilhouette->testPerform(0xffffffff, graphicsPointer);

            director->mPerformListGXPost->perform(0xffffffff, graphicsPointer);
            
            isRenderingOtherPerspectives = false;
            setCamera(1);
            setActiveMario(1);
            setViewport(1);
            setWaterColorForMario(gpMarioOriginal);
        }
        
        GXInvalidateTexAll(); 
        
        /*
        OSReport("Ending p2 screen \n");
        OSReport("---------------------\n");
        OSReport("Content of GXPost\n");
        OSReport("Testing %s \n", director->mPerformListPostDraw->mKeyName);
        for(JGadget::TSingleNodeLinkList::iterator begin = director->mPerformListPostDraw->begin(); 
            begin != director->mPerformListPostDraw->end(); ) {
            
            JDrama::TViewObj* obj = (JDrama::TViewObj*)begin->mData;
            OSReport("   Child: '%s' \n", obj->mKeyName);

            begin = begin->mNext;
        }
        OSReport("---------------------\n");*/
        //OSReport("Mario frame end\n");
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80299d04, 0, 0, 0), processGXInvalidateTexAll);
    
 //   void TConductor_perform(TConductor* conductor, u32 perform_flags, JDrama::TGraphics* graphics) {
 //       OSReport("Update with flags %X \n", perform_flags);
 //     /*  if(perform_flags == 0x2 && !isRenderingOtherPerspectives) {
 //           return;
 //       }*/
 //       perform__10TConductorFUlPQ26JDrama9TGraphics(conductor, perform_flags, graphics);
 //   }
	//SMS_WRITE_32(SMS_PORT_REGION(0x803ad978, 0, 0, 0), (u32)(&TConductor_perform));
    
    void frameUpdate_override(void* self) {
        if(!isRenderingOtherPerspectives) {
            frameUpdate__6MActorFv(self);
        }
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x80008990, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x8003dc70, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x8003e668, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x8005ba58, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80068d7c, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x800d953c, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x800f2734, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x800f2784, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x801949bc, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x801d4184, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x801d41b8, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x801f79dc, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x801f7a2c, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x802069dc, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x8021220c, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80217f34, 0, 0, 0), frameUpdate_override);
    SMS_PATCH_BL(SMS_PORT_REGION(0x802391e4, 0, 0, 0), frameUpdate_override);
    //u32 testing() {
    //    return true;
    //}
    //SMS_PATCH_BL(SMS_PORT_REGION(0x8021b144, 0, 0, 0), testing);
    int test = 0;
    void PerformList_push_back_mapGroup(TPerformList* that, JDrama::TViewObjPtrListT<JDrama::TViewObj, JDrama::TViewObj>* mapObj, u32 flags) {

        auto beginIter = mapObj->mViewObjList.begin();
        auto endIter = mapObj->mViewObjList.end();
        JGadget::TList<JDrama::TViewObj*>::iterator iter = *reinterpret_cast<JGadget::TList<JDrama::TViewObj*>::iterator*>(&beginIter);
        JGadget::TList<JDrama::TViewObj*>::iterator end = *reinterpret_cast<JGadget::TList<JDrama::TViewObj*>::iterator*>(&endIter);
        auto* mapGroup = new JDrama::TViewObjPtrListT<JDrama::TViewObj>("Map group objs");
        while(iter != end) {
        

            if(strcmp((const char*)((u32)(*iter) - 12), "MapObjBase") != 0 && 
                strcmp((const char*)((u32)(*iter) - 12), "RiccoSwitch") != 0 && 
                strcmp((const char*)((u32)(*iter) - 16), "PinnaCoaster") != 0) {
                mapGroup->mViewObjList.insert(mapGroup->mViewObjList.end(), *iter);
            }
            iter++;
        }
        that->push_back(mapGroup, flags);

    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8029c2fc, 0, 0, 0), PerformList_push_back_mapGroup);

    // Bowserfight rendering
    // This is all custom, which is very annoying

    // Description: 
    // thusly we need to handle this shit manually...
    void GXSetViewport_bathwater(f32 xOrig,f32 yOrig,f32 wd,f32 ht,f32 nearZ,f32 farZ) {
        f32 xOffset = xOrig;
        if(perspective == 1) {
            xOffset = 320.0f;
        }

        GXSetViewport(xOffset, yOrig, 640.0f / 2.0f, ht, nearZ, farZ);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x801ab0b4, 0, 0, 0), GXSetViewport_bathwater);

    // Description: Set source copy when copying bathwater from efb to screentexture
    // We copy the part of the texture that the active viewport is rendering
    void GXSetTexCopySrc_bathwater(u16 left, u16 top,u16 wd,u16 ht) {
        u16 xOffset = left;
        if(perspective == 1) {
            xOffset = 320;
        }

        GXSetTexCopySrc(xOffset, top, wd, ht);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x801acd98, 0, 0, 0), GXSetTexCopySrc_bathwater);

    static unsigned int SMSGetGameRenderWidth_320() { 
        if(!isSingleplayerLevel()) {
		    return 320;
        }
        return 640;
    }
    // Description: Sets render width for bathwater EFB
    SMS_PATCH_BL(SMS_PORT_REGION(0x801ac970, 0, 0, 0), SMSGetGameRenderWidth_320);
    
    // Description: Set screen area for mist and bathwater reflection to render
    void draw_mist(u32 x, u32 y, u32 wd, u32 ht, u32 unk) {
        u32 xOffset = x;
        if(perspective == 1) {
            xOffset = 320;
        }
        draw_mist__FUsUsUsUsPv(xOffset, y, wd/2, ht, unk);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x801ad6a8, 0, 0, 0), draw_mist);

    // Manta rendering
    
    SMS_PATCH_BL(SMS_PORT_REGION(0x80110128, 0, 0, 0), GXSetTexCopySrc_bathwater);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80110114, 0, 0, 0), SMSGetGameRenderWidth_320);
    SMS_PATCH_BL(SMS_PORT_REGION(0x80110134, 0, 0, 0), SMSGetGameRenderWidth_320);
    SMS_PATCH_BL(SMS_PORT_REGION(0x801101e4, 0, 0, 0), SMSGetGameRenderWidth_320);

    // Fix sun
    void GXPeekZ_override(u16 x, u16 y, u32* z) {
        if(isSingleplayerLevel()) {
            GXPeekZ(x, y, z);
            return;
        }
        int offsetX = x / 2;
        if(perspective == 1) {
            offsetX += 320;
        }
        GXPeekZ(offsetX, y, z);
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8002eacc, 0, 0, 0), GXPeekZ_override);

    

    void TSunMgr_perform_override(void* sunMgr, u32 performFlags, JDrama::TGraphics* graphics) {
        g_sun = sunMgr;
        perform__7TSunMgrFUlPQ26JDrama9TGraphics(sunMgr, performFlags, graphics);
    }
	SMS_WRITE_32(SMS_PORT_REGION(0x803ad180, 0, 0, 0), TSunMgr_perform_override);
    void TSunModel_perform_override(TSunModel* sunModel, u32 performFlags, JDrama::TGraphics* graphics) {
        g_sunModel = sunModel;
        calcDispRatioAndScreenPos___9TSunModelFv(g_sunModel);
        g_sunModel->getZBufValue();
        perform__9TSunModelFUlPQ26JDrama9TGraphics(sunModel, performFlags, graphics);
    }
	SMS_WRITE_32(SMS_PORT_REGION(0x803ad1c0, 0, 0, 0), TSunModel_perform_override);
    void TLensGlow_perform_override(void* lensGlow, u32 performFlags, JDrama::TGraphics* graphics) {
        g_sunLensGlow = lensGlow;
        perform__9TLensGlowFUlPQ26JDrama9TGraphics(lensGlow, performFlags, graphics);
    }
	SMS_WRITE_32(SMS_PORT_REGION(0x803ad150, 0, 0, 0), TLensGlow_perform_override);
    void TLensFlare_perform_override(void* lensFlare, u32 performFlags, JDrama::TGraphics* graphics) {
        g_sunLensFlare = lensFlare;
        perform__10TLensFlareFUlPQ26JDrama9TGraphics(lensFlare, performFlags, graphics);
    }
	SMS_WRITE_32(SMS_PORT_REGION(0x803ad128, 0, 0, 0), TLensFlare_perform_override);

    // Remove check for graphics on sun stare (enter noki) This might cause bugs...
	SMS_WRITE_32(SMS_PORT_REGION(0x8002e308, 0, 0, 0), 0x60000000);

    
    
    // Description: Fix shadow not using right mtx
    //void boxDrawPrepare_override(TMario* mario, Mtx matrix) {
    //    CPolarSubCamera* camera = getCameraById(getPlayerId(mario));
    //    boxDrawPrepare__6TMarioFPA4_f(mario, camera->mTRSMatrix);
    //}
    //SMS_PATCH_BL(SMS_PORT_REGION(0x8024da54, 0, 0, 0), boxDrawPrepare_override);
    // Fix shadow behind walls
 //   u8 currentSyncDrawMarioId = 0;
 //   #define drawSyncCallback__6TMarioFUs     ((int (*)(...))0x8024D17C)
 //   void TMario_drawSyncCallback_override(TMario* mario, u16 flag) {
 //       //currentSyncDrawMarioId = getPlayerId(mario);
 //       //mario->mAttributes._04 |= 1;

 //       for(int i = 0; i < getPlayerCount(); ++i) {
 //           //drawSyncCallback__6TMarioFUs(getMario(i), flag);
 //       }

 //       //OSReport("Dafux %X %d\n", mario, flag);
 //       //drawSyncCallback__6TMarioFUs(mario, flag);
 //   }
	//SMS_WRITE_32(SMS_PORT_REGION(0x803af008, 0, 0, 0), (u32)(&TMario_drawSyncCallback_override));
	//SMS_WRITE_32(SMS_PORT_REGION(0x803dd744, 0, 0, 0), (u32)(&TMario_drawSyncCallback_override));
 //   SMS_PATCH_B(SMS_PORT_REGION(0x800452dc, 0, 0, 0), TMario_drawSyncCallback_override);
    
    //// Description: Fix isUnderground check
	SMS_WRITE_32(SMS_PORT_REGION(0x8022794c, 0, 0, 0), 0x60000000);

    // Kinda fixes p2's shadow, but looks very weird...
    // FIXME
	//SMS_WRITE_32(SMS_PORT_REGION(0x8024d9cc, 0, 0, 0), 0x60000000);
	//SMS_WRITE_32(SMS_PORT_REGION(0x8024da00, 0, 0, 0), 0x60000000);

    //// Description: Fix is obstructed check
    //// TODO: Fix for horizontal split screen
    //void GXPeekARGB_override(u16 x, u16 y, GXColor* color) {
    //    u16 rX = x / 2;
    //    if(perspective == 1) {
    //        rX += 320;
    //    }
    //    GXPeekARGB(rX, y, color);
    //}
    //SMS_PATCH_BL(SMS_PORT_REGION(0x8024d25c, 0, 0, 0), GXPeekARGB_override);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x8024d1d0, 0, 0, 0), SMSGetGameRenderWidth_320);
}