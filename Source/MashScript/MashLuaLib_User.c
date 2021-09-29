//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------


#include <stdlib.h>


#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "MashLuaLib_User.h"


/*
	This is accessed by the engine so the user can define their own functions.
	The engine is responsible for creating and deleting.
*/
struct sMashLuaUserFunctionImpl *g_aeroLuaUserFunctionList = 0;
unsigned int g_aeroLuaUserFunctionCount = 0;

static const luaL_reg MashUserFunctionsLib[] = {
  {NULL, NULL}
};

LUALIB_API int luaopen_aerouserfunction (lua_State *L) 
{
	int i;

	luaL_openlib(L, LUA_MASH_INPUT_USER_LIB, MashUserFunctionsLib, 0);

	for(i = 0; i < g_aeroLuaUserFunctionCount; ++i)
	{
		lua_pushcfunction_id(L, g_aeroLuaUserFunctionList[i].functionId);
		lua_setfield(L, -2, g_aeroLuaUserFunctionList[i].callingString);
	}

  return 1;
}   