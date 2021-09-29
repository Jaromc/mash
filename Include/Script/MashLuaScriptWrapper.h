//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_LUA_SCRIPT_WRAPPER_H_
#define _MASH_LUA_SCRIPT_WRAPPER_H_

#include "MashReferenceCounter.h"
#include "MashLuaScript.h"

namespace mash
{
	class MashScriptManager;

    /*!
        Script wrapper for reference counting a script. The script should never be
        deleted directly, instead, call Drop() from this wrapper when you are done with 
        the script.
    */
	class MashLuaScriptWrapper : public MashReferenceCounter
	{
	private:
		MashLuaScript *m_script;
		MashScriptManager *m_scriptManager;
	public:
		MashLuaScriptWrapper(MashScriptManager *scriptManager, MashLuaScript *script);
		~MashLuaScriptWrapper();

        //! Gets the script.
		MashLuaScript* GetScript()const;
	};

	inline MashLuaScript* MashLuaScriptWrapper::GetScript()const
	{
		return m_script;
	}
}

#endif