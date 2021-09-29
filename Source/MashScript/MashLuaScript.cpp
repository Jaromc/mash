//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashLuaScript.h"

#include "CMashLuaHelper.h"
#include "MashLog.h"

extern "C"
{
#include "./lua/lua.h"
#include "./lua/lualib.h"
#include "./lua/lauxlib.h"
}

namespace mash
{
	MashLuaScript::MashLuaScript()
	{
	}

	MashLuaScript::~MashLuaScript()
	{

	}

	bool MashLuaScript::GetFunctionExists(const int8 *sFunctionName)const
	{
		lua_State *luaState = MashLuaState_to_lua_State(this);
		lua_getfield(luaState, LUA_GLOBALSINDEX, sFunctionName);
		int32 iResult = lua_isfunction(luaState, -1);

		lua_pop(luaState,-1);

		if (iResult)
			return true;

		return false;
	}

	eMASH_STATUS MashLuaScript::CallFunction(const int8 *functionName, int32 iNumParameters, int32 iNumResults)
	{
		lua_State *luaState = MashLuaState_to_lua_State(this);
		lua_getglobal(luaState, functionName);
		lua_call(luaState, iNumParameters, iNumResults);

		return aMASH_OK;
	}

	int32 MashLuaScript::GetTop()
	{
		return lua_gettop(MashLuaState_to_lua_State(this));
	}

	int32 MashLuaScript::GetInt(int32 iIndex)
	{
		lua_State *luaState = MashLuaState_to_lua_State(this);
		if (!lua_isnumber(luaState, iIndex))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Lua variable is of incorrect type", 
				"MashLuaScript::GetInt");

			return 0;
		}

		return (int32)lua_tonumber(luaState, iIndex);
	}

	f32 MashLuaScript::GetFloat(int32 index)
	{
		lua_State *luaState = MashLuaState_to_lua_State(this);
		if (!lua_isnumber(luaState, index))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Lua variable is of incorrect type", 
				"MashLuaScript::GetFloat");

			return 0.0f;
		}

		return (f32)lua_tonumber(luaState, index);
	}

	bool MashLuaScript::GetBool(int32 index)
	{
		lua_State *luaState = MashLuaState_to_lua_State(this);
		if (!lua_isnumber(luaState, index))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Lua variable is of incorrect type", 
				"MashLuaScript::GetBool");

			return false;
		}

		return (bool)lua_toboolean(luaState, index);
	}

	void MashLuaScript::GetString(int32 index, MashStringc &out)
	{
		lua_State *luaState = MashLuaState_to_lua_State(this);
		if (!lua_isstring(luaState, index))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Lua variable is of incorrect type", 
				"MashLuaScript::GetBool");

			return;
		}

		out = lua_tostring(luaState, index);
	}

	void MashLuaScript::PushFloat(f32 value)
	{
		lua_pushnumber(MashLuaState_to_lua_State(this), value);
	}

	void MashLuaScript::PushInt(int32 value)
	{
		lua_pushnumber(MashLuaState_to_lua_State(this), value);
	}

	void MashLuaScript::PushBool(bool value)
	{
		lua_pushboolean(MashLuaState_to_lua_State(this), value);
	}

	void MashLuaScript::PushString(const int8 *value)
	{
		lua_pushstring(MashLuaState_to_lua_State(this), value);
	}

	void MashLuaScript::PushUserData(void* value)
	{
		lua_pushlightuserdata(MashLuaState_to_lua_State(this), value);
	}

	int32 MashLuaScript::GetGlobalInt(const int8 *name)
	{
		lua_State *luaState = MashLuaState_to_lua_State(this);

		//reset the stack index
		lua_settop(luaState, 0);
		
		//put the lua global variable "name" on the stack
		lua_getglobal(luaState, name);

		//check the varibale is the correct type
		if (!lua_isnumber(luaState,1))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Lua variable is of incorrect type", "MashLuaScript::GetGlobalInt");

			return 0;
		}

		//grab the value and cast to the correct type
		int32 val = (int32)lua_tonumber(luaState,1);

		//remove the value from the stack
		lua_pop(luaState,1);

		return val;
	}

	f32 MashLuaScript::GetGlobalFloat(const int8 *name)
	{
		//return MashLuaHelper::GetGlobalFloat(this, name, out);
		lua_State *luaState = MashLuaState_to_lua_State(this);

		//reset the stack index
		lua_settop(luaState, 0);
		
		//put the lua global variable "name" on the stack
		lua_getglobal(luaState, name);

		//check the varibale is the correct type
		if (!lua_isnumber(luaState,1))
		{
			//if (CMashLog::IsMashLoggingEnabled)
			//	CMashLog::WriteToLog(MashErrorMsg("lua variable is of incorrect type : " + MashStringc(name)));
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Lua variable is of incorrect type", "MashLuaScript::GetGlobalFloat");

			return 0.0f;
		}

		//grab the value and cast to the correct type
		f32 val = (f32)lua_tonumber(luaState,1);

		//remove the value from the stack
		lua_pop(luaState,1);

		return val;
	}

	bool MashLuaScript::GetGlobalBool(const int8 *name)
	{
		//return MashLuaHelper::GetGlobalBool(this, name, out);
		lua_State *luaState = MashLuaState_to_lua_State(this);

		//reset the stack index
		lua_settop(luaState,0);
		//put the lua global variable "name" on the stack
		lua_getglobal(luaState,name);
		//check the varibale is the correct type
		if (!lua_isboolean(luaState,1))
		{
			//if (CMashLog::IsMashLoggingEnabled)
			//	CMashLog::WriteToLog(MashErrorMsg("lua variable is of incorrect type : " + MashStringc(name)));
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Lua variable is of incorrect type", "MashLuaScript::GetGlobalBool");

			return false;
		}

		bool b = lua_toboolean(luaState,1);
		//remove the value from the stack
		lua_pop(luaState,1);

		return b;
	}

	void MashLuaScript::GetGlobalString(const int8 *name, MashStringc &out)
	{
		//return MashLuaHelper::GetGlobalString(this, name, out);
		lua_State *luaState = MashLuaState_to_lua_State(this);

		lua_settop(luaState, 0);

		lua_getglobal(luaState, name);
		//check the varibale is the correct type
		if (!lua_isstring(luaState,1))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Lua variable is of incorrect type", "MashLuaScript::GetGlobalString");

			return;
		}

		out = lua_tostring(luaState,1);
		//remove the value from the stack
		lua_pop(luaState,1);
	}

	void MashLuaScript::SetGlobalInt(const int8 *name, int32 data)
	{
		PushInt(data);
		lua_setglobal(MashLuaState_to_lua_State(this), name);
	}

	void MashLuaScript::SetGlobalFloat(const int8 *name, f32 data)
	{
		PushFloat(data);
		lua_setglobal(MashLuaState_to_lua_State(this), name);
	}

	void MashLuaScript::SetGlobalBool(const int8 *name, bool data)
	{
		PushBool(data);
		lua_setglobal(MashLuaState_to_lua_State(this), name);
	}

	void MashLuaScript::SetGlobalString(const int8 *name, const int8 *data)
	{
		PushString(data);
		lua_setglobal(MashLuaState_to_lua_State(this), name);
	}
}