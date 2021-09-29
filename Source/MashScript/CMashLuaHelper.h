//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_LUA_HELPER_H_
#define _C_MASH_LUA_HELPER_H_

namespace mash
{
	#define MashLuaState_to_lua_State(state) ((lua_State*)(state))
	#define lua_State_To_MashLuaState(L) ((MashLuaScript*)L)
}

#endif