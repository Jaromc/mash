//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashScriptAccessors.h"
#include "MashDevice.h"
#include "MashInputManager.h"
#include "MashSceneNode.h"
#include "MashSceneManager.h"
#include "MashVideo.h"
#include "MashMaterialManager.h"
#include "MashTimer.h"
#include "MashScriptManager.h"
#include <assert.h>

mash::MashInputManager *g_pMashLuaInputManager = 0;
mash::MashSceneManager *g_pMashLuaSceneManager = 0;
mash::MashVideo *g_pMashLuaVideo = 0;
mash::MashTimer *g_pMashLuaTimer = 0;
mash::MashScriptManager *g_pMashScriptManager = 0;

/*
	This function assumes m_pDevice has been initialised before calling.
*/
int MashLuaAccessorInitialise(void *pDevicePointer)
{
	mash::MashDevice *pDevice = (mash::MashDevice*)pDevicePointer;

	if (!pDevice)
		return 0;

	g_pMashLuaInputManager = pDevice->GetInputManager();
	g_pMashLuaSceneManager = pDevice->GetSceneManager();
	g_pMashLuaVideo = pDevice->GetRenderer();
	g_pMashLuaTimer = pDevice->GetTimer();
	g_pMashScriptManager = pDevice->GetScriptManager();

	return 1;
}

int _MashLua_CallUserFunction(void *state, unsigned int callbackId)
{
	return g_pMashScriptManager->_CallUserFunction(state, callbackId);
}

int _MashLua_GetIsPressed(unsigned int player, unsigned int action)
{
	return (int)g_pMashLuaInputManager->HelperIsKeyPressed(player, action);
}

int _MashLua_GetIsReleased(unsigned int player, unsigned int action)
{
    return (int)g_pMashLuaInputManager->HelperIsKeyReleased(player, action);
}

int _MashLua_GetIsHeld(unsigned int player, unsigned int action)
{
   return (int)g_pMashLuaInputManager->HelperIsKeyHeld(player, action);
}

float _MashLua_GetKeyValue(unsigned int player, unsigned int action)
{
	return g_pMashLuaInputManager->HelperGetKeyValue(player, action);
}

int _MashLua_GetCurrentSceneNodeID(void)
{
	return g_pMashLuaSceneManager->GetCurrentScriptSceneNode()->GetNodeID();
}

int _MashLua_GetSceneNodeIDByName(const char *sName)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByName(sName);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	return pNode->GetNodeID();
}

int _MashLua_DetachSceneNode(int iNodeToRemove)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNodeToRemove);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	pNode->Detach();

	return MASH_LUA_OK;
}

int _MashLua_AttachSceneNode(int iNodeToAttach, int iAttachTo)
{
	mash::MashSceneNode *pNodeToAttach = g_pMashLuaSceneManager->GetSceneNodeByID(iNodeToAttach);
	mash::MashSceneNode *pAttachTo = g_pMashLuaSceneManager->GetSceneNodeByID(iAttachTo);

	if (pNodeToAttach == 0 || pAttachTo == 0)
		return MASH_LUA_FAIL;

	if (pAttachTo->AddChild(pNodeToAttach) == mash::aMASH_FAILED)
		return MASH_LUA_FAIL;

	return MASH_LUA_OK;
}

int _MashLua_GetSceneNodePosition(int iNode, float *x, float *y, float *z)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	mash::MashVector3 vPosition = pNode->GetLocalTransformState().translation;

	*x = vPosition.x;
	*y = vPosition.y;
	*z = vPosition.z;

	return MASH_LUA_OK;
}

int _MashLua_GetNodeRotation(int iNode, float *fPitch, float *fYaw, float *fRoll)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	mash::MashVector3 vEuler;
	pNode->GetLocalTransformState().orientation.ToEulerAngles(vEuler);

	*fRoll = vEuler.x;
	*fPitch = vEuler.y;
	*fYaw = vEuler.z;

	return MASH_LUA_OK;
}

int _MashLua_GetSceneNodePositionAbs(int iNode, float *x, float *y, float *z)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	mash::MashVector3 vPosition = pNode->GetWorldTransformState().translation;

	*x = vPosition.x;
	*y = vPosition.y;
	*z = vPosition.z;

	return MASH_LUA_OK;
}

int _MashLua_GetNodeRotationAbs(int iNode, float *fPitch, float *fYaw, float *fRoll)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	mash::MashVector3 vEuler;
	pNode->GetWorldTransformState().orientation.ToEulerAngles(vEuler);

	*fRoll = vEuler.x;
	*fPitch = vEuler.y;
	*fYaw = vEuler.z;

	return MASH_LUA_OK;
}

int _MashLua_SetSceneNodeRotation(int iNode, float x, float y, float z)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;
    
	mash::MashQuaternion qNewRot;
	qNewRot.SetEuler(x, y, z);
	pNode->SetOrientation(qNewRot);

	return MASH_LUA_OK;
}

