//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_SCENE_NODE_SCRIPT_HANDLER_H_
#define _C_MASH_SCENE_NODE_SCRIPT_HANDLER_H_

#include "MashSceneNodeScriptHandler.h"
#include "MashLuaScriptWrapper.h"

namespace mash
{
	class MashInputManager;
	class MashSceneManager;

	class CMashSceneNodeScriptHandler : public MashSceneNodeScriptHandler
	{
	private:
		enum eEVENT_FLAGS
		{
			aEVENTFLAG_UPDATE = 1,
            aEVENTFLAG_MOUSE_ENTER = 2,
            aEVENTFLAG_MOUSE_EXIT = 4,
			aEVENTFLAG_ATTACH = 8
		};

		enum eEVENT_INDEX
		{
			aEVENTINDEX_UPDATE,
			//aEVENTINDEX_VISIBLE,
			//aEVENTINDEX_INVISIBLE,
			aEVENTINDEX_ATTACH,
			//aEVENTINDEX_DETACH,
			aEVENTINDEX_MOUSE_ENTER,
			aEVENTINDEX_MOUSE_EXIT,

			aEVENTINDEX_COUNT
		};

		bool m_bIsVisible;
		bool m_bMouseEnter;

		MashInputManager *m_pInputManager;
		MashSceneManager *m_pSceneManager;
		uint32 m_iFunctionFlags;
		MashLuaScriptWrapper *m_pScript;

		void RegisterAllFunctions();
		eMASH_STATUS CallEventScriptFunction(eEVENT_INDEX eEvent);
	public:
		CMashSceneNodeScriptHandler(MashSceneManager *pSceneManager,
			MashInputManager *pInputManager,
			MashLuaScriptWrapper *pScript);

		~CMashSceneNodeScriptHandler();

		void OnNodeAttach(MashSceneNode *pSceneNode);
		void OnNodeUpdate(MashSceneNode *pSceneNode, f32 dt);
	};
}

#endif