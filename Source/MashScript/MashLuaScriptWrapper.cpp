//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashLuaScriptWrapper.h"
#include "MashScriptManager.h"
namespace mash
{
	MashLuaScriptWrapper::MashLuaScriptWrapper(MashScriptManager *scriptManager, MashLuaScript *script):m_scriptManager(scriptManager), m_script(script)
	{
		m_scriptManager->Grab();
	}

	MashLuaScriptWrapper::~MashLuaScriptWrapper()
	{
		m_scriptManager->_DestroyLuaScript(m_script);
		m_scriptManager->Drop();
	}
}