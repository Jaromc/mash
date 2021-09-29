//
//
//#include <stdlib.h>
//
//
//#include "lua.h"
//#include "lualib.h"
//#include "lauxlib.h"
//
//#include "../CMashScriptAccessors.h"
//
//#define aBTN_1               0x00
//#define aBTN_2               0x01
//#define aBTN_3               0x02
//#define aBTN_4               0x03
//#define aBTN_5               0x04
//#define aBTN_6               0x05
//#define aBTN_7               0x06
//#define aBTN_8               0x07
//#define aBTN_9               0x08
//
//static const luaL_reg MashInputLibBtn[] = {
//  {NULL, NULL}
//};
//
//LUALIB_API int32 luaopen_aeroinputbtn (lua_State *L) {
//  luaL_openlib(L, LUA_MASH_INPUT_BTN_LIB, MashInputLibBtn, 0);
//
//  lua_pushnumber(L, aBTN_1);
//  lua_setfield(L, -2, "a");
//  lua_pushnumber(L, aBTN_2);
//  lua_setfield(L, -2, "b");
//  lua_pushnumber(L, aBTN_3);
//  lua_setfield(L, -2, "c");
//  lua_pushnumber(L, aBTN_4);
//  lua_setfield(L, -2, "d");
//  lua_pushnumber(L, aBTN_5);
//  lua_setfield(L, -2, "e");
//  lua_pushnumber(L, aBTN_6);
//  lua_setfield(L, -2, "f");
//  lua_pushnumber(L, aBTN_7);
//  lua_setfield(L, -2, "g");
//  lua_pushnumber(L, aBTN_8);
//  lua_setfield(L, -2, "h");
//  lua_pushnumber(L, aBTN_9);
//  lua_setfield(L, -2, "i");
//
//  return 1;
//}