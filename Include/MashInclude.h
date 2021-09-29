//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_INCLUDE_H_
#define _MASH_INCLUDE_H_

#include "Mash.h"
#include "MashVector2.h"
#include "MashVector3.h"
#include "MashVector4.h"
#include "MashAABB.h"
#include "MashLine3.h"
#include "MashMatrix4.h"
#include "MashQuaternion.h"
#include "MashRay.h"
#include "MashSphere.h"
#include "MashPlane.h"
#include "MashOBB.h"
#include "MashTriangle.h"
#include "MashEventTypes.h"
#include "MashEventDispatch.h"
#include "MashRenderInfo.h"
#include "MashAutoEffectParameter.h"
#include "MashEffect.h"
#include "MashTechniqueInstance.h"
#include "MashTechnique.h"
#include "MashMaterial.h"
#include "MashTexture.h"
#include "MashVideo.h"
#include "MashTypes.h"
#include "MashSkin.h"
#include "MashMaterialManager.h"
#include "MashControllerManager.h"
#include "MashCullTechnique.h"
#include "MashSceneManager.h"
#include "MashSceneNodeCallback.h"
#include "MashSceneNode.h"
#include "MashDummy.h"
#include "MashCamera.h"
#include "MashSubEntity.h"
#include "MashEntity.h"
#include "MashDummy.h"
#include "MashLight.h"
#include "MashDecal.h"
#include "MashParticleSystem.h"
#include "MashAnimationBuffer.h"
#include "MashAnimationController.h"
#include "MashAnimationMixer.h"
#include "MashScenePick.h"
#include "MashRenderSurface.h"
#include "MashTimer.h"
#include "MashGeometryBatch.h"
#include "MashIndexBuffer.h"
#include "MashVertexBuffer.h"
#include "MashTriangleBuffer.h"
#include "MashTriangleCollider.h"
#include "MashMeshBuilder.h"
#include "MashGeometryHelper.h"
#include "MashMeshBuffer.h"
#include "MashVertex.h"
#include "MashModel.h"
#include "MashStaticMesh.h"
#include "MashDynamicMesh.h"
#include "MashGeometryBatch.h"
#include "MashMaterialBuilder.h"
#include "MashInputManager.h"
#include "MashXMLReader.h"
#include "MashXMLWriter.h"
#include "MashFileStream.h"
#include "MashFileManager.h"
#include "MashGenericScriptReader.h"
#include "MashMemory.h"
#include "MashHelper.h"
#include "MashList.h"
#include "MashArray.h"
#include "MashString.h"
#include "MashStringHelper.h"
#include "MashGenericArray.h"

#include "MashEllipsoidColliderController.h"
#include "MashFreeMovementController.h"
#include "MashDirectionalShadowCascadeCaster.h"
#include "MashSpotShadowCaster.h"
#include "MashPointShadowCaster.h"

#include "./Physics/MashPhysics.h"
#include "./Physics/MashPhysicsRigidBody.h"
#include "./Physics/MashPhysicsCollisionShape.h"

#include "./Script/MashLuaScript.h"
#include "./Script/MashLuaScriptWrapper.h"
#include "./Script/MashLuaTypes.h"
#include "./Script/MashLuaTypesC.h"
#include "./Script/MashSceneNodeScriptHandler.h"
#include "./Script/MashScriptManager.h"

#include "./GUI/MashGUIManager.h"

#include "MashLog.h"

#endif