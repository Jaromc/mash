

#include <stdlib.h>


#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "MashLuaLib_Key.h"
/*
	This is accessed by the engine so the user can define their own key strings.
	The engine is responsible for creating and deleting.
*/
struct sMashLuaKeyValue *g_aeroLuaKeyValueList = 0;
unsigned int g_aeroLuaKeyValueCount = 0;

static const luaL_reg MashInputKeyLib[] = {
  {NULL, NULL}
};

LUALIB_API int luaopen_aeroinputkey (lua_State *L) 
{
	int i;

	luaL_openlib(L, LUA_MASH_INPUT_KEYS_LIB, MashInputKeyLib, 0);

	for(i = 0; i < g_aeroLuaKeyValueCount; ++i)
	{
		lua_pushinteger(L, g_aeroLuaKeyValueList[i].value);
		lua_setfield(L, -2, g_aeroLuaKeyValueList[i].keyString);
	}

  return 1;
}   