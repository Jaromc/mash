

#include <stdlib.h>


#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "../CMashScriptAccessors.h"


static int MashLua_GetSceneNodeID(lua_State *L)
{
	lua_pushinteger(L, _MashLua_GetCurrentSceneNodeID());

	return 1;
}

static int MashLua_GetSceneNodeIDByName(lua_State *L)
{
	const char *sName = lua_tostring(L, 1);

	if (!lua_isstring(L, 1))
	{
		printf("GetNodeIDByName function failed within script. Parameter is not of type string.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	lua_pushinteger(L, _MashLua_GetSceneNodeIDByName(sName));

	return 1;
}

static int MashLua_DetachSceneNode(lua_State *L)
{
	int iNodeToRemove = (int)lua_tointeger(L, 1);

	if (!lua_isnumber(L, 1))
	{
		printf("DetachNode function failed within script. Parameter is not of type integer.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	lua_pushinteger(L, _MashLua_DetachSceneNode(iNodeToRemove));

	return 1;
}

/*
	TODO : Test this function alot!
*/
//static int MashLua_DestroySceneNode(lua_State *L)
//{
//	int iNodeToDestroy = (int)lua_tointeger(L, 1);
//
//	if (!lua_isnumber(L, 1))
//	{
//		printf("DestroyNode function failed within script. Parameter is not of type integer.");
//
//		lua_pushinteger(L, MASH_LUA_FAIL);
//
//		return 1;
//	}
//
//	lua_pushinteger(L, _MashLua_DestroySceneNode(iNodeToDestroy));
//
//	return 1;
//}

static int MashLua_AttachSceneNode(lua_State *L)
{
	int iNodeToAttach = (int)lua_tointeger(L, 1);
	int iAttachTo = (int)lua_tointeger(L, 2);

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
	{
		printf("AttachNode function failed within script. Parameter is not of type integer.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}
	
	lua_pushinteger(L, _MashLua_AttachSceneNode(iNodeToAttach, iAttachTo));

	return 1;
}

/*
	Usage:

	\Lua code

	x, y, z = GetNodePosition(iNodeID);

	\End Lua code
*/
static int MashLua_GetNodePosition(lua_State *L)
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);

	if (!lua_isnumber(L, 1))
	{
		printf("GetNodePosition function failed within script. Parameter is not of type integer.");

		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);

		return 3;
	}


	iResult = _MashLua_GetSceneNodePosition(iNode, &x, &y, &z);
	
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	lua_pushnumber(L, z);

	return 3;
}

static int MashLua_GetNodeRotation(lua_State *L)
{
	float fRoll = 0.0f;
	float fPitch = 0.0f;
	float fYaw = 0.0f;
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);

	if (!lua_isnumber(L, 1))
	{
		printf("GetNodeRotation function failed within script. Parameter is not of type integer.");

		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);

		return 3;
	}

	iResult = _MashLua_GetNodeRotation(iNode, &fRoll, &fPitch, &fYaw);
	
	lua_pushnumber(L, fRoll);
	lua_pushnumber(L, fPitch);
	lua_pushnumber(L, fYaw);

	return 3;
}

static int MashLua_GetNodePositionAbs(lua_State *L)
{
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);

	if (!lua_isnumber(L, 1))
	{
		printf("GetNodePositionAbs function failed within script. Parameter is not of type integer.");

		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);

		return 3;
	}

	iResult = _MashLua_GetSceneNodePositionAbs(iNode, &x, &y, &z);
	
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	lua_pushnumber(L, z);

	return 3;
}

static int MashLua_GetNodeRotationAbs(lua_State *L)
{
	float fRoll = 0.0f;
	float fPitch = 0.0f;
	float fYaw = 0.0f;
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);

	if (!lua_isnumber(L, 1))
	{
		printf("GetNodeRotationAbs function failed within script. Parameter is not of type integer.");

		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);

		return 3;
	}

	iResult = _MashLua_GetNodeRotationAbs(iNode, &fRoll, &fPitch, &fYaw);
	
	lua_pushnumber(L, fRoll);
	lua_pushnumber(L, fPitch);
	lua_pushnumber(L, fYaw);

	return 3;
}

