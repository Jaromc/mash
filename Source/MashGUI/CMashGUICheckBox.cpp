//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUICheckBox.h"
#include "MashGUIManager.h"
namespace mash
{

	CMashGUICheckBox::CMashGUICheckBox(MashGUIManager *pGUIManager,
		MashInputManager *pInputManager,
		MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement):MashGUICheckBox(pGUIManager, pInputManager, pParent, destination),
			m_bIsChecked(false), m_styleElement(styleElement)
	{

	}

	CMashGUICheckBox::~CMashGUICheckBox()
	{
		
	}

	void CMashGUICheckBox::SetChecked(bool bIsChecked)
	{
		if (bIsChecked != m_bIsChecked)
		{
			m_bIsChecked = bIsChecked;

			sGUIEvent newGUIMsg;
			

			if (m_bIsChecked)
				newGUIMsg.GUIEvent = aGUIEVENT_CB_TOGGLE_ON;
			else
				newGUIMsg.GUIEvent = aGUIEVENT_CB_TOGGLE_OFF;

			newGUIMsg.component = this;
			ImmediateBroadcast(newGUIMsg);
		}
	}

	void CMashGUICheckBox::OnEvent(const sInputEvent &eventData)
	{
		if (GetEventsEnabled())
		{
			if (eventData.action == aMOUSEEVENT_B1)
			{
				switch(eventData.isPressed)
				{
				case 0:
					{
                        bool bNewCheckedState = !m_bIsChecked;
                        SetChecked(bNewCheckedState);
						break;
					}

				};
			}
		}
	}

	void CMashGUICheckBox::Draw()
	{
		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			MashGUIComponent::Draw();

			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
			MashGUISkin *activeSkin = 0;
			if (m_bIsChecked)
				activeSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_CHECKBOX_ON);
			else
				activeSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_CHECKBOX_OFF);

			m_GUIManager->DrawSprite(m_absoluteRegion, m_absoluteClippedRegion, activeSkin, m_overrideTransparency);
		}
	}
}