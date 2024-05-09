#include <sdk.h>
#include <macros.h>
#include <raw_fn.hxx>
#include <JDrama/JDRGraphics.hxx>
#include <SMS/Camera/PolarSubCamera.hxx>

#include "players.hxx"
#include "camera.hxx"

#include <SMS/Manager/ModelWaterManager.hxx>

namespace SMSCoop {

    //// Description: Check if object is within view of the camera or not
    //int ViewFrustumClipCheck(double param_1, JDrama::TGraphics* param_2, TVec3f* param_3) {
    //    int theResult = ViewFrustumClipCheck__FPQ26JDrama9TGraphicsP3Vecf(param_1, param_2, param_3);
    //    if(theResult) 
    //        return theResult;
    //    //int theResult = 0;
    //    for(int i = 0; i < getPlayerCount(); ++i) {
    //        CPolarSubCamera* camera = getCameraById(i);
    //        PSMTXCopy(camera->mTRSMatrix, *(Mtx*)((u32)param_2 + 0xb4));
    //        SetViewFrustumClipCheckPerspective__Fffff(camera->mProjectionFovy, camera->mProjectionAspect, 10.0, 100000.0);
    //    
    //        theResult  = ViewFrustumClipCheck__FPQ26JDrama9TGraphicsP3Vecf(param_1, param_2, param_3);
    //        if(theResult) break;

    //    }
    //
    //    CPolarSubCamera* camera = getCameraById(1);
    //    PSMTXCopy(camera->mTRSMatrix, *(Mtx*)((u32)param_2 + 0xb4));
    //    return theResult;
    //}

    //SMS_PATCH_BL(SMS_PORT_REGION(0x8000752c, 0, 0, 0), ViewFrustumClipCheck);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x80007684, 0, 0, 0), ViewFrustumClipCheck);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x80009eb8, 0, 0, 0), ViewFrustumClipCheck);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x80034078, 0, 0, 0), ViewFrustumClipCheck); // Conductor clipping 
    //SMS_PATCH_BL(SMS_PORT_REGION(0x8003d0e4, 0, 0, 0), ViewFrustumClipCheck);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x8004a274, 0, 0, 0), ViewFrustumClipCheck);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x8004a2cc, 0, 0, 0), ViewFrustumClipCheck);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x80069368, 0, 0, 0), ViewFrustumClipCheck);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x800693c0, 0, 0, 0), ViewFrustumClipCheck);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x80069438, 0, 0, 0), ViewFrustumClipCheck);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x8007fdb4, 0, 0, 0), ViewFrustumClipCheck);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x8007fe0c, 0, 0, 0), ViewFrustumClipCheck);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x80135c30, 0, 0, 0), ViewFrustumClipCheck);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x8020a320, 0, 0, 0), ViewFrustumClipCheck);
    //SMS_PATCH_BL(SMS_PORT_REGION(0x8021b144, 0, 0, 0), ViewFrustumClipCheck); // ClipActorAux in TLiveManager (converted

    // Description: Increases distance from gpMarioOriginal(player 1) where water can exist to a very high number. 
    // This is to fix water for second player when they are far away from the "main" player
    void TModelWaterManager_perform_move(TModelWaterManager* waterManager) {
	    // OPTIMIZATION: Do this on load of water manager instead of every time it tries to move the water
	    *(f32*)(((u32*)waterManager) + 0x5e08/4) = 1000000.0f;
	    waterManager->move();
    }
    SMS_PATCH_BL(SMS_PORT_REGION(0x8027bedc, 0, 0, 0), TModelWaterManager_perform_move);
}