static int MashLua_SetNodeRotation(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float z = (float)lua_tonumber(L, 4);

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
	{
		printf("SetNodeRotation function failed within script. Parameter is not of type integer or float.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	iResult = _MashLua_SetSceneNodeRotation(iNode, x, y, z);

	lua_pushinteger(L, iResult);

	return 1;
}

static int MashLua_SetNodePosition(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float z = (float)lua_tonumber(L, 4);

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
	{
		printf("SetNodePosition function failed within script. Parameter is not of type integer or float.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	iResult = _MashLua_SetSceneNodePosition(iNode, x, y, z);

	lua_pushinteger(L, iResult);

	return 1;
}

static int MashLua_AddNodePosition(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float z = (float)lua_tonumber(L, 4);

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
	{
		printf("AddNodePosition function failed within script. Parameter is not of type integer or float.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	iResult = _MashLua_AddSceneNodePosition(iNode, x, y, z);

	lua_pushinteger(L, iResult);

	return 1;
}


static int MashLua_AddNodeRotation(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float z = (float)lua_tonumber(L, 4);

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
	{
		printf("AddNodeRotation function failed within script. Parameter is not of type integer or float.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	iResult = _MashLua_AddSceneNodeRotation(iNode, x, y, z);

	lua_pushinteger(L, iResult);

	return 1;
}
//static int MashLua_SetNodeSkin(lua_State *L)
//{
//	int iResult = 0;
//	int iNode = (int)lua_tointeger(L, 1);
//	const char *sSkin = lua_tostring(L, 2);
//
//	if (!lua_isnumber(L, 1) || !lua_isstring(L, 2))
//	{
//		printf("SetNodeSkin function failed within script. Parameter is not of type integer or string.");
//
//		lua_pushinteger(L, MASH_LUA_FAIL);
//
//		return 1;
//	}
//
//	iResult = _MashLua_SetNodeSkin(iNode, sSkin);
//
//	lua_pushinteger(L, iResult);
//
//	return 1;
//}

static int MashLua_IsParticle(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);

	if (!lua_isnumber(L, 1))
	{
		printf("IsParticle function failed within script. Parameter is not of type integer.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	iResult = _MashLua_IsParticle(iNode);

	lua_pushboolean(L, iResult);

	return 1;
}

static int MashLua_IsDummy(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);

	if (!lua_isnumber(L, 1))
	{
		printf("IsDummy function failed within script. Parameter is not of type integer.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	iResult = _MashLua_IsDummy(iNode);

	lua_pushboolean(L, iResult);

	return 1;
}

static int MashLua_IsBone(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);

	if (!lua_isnumber(L, 1))
	{
		printf("IsDummy function failed within script. Parameter is not of type integer.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	iResult = _MashLua_IsDummy(iNode);

	lua_pushboolean(L, iResult);

	return 1;
}

static int MashLua_IsEntity(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);

	if (!lua_isnumber(L, 1))
	{
		printf("IsEntity function failed within script. Parameter is not of type integer.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	iResult = _MashLua_IsEntity(iNode);

	lua_pushboolean(L, iResult);

	return 1;
}

static int MashLua_IsLight(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);

	if (!lua_isnumber(L, 1))
	{
		printf("IsLight function failed within script. Parameter is not of type integer.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	iResult = _MashLua_IsLight(iNode);

	lua_pushboolean(L, iResult);

	return 1;
}

static int MashLua_IsCamera(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);

	if (!lua_isnumber(L, 1))
	{
		printf("IsCamera function failed within script. Parameter is not of type integer.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	iResult = _MashLua_IsCamera(iNode);

	lua_pushboolean(L, iResult);

	return 1;
}

static int MashLua_IsVisible(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);

	if (!lua_isnumber(L, 1))
	{
		printf("IsVisible function failed within script. Parameter is not of type integer.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	iResult = _MashLua_IsVisible(iNode);

	lua_pushboolean(L, iResult);

	return 1;
}

//static int MashLua_IsCulled(lua_State *L)
//{
//	int iResult = 0;
//	int iNode = (int)lua_tointeger(L, 1);
//
//	if (!lua_isnumber(L, 1))
//	{
//		printf("IsCulled function failed within script. Parameter is not of type integer.");
//
//		lua_pushinteger(L, MASH_LUA_FAIL);
//
//		return 1;
//	}
//
//	iResult = _MashLua_IsCulled(iNode);
//
//	lua_pushboolean(L, iResult);
//
//	return 1;
//}

static int MashLua_SetVisible(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);
	int iIsVisible = (int)lua_toboolean(L, 1);

	if (!lua_isnumber(L, 1) || !lua_isboolean(L, 2))
	{
		printf("SetVisible function failed within script. Parameter is not of type integer or boolean.");

		lua_pushinteger(L, MASH_LUA_FAIL);

		return 1;
	}

	iResult = _MashLua_SetVisible(iNode, iIsVisible);

	lua_pushboolean(L, iResult);

	return 1;
}

static int MashLua_TransformVectorByNodeAbs(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float z = (float)lua_tonumber(L, 4);
	float newX, newY, newZ;

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
	{
		printf("TransformVectorByNodeAbs function failed within script. Parameter is not of type integer or float.");

		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);

		return 3;
	}

	
	_MashLua_TransformVectorByNodeAbs(iNode, x, y, z, &newX, &newY, &newZ);

	lua_pushnumber(L, newX);
	lua_pushnumber(L, newY);
	lua_pushnumber(L, newZ);

	return 3;
}

static int MashLua_RotateVectorByNodeAbs(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float z = (float)lua_tonumber(L, 4);
	float newX, newY, newZ;

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
	{
		printf("RotateVectorByNodeAbs function failed within script. Parameter is not of type integer or float.");

		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);

		return 3;
	}

	_MashLua_RotateVectorByNodeAbs(iNode, x, y, z, &newX, &newY, &newZ);

	lua_pushnumber(L, newX);
	lua_pushnumber(L, newY);
	lua_pushnumber(L, newZ);

	return 3;
}

static int MashLua_RotateVectorByNodeNormAbs(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float z = (float)lua_tonumber(L, 4);
	float newX, newY, newZ;

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
	{
		printf("RotateVectorByNodeNormAbs function failed within script. Parameter is not of type integer or float.");

		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);

		return 3;
	}

	_MashLua_RotateVectorByNodeNormAbs(iNode, x, y, z, &newX, &newY, &newZ);

	lua_pushnumber(L, newX);
	lua_pushnumber(L, newY);
	lua_pushnumber(L, newZ);

	return 3;
}

//
static int MashLua_TransformVectorByNode(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float z = (float)lua_tonumber(L, 4);
	float newX, newY, newZ;

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
	{
		printf("TransformVectorByNode function failed within script. Parameter is not of type integer or float.");

		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);

		return 3;
	}

	_MashLua_TransformVectorByNode(iNode, x, y, z, &newX, &newY, &newZ);

	lua_pushnumber(L, newX);
	lua_pushnumber(L, newY);
	lua_pushnumber(L, newZ);

	return 3;
}

static int MashLua_RotateVectorByNode(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float z = (float)lua_tonumber(L, 4);
	float newX, newY, newZ;

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
	{
		printf("RotateVectorByNode function failed within script. Parameter is not of type integer or float.");

		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);

		return 3;
	}

	_MashLua_RotateVectorByNode(iNode, x, y, z, &newX, &newY, &newZ);

	lua_pushnumber(L, newX);
	lua_pushnumber(L, newY);
	lua_pushnumber(L, newZ);

	return 3;
}

static int MashLua_RotateVectorByNodeNorm(lua_State *L)
{
	int iResult = 0;
	int iNode = (int)lua_tointeger(L, 1);
	float x = (float)lua_tonumber(L, 2);
	float y = (float)lua_tonumber(L, 3);
	float z = (float)lua_tonumber(L, 4);
	float newX, newY, newZ;

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4))
	{
		printf("RotateVectorByNodeNorm function failed within script. Parameter is not of type integer or float.");

		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);
		lua_pushinteger(L, MASH_LUA_FAIL);

		return 3;
	}

	_MashLua_RotateVectorByNodeNorm(iNode, x, y, z, &newX, &newY, &newZ);

	lua_pushnumber(L, newX);
	lua_pushnumber(L, newY);
	lua_pushnumber(L, newZ);

	return 3;
}
//

static int MashLua_GetFixedTime(lua_State *L)
{
	float t = 0.0f;

	t = _MashLua_GetFixedTime();

	lua_pushnumber(L, t);

	return 1;
}

static const luaL_reg MashSceneLib[] = {
  {"getNode", MashLua_GetSceneNodeID},
  {"getNodeByName", MashLua_GetSceneNodeIDByName},
  {"detachNode", MashLua_DetachSceneNode},
  //{"DestroyNode", MashLua_DestroySceneNode},
  {"attachNode", MashLua_AttachSceneNode},
  {"getNodePosition", MashLua_GetNodePosition},
  {"getNodeRotation", MashLua_GetNodeRotation},
  {"getNodePositionAbs", MashLua_GetNodePositionAbs},
  {"getNodeRotationAbs", MashLua_GetNodeRotationAbs},
  {"setNodeRotation", MashLua_SetNodeRotation},
  {"setNodePosition", MashLua_SetNodePosition},
  {"addNodeRotation", MashLua_AddNodeRotation},
  {"addNodePosition", MashLua_AddNodePosition},
  //{"SetNodeSkin", MashLua_SetNodeSkin},
  {"setVisible", _MashLua_SetVisible},
  {"isEntity", MashLua_IsEntity},
  {"isLight", MashLua_IsLight},
  {"isCamera", MashLua_IsCamera},

  {"isParticle", MashLua_IsParticle},
  {"isDummy", MashLua_IsDummy},
  {"isBone", MashLua_IsBone},

  {"isVisible", MashLua_IsVisible},
  {"getFixedTime", MashLua_GetFixedTime},

  {"transformVectorByNodeAbs", MashLua_TransformVectorByNodeAbs},
  {"rotateVectorByNodeAbs", MashLua_RotateVectorByNodeAbs},
  {"rotateVectorByNodeNormAbs", MashLua_RotateVectorByNodeNormAbs},

  {"transformVectorByNode", MashLua_TransformVectorByNode},
  {"rotateVectorByNode", MashLua_RotateVectorByNode},
  {"rotateVectorByNodeNorm", MashLua_RotateVectorByNodeNorm},

  //{"IsCulled", MashLua_IsCulled},
  {NULL, NULL}
};

LUALIB_API int luaopen_aeroscene (lua_State *L) {
  luaL_openlib(L, LUA_MASH_SCENE_LIB, MashSceneLib, 0);

  return 1;
}