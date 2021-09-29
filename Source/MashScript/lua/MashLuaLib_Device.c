#include "MashLuaLib_Device.h"
#include "MashLuaLibSettings.h"
#include "../CMashScriptAccessors.h"


	int _MashLuaGetIsKeyDown(int iKey)
	{
		return _MashLua_GetIsKeyDown(iKey);
		//g_pMashLuaDevice->GetInputManager()->IsPressed(mash::eMASH_INPUT_TYPE::aINPUT_KEYBOARD, iKey);
	}


