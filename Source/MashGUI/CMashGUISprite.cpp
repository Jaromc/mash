//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUISprite.h"
#include "MashGUIManager.h"

namespace mash
{
	CMashGUISprite::CMashGUISprite(MashGUIManager *pGUIManager,
		MashInputManager *pInputManager,
		MashGUIComponent *pParent,
		const MashGUIRect &destination,
		int32 styleElement):MashGUISprite(pGUIManager, pInputManager, pParent, destination), m_styleElement(styleElement)
	{
	}

	CMashGUISprite::~CMashGUISprite()
	{
		
	}

	void CMashGUISprite::Draw()
	{
		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			MashGUIComponent::Draw();
			m_GUIManager->DrawSprite(m_absoluteRegion, m_absoluteClippedRegion, &m_skin, m_overrideTransparency);
		}
	}
}