int _MashLua_SetSceneNodePosition(int iNode, float x, float y, float z)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	pNode->SetPosition(mash::MashVector3(x, y, z));

	return MASH_LUA_OK;
}

int _MashLua_AddSceneNodeRotation(int iNode, float x, float y, float z)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	mash::MashQuaternion qNewRot;
	qNewRot.SetEuler(x, y, z);
	pNode->AddOrientation(qNewRot);

	return MASH_LUA_OK;
}

int _MashLua_AddSceneNodePosition(int iNode, float x, float y, float z)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	pNode->AddPosition(mash::MashVector3(x, y, z));

	return MASH_LUA_OK;
}

int _MashLua_TransformVectorByNodeAbs(int iNode, float x, float y, float z, float *newX, float *newY, float *newZ)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	mash::MashVector3 vec(x, y, z);
	mash::MashVector3 newVecOut = pNode->GetWorldTransformState().Transform(vec);

	*newX = newVecOut.x;
	*newY = newVecOut.y;
	*newZ = newVecOut.z;

	return MASH_LUA_OK;
}

int _MashLua_RotateVectorByNodeAbs(int iNode, float x, float y, float z, float *newX, float *newY, float *newZ)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	mash::MashVector3 vec(x, y, z);
	mash::MashVector3 newVecOut = pNode->GetWorldTransformState().TransformRotation(vec);
	
	*newX = newVecOut.x;
	*newY = newVecOut.y;
	*newZ = newVecOut.z;

	return MASH_LUA_OK;
}

int _MashLua_RotateVectorByNodeNormAbs(int iNode, float x, float y, float z, float *newX, float *newY, float *newZ)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	mash::MashVector3 vec(x, y, z);
	mash::MashVector3 newVecOut = pNode->GetWorldTransformState().TransformRotation(vec);
	newVecOut.Normalize();

	*newX = newVecOut.x;
	*newY = newVecOut.y;
	*newZ = newVecOut.z;

	return MASH_LUA_OK;
}

int _MashLua_TransformVectorByNode(int iNode, float x, float y, float z, float *newX, float *newY, float *newZ)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	mash::MashVector3 vec(x, y, z);
	mash::MashVector3 newVecOut = pNode->GetLocalTransformState().Transform(vec);

	*newX = newVecOut.x;
	*newY = newVecOut.y;
	*newZ = newVecOut.z;

	return MASH_LUA_OK;
}

int _MashLua_RotateVectorByNode(int iNode, float x, float y, float z, float *newX, float *newY, float *newZ)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	mash::MashVector3 vec(x, y, z);
	mash::MashVector3 newVecOut = pNode->GetLocalTransformState().TransformRotation(vec);
	
	*newX = newVecOut.x;
	*newY = newVecOut.y;
	*newZ = newVecOut.z;

	return MASH_LUA_OK;
}

int _MashLua_RotateVectorByNodeNorm(int iNode, float x, float y, float z, float *newX, float *newY, float *newZ)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	mash::MashVector3 vec(x, y, z);
	mash::MashVector3 newVecOut = pNode->GetLocalTransformState().TransformRotation(vec);
	newVecOut.Normalize();

	*newX = newVecOut.x;
	*newY = newVecOut.y;
	*newZ = newVecOut.z;

	return MASH_LUA_OK;
}

int _MashLua_IsParticle(int iNode)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return 0;

    if (pNode->GetNodeType() & mash::aNODETYPE_PARTICLE_EMITTER)
		return 1;

	return 0;
}

int _MashLua_IsDummy(int iNode)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return 0;

    if (pNode->GetNodeType() & mash::aNODETYPE_DUMMY)
		return 1;

	return 0;
}

int _MashLua_IsBone(int iNode)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return 0;

    if (pNode->GetNodeType() & mash::aNODETYPE_BONE)
		return 1;

	return 0;
}

int _MashLua_IsEntity(int iNode)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return 0;

    if (pNode->GetNodeType() & mash::aNODETYPE_ENTITY)
		return 1;

	return 0;
}

int _MashLua_IsLight(int iNode)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return 0;

	if (pNode->GetNodeType() & mash::aNODETYPE_LIGHT)
		return 1;

	return 0;
}

int _MashLua_IsCamera(int iNode)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return 0;


	if (pNode->GetNodeType() & mash::aNODETYPE_CAMERA)
		return 1;

	return 0;
}

int _MashLua_SetVisible(int iNode, int bIsVisible)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return MASH_LUA_FAIL;

	pNode->SetVisible(bIsVisible);

	return MASH_LUA_OK;
}

float _MashLua_GetFixedTime()
{
	return g_pMashLuaTimer->GetFixedTimeInSeconds();
}

int _MashLua_IsVisible(int iNode)
{
	mash::MashSceneNode *pNode = g_pMashLuaSceneManager->GetSceneNodeByID(iNode);

	if (pNode == 0)
		return 0; //return not visible

	return pNode->IsVisible();
}
