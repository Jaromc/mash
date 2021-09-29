

#include <stdlib.h>


#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "../CMashScriptAccessors.h"

#define aINPUT_KEYBOARD		0x00
#define aINPUT_MOUSE		0x01

//static int MashLuaLib_GetIsKeyUp(lua_State *L)
//{
//	int iKey = (int)lua_tonumber(L, 1);
//
//	if (!lua_isnumber(L, 1))
//	{
//		printf("GetIsKeyUp function failed within script. Parameter is not of type integer.");
//		return 0;
//	}
//
//	lua_pushboolean(L, _MashLua_GetIsKeyUp(iKey));
//
//	return 1;
//}
//
//static int MashLuaLib_GetIsKeyDown(lua_State *L)
//{
//	int iKey = (int)lua_tonumber(L, 1);
//
//	if (!lua_isnumber(L, 1))
//	{
//		printf("GetIsKeyDown function failed within script. Parameter is not of type integer.");
//		return 0;
//	}
//
//	lua_pushboolean(L, _MashLua_GetIsKeyDown(iKey));
//
//	return 1;
//}

static int MashLua_GetIsReleased(lua_State *L)
{
	int iType = (int)lua_tointeger(L, 1);
	int iKey = (int)lua_tointeger(L, 2);

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
	{
		printf("IsReleased function failed within script. Parameter is not of type integer.");
		lua_pushinteger(L, -1);

		return 1;
	}

	/*
		There is type checking within the input manager.
		So if the user enters something invalid it should just return false.
	*/
	lua_pushboolean(L, _MashLua_GetIsReleased(iType, iKey));

	return 1;
}

static int MashLuaLib_GetIsPressed(lua_State *L)
{
	int iType = (int)lua_tointeger(L, 1);
	int iKey = (int)lua_tointeger(L, 2);

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
	{
		printf("IsPressed function failed within script. Parameter is not of type integer.");
		lua_pushinteger(L, -1);

		return 1;
	}

	/*
		There is type checking within the input manager.
		So if the user enters something invalid it should just return false.
	*/
	lua_pushboolean(L, _MashLua_GetIsPressed(iType, iKey));

	return 1;
}

static int MashLuaLib_GetIsHeld(lua_State *L)
{
	int iType = (int)lua_tointeger(L, 1);
	int iKey = (int)lua_tointeger(L, 2);

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
	{
		printf("IsHeld function failed within script. Parameter is not of type integer.");
		return 0;
	}

	/*
		There is type checking within the input manager.
		So if the user enters something invalid it should just return false.
	*/
	lua_pushboolean(L, _MashLua_GetIsHeld(iType, iKey));

	return 1;
}

static int MashLuaLib_GetValue(lua_State *L)
{
	int player = (int)lua_tointeger(L, 1);
	int iKey = (int)lua_tointeger(L, 2);

	if (!lua_isnumber(L, 1) || !lua_isnumber(L, 2))
	{
		printf("GetValue function failed within script. Parameter is not of type integer.");
		return 0;
	}

	/*
		There is type checking within the input manager.
		So if the user enters something invalid it should just return false.
	*/
	lua_pushnumber(L, _MashLua_GetKeyValue(player, iKey));

	return 1;
}

static const luaL_reg MashInputLib[] = {
  {"isPressed", MashLuaLib_GetIsPressed},
  {"isReleased", MashLua_GetIsReleased},
  {"isHeld", MashLuaLib_GetIsHeld},
  {"getValue", MashLuaLib_GetValue},
  {NULL, NULL}
};

LUALIB_API int luaopen_aeroinput (lua_State *L) {
  luaL_openlib(L, LUA_MASH_INPUT_LIB, MashInputLib, 0);

  return 1;
}