//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_SCRIPT_H_
#define _C_MASH_SCRIPT_H_


#include "MashScriptManager.h"
#include "CMashLuaHelper.h"
#include <map>
#include "MashArray.h"
#include "MashString.h"

namespace mash
{
	
	class CMashScriptManager : public MashScriptManager
	{
	public:
		struct sFunctionMapping
		{
			int32 (*functPtr)(MashLuaScript *script);

			sFunctionMapping(){}
			sFunctionMapping(int32 (*_functPtr)(MashLuaScript *script)):functPtr(_functPtr){}
		};
	public:
		MashArray<sFunctionMapping> m_functionMapping;
		MashDevice *m_device;
		MashLuaScript* CreateLuaState(const int8 *scriptName);
	public:
		CMashScriptManager();
		virtual ~CMashScriptManager();

		virtual MashLuaScriptWrapper* CreateLuaScript(const int8 *scriptName);
		virtual eMASH_STATUS _DestroyLuaScript(MashLuaScript *pScript);

		eMASH_STATUS _Initialise(MashDevice *device);
		void SetLibInputValues(sMashLuaKeyValue *values);
		void SetLibUserFunctions(sMashLuaUserFunction *functPtrList);

		MashSceneNodeScriptHandler* CreateSceneNodeScriptHandler(MashLuaScriptWrapper *pScript);
		int32 _CallUserFunction(void *state, uint32 functionId);
	};
}



#endif