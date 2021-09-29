//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_SCRIPT_ACCESSORS_H_
#define _C_MASH_SCRIPT_ACCESSORS_H_

#include "CMashScriptCompilerSettings.h"

#define MASH_LUA_FAIL -1
#define MASH_LUA_OK 1

MASH_EXTERN_C int MashLuaAccessorInitialise(void *pDevicePointer);
MASH_EXTERN_C int _MashLua_CallUserFunction(void *state, unsigned int callbackId);
MASH_EXTERN_C int _MashLua_GetIsPressed(unsigned int player, unsigned int action);
MASH_EXTERN_C int _MashLua_GetIsReleased(unsigned int player, unsigned int action);
MASH_EXTERN_C int _MashLua_GetIsHeld(unsigned int player, unsigned int action);
MASH_EXTERN_C float _MashLua_GetKeyValue(unsigned int player, unsigned int action);

MASH_EXTERN_C int _MashLua_GetCurrentSceneNodeID(void);
MASH_EXTERN_C int _MashLua_GetSceneNodeIDByName(const char *sName);
MASH_EXTERN_C int _MashLua_DetachSceneNode(int iNodeToRemove);

MASH_EXTERN_C int _MashLua_AttachSceneNode(int iNodeToAttach, int iAttachTo);
MASH_EXTERN_C int _MashLua_GetSceneNodePosition(int iNode, float *x, float *y, float *z);
MASH_EXTERN_C int _MashLua_GetNodeRotation(int iNode, float *fPitch, float *fYaw, float *fRoll);
MASH_EXTERN_C int _MashLua_GetSceneNodePositionAbs(int iNode, float *x, float *y, float *z);
MASH_EXTERN_C int _MashLua_GetNodeRotationAbs(int iNode, float *fPitch, float *fYaw, float *fRoll);
MASH_EXTERN_C int _MashLua_SetSceneNodeRotation(int iNode, float x, float y, float z);
MASH_EXTERN_C int _MashLua_SetSceneNodePosition(int iNode, float x, float y, float z);

MASH_EXTERN_C int _MashLua_AddSceneNodeRotation(int iNode, float x, float y, float z);
MASH_EXTERN_C int _MashLua_AddSceneNodePosition(int iNode, float x, float y, float z);
MASH_EXTERN_C int _MashLua_TransformVectorByNodeAbs(int iNode, float x, float y, float z, float *newX, float *newY, float *newZ);
MASH_EXTERN_C int _MashLua_RotateVectorByNodeAbs(int iNode, float x, float y, float z, float *newX, float *newY, float *newZ);
MASH_EXTERN_C int _MashLua_RotateVectorByNodeNormAbs(int iNode, float x, float y, float z, float *newX, float *newY, float *newZ);
MASH_EXTERN_C int _MashLua_TransformVectorByNode(int iNode, float x, float y, float z, float *newX, float *newY, float *newZ);
MASH_EXTERN_C int _MashLua_RotateVectorByNode(int iNode, float x, float y, float z, float *newX, float *newY, float *newZ);
MASH_EXTERN_C int _MashLua_RotateVectorByNodeNorm(int iNode, float x, float y, float z, float *newX, float *newY, float *newZ);

MASH_EXTERN_C int _MashLua_IsEntity(int iNode);
MASH_EXTERN_C int _MashLua_IsLight(int iNode);
MASH_EXTERN_C int _MashLua_IsCamera(int iNode);
MASH_EXTERN_C int _MashLua_IsParticle(int iNode);
MASH_EXTERN_C int _MashLua_IsDummy(int iNode);
MASH_EXTERN_C int _MashLua_IsBone(int iNode);
MASH_EXTERN_C int _MashLua_SetVisible(int iNode, int bIsVisible);
MASH_EXTERN_C float _MashLua_GetFixedTime();
MASH_EXTERN_C int _MashLua_IsVisible(int iNode);

#endif

