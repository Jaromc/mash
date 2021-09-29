//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashSceneNodeScriptHandler.h"
#include "MashLog.h"
#include "MashInputManager.h"
#include "MashVideo.h"
#include "MashSceneManager.h"
#include "MashGeometryHelper.h"
#include "MashCamera.h"
#include "MashDevice.h"
#include "MashRay.h"
namespace mash
{
	/*
		The number of event names must match the number
		of eventFlags
	*/
	static const int8* const sEventNames[] = 
	{
		"onUpdate",
		"onAttach",
		"onMouseEnter",
		"onMouseExit",
		0
	};

	CMashSceneNodeScriptHandler::CMashSceneNodeScriptHandler(MashSceneManager *pSceneManager,
			MashInputManager *pInputManager,
			MashLuaScriptWrapper *pScript):MashSceneNodeScriptHandler(), m_pSceneManager(pSceneManager),
			m_pInputManager(pInputManager), m_pScript(pScript), m_iFunctionFlags(0), 
			m_bMouseEnter(false), m_bIsVisible(false)
	{
		if (m_pScript)
			m_pScript->Grab();

		RegisterAllFunctions();
	}

	CMashSceneNodeScriptHandler::~CMashSceneNodeScriptHandler()
	{
		//TODO : Remove this properly
		if (m_pScript)
		{
			m_pScript->Drop();
			m_pScript = 0;
		}
	}

	void CMashSceneNodeScriptHandler::OnNodeAttach(MashSceneNode *pSceneNode)
	{
		if (!pSceneNode)
			return;

		//do some init
		m_bIsVisible = pSceneNode->IsVisible();

		if (m_iFunctionFlags & aEVENTFLAG_ATTACH)
		{
			m_pSceneManager->_SetCurrentScriptSceneNode(pSceneNode);
			CallEventScriptFunction(aEVENTINDEX_ATTACH);
		}
	}

	void CMashSceneNodeScriptHandler::RegisterAllFunctions()
	{
		MashLuaScript *script = m_pScript->GetScript();
		if (script->GetFunctionExists(sEventNames[aEVENTINDEX_UPDATE]))
			m_iFunctionFlags |= aEVENTFLAG_UPDATE;
		if (script->GetFunctionExists(sEventNames[aEVENTINDEX_ATTACH]))
			m_iFunctionFlags |= aEVENTFLAG_ATTACH;
		if (script->GetFunctionExists(sEventNames[aEVENTINDEX_MOUSE_ENTER]))
			m_iFunctionFlags |= aEVENTFLAG_MOUSE_ENTER;
		if (script->GetFunctionExists(sEventNames[aEVENTINDEX_MOUSE_EXIT]))
			m_iFunctionFlags |= aEVENTFLAG_MOUSE_EXIT;
	}

	eMASH_STATUS CMashSceneNodeScriptHandler::CallEventScriptFunction(eEVENT_INDEX eEvent)
	{
		if (m_pScript->GetScript()->CallFunction(sEventNames[eEvent], 0, 0) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
							"CMashSceneNodeScriptHandler::CallEventScriptFunction",
							"Event failed at : %s",
							sEventNames[eEvent]);

			return aMASH_FAILED;
		}

		return aMASH_OK;
	}

	void CMashSceneNodeScriptHandler::OnNodeUpdate(MashSceneNode *pSceneNode, f32 dt)
	{
		if (!pSceneNode)
			return;

		m_pSceneManager->_SetCurrentScriptSceneNode(pSceneNode);

		if (m_iFunctionFlags & aEVENTFLAG_UPDATE)
			CallEventScriptFunction(aEVENTINDEX_UPDATE);

		if ((m_iFunctionFlags & aEVENTFLAG_MOUSE_ENTER) || 
			(m_iFunctionFlags & aEVENTFLAG_MOUSE_EXIT))
		{
			MashCamera *pActiveCamera = m_pSceneManager->GetActiveCamera();
			if (pActiveCamera)
			{
				mash::MashVector2 mousePos = m_pInputManager->GetCursorPosition();
				const sMashViewPort *vp = &MashDevice::StaticDevice->GetRenderer()->GetViewport();

				mash::MashRay ray;
				pActiveCamera->TransformScreenToWorldPosition(MashVector2(vp->width, vp->height),
					mousePos,
					ray.origin,
					ray.dir);

				bool bCollision = mash::collision::Ray_AABB(pSceneNode->GetWorldBoundingBox(), ray);
				if (bCollision && !m_bMouseEnter)
				{
					if (m_iFunctionFlags & aEVENTFLAG_MOUSE_ENTER)
						CallEventScriptFunction(aEVENTINDEX_MOUSE_ENTER);
				}
				else if (!bCollision && m_bMouseEnter)
				{
					if (m_iFunctionFlags & aEVENTFLAG_MOUSE_EXIT)
						CallEventScriptFunction(aEVENTINDEX_MOUSE_EXIT);
				}

				m_bMouseEnter = bCollision;
			}
		}
	}
}