//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashScriptManager.h"
#include "MashLuaScriptWrapper.h"
#include "MashFileManager.h"
#include "MashLog.h"
#include "MashDevice.h"
#include "CMashScriptAccessors.h"
#include "MashLuaLib_Key.h"
#include "MashLuaLib_User.h"
#include "MashInputManager.h"
#include "CMashSceneNodeScriptHandler.h"
#include "MashFileStream.h"

extern "C"
{
#include "./lua/lua.h"
#include "./lua/lualib.h"
#include "./lua/lauxlib.h"
}



namespace mash
{
	CMashScriptManager *g_scriptManager = 0;

	MashScriptManager* CreateMashScriptManager()
	{
		g_scriptManager = MASH_NEW_COMMON CMashScriptManager();
		return g_scriptManager;
	}

	CMashScriptManager::CMashScriptManager():m_device(0)
	{
	}

	CMashScriptManager::~CMashScriptManager()
	{
		if (g_aeroLuaKeyValueList)
		{
			for(uint32 i = 0; i < g_aeroLuaKeyValueCount; ++i)
				MASH_FREE(g_aeroLuaKeyValueList[i].keyString);

			MASH_FREE(g_aeroLuaKeyValueList);
			g_aeroLuaKeyValueList = 0;
		}

		if (g_aeroLuaUserFunctionList)
		{
			for(uint32 i = 0; i < g_aeroLuaUserFunctionCount; ++i)
				MASH_FREE(g_aeroLuaUserFunctionList[i].callingString);

			MASH_FREE(g_aeroLuaUserFunctionList);
			g_aeroLuaUserFunctionList = 0;
		}
	}

	eMASH_STATUS CMashScriptManager::_Initialise(MashDevice *device)
	{
		if (MashLuaAccessorInitialise(device) == 0)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to initialise script accessors.", "CMashScriptManager::_Initialise");
			return aMASH_FAILED;
		}

		m_device = device;

		return aMASH_OK;
	}

	int32 CMashScriptManager::_CallUserFunction(void *state, uint32 functionId)
	{
		return m_functionMapping[functionId].functPtr(lua_State_To_MashLuaState((lua_State*)state));
	}

	void CMashScriptManager::SetLibUserFunctions(sMashLuaUserFunction *functPtrList)
	{
		static uint32 functionCounter = 0;

		if (g_aeroLuaUserFunctionList)
		{
			for(uint32 i = 0; i < g_aeroLuaUserFunctionCount; ++i)
				MASH_FREE(g_aeroLuaUserFunctionList[i].callingString);

			MASH_FREE(g_aeroLuaUserFunctionList);

			m_functionMapping.Clear();
		}

		g_aeroLuaUserFunctionList = 0;
		g_aeroLuaUserFunctionCount = 0;

		if (!functPtrList)
			return;

		while(functPtrList[g_aeroLuaUserFunctionCount].functPtr)
			++g_aeroLuaUserFunctionCount;

		if (g_aeroLuaUserFunctionCount == 0)
			return;

		g_aeroLuaUserFunctionList = (sMashLuaUserFunctionImpl*)MASH_ALLOC_COMMON(sizeof(sMashLuaUserFunctionImpl) * g_aeroLuaUserFunctionCount);

		for(uint32 i = 0; i < g_aeroLuaUserFunctionCount; ++i)
		{
			g_aeroLuaUserFunctionList[i].callingString = (int8*)MASH_ALLOC_COMMON(strlen(functPtrList[i].callingString) + 1);
			strcpy(g_aeroLuaUserFunctionList[i].callingString, functPtrList[i].callingString);
			g_aeroLuaUserFunctionList[i].functionId = functionCounter;

			m_functionMapping.PushBack(sFunctionMapping(functPtrList[i].functPtr));
			++functionCounter;
		}
	}

	void CMashScriptManager::SetLibInputValues(sMashLuaKeyValue *values)
	{
		if (g_aeroLuaKeyValueList)
		{
			for(uint32 i = 0; i < g_aeroLuaKeyValueCount; ++i)
				MASH_FREE(g_aeroLuaKeyValueList[i].keyString);

			MASH_FREE(g_aeroLuaKeyValueList);
		}

		g_aeroLuaKeyValueList = 0;
		g_aeroLuaKeyValueCount = 0;

		if (!values)
			return;

		while(values[g_aeroLuaKeyValueCount].keyString)
			++g_aeroLuaKeyValueCount;

		if (g_aeroLuaKeyValueCount == 0)
			return;

		g_aeroLuaKeyValueList = (sMashLuaKeyValue*)MASH_ALLOC_COMMON(sizeof(sMashLuaKeyValue) * g_aeroLuaKeyValueCount);

		for(uint32 i = 0; i < g_aeroLuaKeyValueCount; ++i)
		{
			g_aeroLuaKeyValueList[i].keyString = (int8*)MASH_ALLOC_COMMON(strlen(values[i].keyString) + 1);
			strcpy(g_aeroLuaKeyValueList[i].keyString, values[i].keyString);
			g_aeroLuaKeyValueList[i].value = values[i].value;
		}

		if (m_device)
			m_device->GetInputManager()->EnabledInputHelpers(true);
	}

	MashLuaScript* CMashScriptManager::CreateLuaState(const int8 *scriptName)
	{
		MashFileStream *fileStream = m_device->GetFileManager()->CreateFileStream();
		if (fileStream->LoadFile(scriptName, aFILE_IO_TEXT) == aMASH_FAILED)
		{
			int8 buffer[256];
			sprintf(buffer, "Failed to read lua file '%s'.", scriptName);
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, buffer, "CMashScriptManager::CreateLuaState");
			fileStream->Destroy();
			return 0;
		}

		lua_State *newState = lua_open();

		//TODO : Use luaL_loadfile(). It returns errors and doesnt assert.
		if (luaL_dostring(newState, (const int8*)fileStream->GetData()) != 0)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Lua script failed to load.", "CMashScriptManager::CreateLuaState");

			lua_close(newState);

			return 0;
		}

		fileStream->Destroy();

		//open libraries for this script
		luaL_openlibs(newState );

		MashLuaScript *aeroLuaState = lua_State_To_MashLuaState(newState);

		return aeroLuaState;
	}

	MashLuaScriptWrapper* CMashScriptManager::CreateLuaScript(const int8 *scriptName)
	{
		MashLuaScript *pMashLuaScript = 0;

		if (!pMashLuaScript)
		{
			pMashLuaScript = CreateLuaState(scriptName);
		}

		if (!pMashLuaScript)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Lua script failed to load.", "CMashScriptManager::CreateLuaScript");

			return 0;
		}

		MashLuaScriptWrapper *scriptWrapper = MASH_NEW_COMMON MashLuaScriptWrapper(this, pMashLuaScript);

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
						"CMashScriptManager::CreateLuaScript",
						"Lua script loaded '%s'.",
						scriptName);

		return scriptWrapper;
	}

	MashSceneNodeScriptHandler* CMashScriptManager::CreateSceneNodeScriptHandler(MashLuaScriptWrapper *pScript)
	{
		if (!pScript)
			return 0;

		CMashSceneNodeScriptHandler *pNewScripHandler = MASH_NEW_COMMON CMashSceneNodeScriptHandler(m_device->GetSceneManager(),
			m_device->GetInputManager(),
			pScript);

		//the handler now owns it
		pScript->Drop();

		return pNewScripHandler;
	}

	eMASH_STATUS CMashScriptManager::_DestroyLuaScript(MashLuaScript *pScript)
	{
		lua_close(MashLuaState_to_lua_State(pScript));

		return aMASH_FAILED;
	}

}