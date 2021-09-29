//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUITabControl.h"
#include "MashGUIManager.h"
namespace mash
{
	CMashGUITabControl::CMashGUITabControl(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement):MashGUITabControl(pGUIManager, pInputManager, pParent, destination),
			m_fTabWidthTotal(0.0f),
			m_iActiveTab(-1), m_iHoverTab(-1), m_bForceTabUpdate(false), m_styleElement(styleElement),
			m_tabIDCount(0)
	{
		m_absoluteTabRegion = m_absoluteRegion;

		const MashGUIStyle *style = pGUIManager->GetActiveGUIStyle();
		if (style)
		{
			m_absoluteTabRegion.bottom = m_absoluteTabRegion.top + style->GetFont()->GetMaxCharacterHeight();
		}

		/*
			Always have at least 1 tab
		*/
		AddTab("Tab 1");
	}

	CMashGUITabControl::~CMashGUITabControl()
	{
		const uint32 tabCount = m_tabs.Size();
		for(uint32 i = 0; i < tabCount; ++i)
		{
			if (m_tabs[i].pTab)
				m_tabs[i].pTab->Destroy();
		}

		m_tabs.Clear();
	}

	void CMashGUITabControl::OnStyleChange(MashGUIStyle *style)
	{
		const uint32 iItemCount = m_tabs.Size();
		for(uint32 i = 0; i < iItemCount; ++i)
		{
			if (m_tabs[i].pTab)
			{
				m_tabs[i].textHandler.SetFont(style->GetFont());
				m_tabs[i].pTab->OnStyleChange(style);
			}
		}
	}

	void CMashGUITabControl::SetOverrideTransparency(bool state, uint8 alpha, bool affectFont, f32 alphaMaskThreshold)
	{
		MashGUIComponent::SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);

		const uint32 iItemCount = m_tabs.Size();
		for(uint32 i = 0; i < iItemCount; ++i)
		{
			if (m_tabs[i].pTab)
				m_tabs[i].pTab->SetOverrideTransparency(state, alpha, affectFont, alphaMaskThreshold);
		}
	}

	void CMashGUITabControl::SetActiveTabByID(int32 iTabID)
	{
		m_iActiveTab = iTabID;
	}

	void CMashGUITabControl::UpdateTabControl(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
		MashGUIFont *activeFont = activeStyle->GetFont();

		const f32 tabHeightBuffer = 5.0f;
		const f32 tabWidthBuffer = 10.0f;
		//this is the background behin the tabs (not the window)
		m_absoluteTabRegion = m_absoluteRegion;
		m_absoluteTabRegion.bottom = m_absoluteTabRegion.top + activeFont->GetMaxCharacterHeight() + tabHeightBuffer;

		if (positionChangeOnly)
		{
			const uint32 iItemCount = m_tabs.Size();
			for(uint32 i = 0; i < iItemCount; ++i)
			{
				sTab *tab = &m_tabs[i];
				tab->absoluteClippedRect.left += deltaX;
				tab->absoluteClippedRect.right += deltaX;
				tab->absoluteClippedRect.top += deltaY;
				tab->absoluteClippedRect.bottom += deltaY;

				tab->absoluteRect.left += deltaX;
				tab->absoluteRect.right += deltaX;
				tab->absoluteRect.top += deltaY;
				tab->absoluteRect.bottom += deltaY;

				tab->textHandler.AddPosition(deltaX, deltaY);

				tab->pTab->UpdateRegion();
			}
		}
		else
		{
			//this is the window below the tabs
			MashGUIRect tabWindowDest(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, m_absoluteTabRegion.bottom - m_absoluteTabRegion.top), MashGUIUnit(1.0f, 0.0f), MashGUIUnit(1.0f, 0.0f));

			//update button rects
			f32 fCurrentWidth = 0.0f;
			const uint32 iItemCount = m_tabs.Size();
			for(uint32 i = 0; i < iItemCount; ++i)
			{
				//build the tabs.
				//String width plus some extra space
				const f32 fTextWidth = activeFont->GetStringLength(m_tabs[i].textHandler.GetString().GetCString()) + tabWidthBuffer/*activeFont->GetCursorAdvanceSize('A')*/;
				m_tabs[i].absoluteRect.left = m_absoluteRegion.left + fCurrentWidth;
				m_tabs[i].absoluteRect.right = m_tabs[i].absoluteRect.left + fTextWidth;
				m_tabs[i].absoluteRect.top = m_absoluteRegion.top;
				m_tabs[i].absoluteRect.bottom = m_tabs[i].absoluteRect.top + activeFont->GetMaxCharacterHeight() + tabHeightBuffer;

				m_tabs[i].absoluteClippedRect = m_tabs[i].absoluteRect;
				if (m_tabs[i].absoluteClippedRect.ClipGUI(m_absoluteClippedRegion) == aCULL_CULLED)
					m_tabs[i].bCulled = true;
				else
					m_tabs[i].bCulled = false;

				fCurrentWidth += fTextWidth;

				//resize the component and its children
				m_tabs[i].pTab->SetDestinationRegion(tabWindowDest);
				m_tabs[i].textHandler.SetRegion(m_tabs[i].absoluteRect, m_tabs[i].absoluteClippedRect);
			}
		}
		
		m_bForceTabUpdate = false;
	}

	void CMashGUITabControl::OnResize(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		if (m_bForceTabUpdate)
			positionChangeOnly = false;

		UpdateTabControl(positionChangeOnly, deltaX, deltaY);
	}

	MashGUIComponent* CMashGUITabControl::GetClosestIntersectingChild(const mash::MashVector2 &vScreenPos, bool bTestAllChildren)
	{
		if (bTestAllChildren || (GetRenderEnabled() && GetCanHaveFocus()))
		{
			const uint32 iItemCount = m_tabs.Size();
			for(uint32 i = 0; i < iItemCount; ++i)
			{
				MashGUIComponent *pResult = m_tabs[i].pTab->GetClosestIntersectingChild(vScreenPos, bTestAllChildren);

				if (pResult)
					return pResult;
			}

			MashGUIComponent *pResult = MashGUIComponent::GetClosestIntersectingChild(vScreenPos, bTestAllChildren);

			if (pResult)
				return pResult;
		}

		return 0;
	}

	int32 CMashGUITabControl::AddTab(const MashStringc &text)
	{
		const int32 iTabID = m_tabIDCount++;

		MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();

		//make the tab window start below the tabs
		MashGUIRect tabWindowDest = m_destinationRegion;
		tabWindowDest.top.offset += activeStyle->GetFont()->GetMaxCharacterHeight();

		sTab newTab;
		newTab.pTab = m_GUIManager->_GetGUIFactory()->CreateView(tabWindowDest, this);
		newTab.id = iTabID;
		newTab.textHandler.SetString(text);
		newTab.textHandler.SetFormat(activeStyle->GetFont(), MashGUIFont::aCENTER, false);
		m_tabs.PushBack(newTab);

		if (m_iActiveTab < 0)
			SetActiveTabByID(iTabID);

		m_bForceTabUpdate = true;

		return iTabID;
	}

	CMashGUITabControl::sTab* CMashGUITabControl::GetTab(int32 id)const
	{
		const uint32 tabCount = m_tabs.Size();
		for(uint32 i = 0; i < tabCount; ++i)
		{
			if (m_tabs[i].id == id)
				return (sTab*)&m_tabs[i];
		}

		return 0;
	}

	void CMashGUITabControl::SetVerticalScrollState(int32 tab, bool state)
	{
		sTab *activeTab = GetTab(tab);
		if (activeTab)
			activeTab->pTab->SetVerticalScrollState(state);
	}

	void CMashGUITabControl::SetHorizontalScrollState(int32 tab, bool state)
	{
		sTab *activeTab = GetTab(tab);
		if (activeTab)
			activeTab->pTab->SetHorizontalScrollState(state);
	}

	eMASH_STATUS CMashGUITabControl::AddChildToTab(int32 tab, MashGUIComponent *pChild)
	{
		sTab *activeTab = GetTab(tab);
		if (activeTab)
		{
			activeTab->pTab->AddChild(pChild);
			return aMASH_OK;
		}

		return aMASH_FAILED;
	}

	MashGUIView* CMashGUITabControl::GetView()
	{
		sTab *activeTab = GetTab(m_iActiveTab);
		if (activeTab)
			return activeTab->pTab;

		return 0;
	}

	void CMashGUITabControl::SetText(int32 id, const MashStringc &text)
	{
		sTab *activeTab = GetTab(id);
		if (activeTab)
		{
			activeTab->textHandler.SetString(text);
			//an update is needed cause the tab will need to be resized
			m_bForceTabUpdate = true;
		}
	}

	const MashStringc& CMashGUITabControl::GetTabText(int32 id)const
	{
		const sTab *activeTab = GetTab(id);
		if (activeTab)
		{
			return activeTab->textHandler.GetString();
		}

		return g_staticDefaultString;
	}

	MashGUIView* CMashGUITabControl::GetTabView(int32 id)const
	{
		const sTab *activeTab = GetTab(id);
		if (activeTab)
		{
			return activeTab->pTab;
		}

		return 0;
	}

	void CMashGUITabControl::RemoveTab(int32 id)
	{
		const uint32 tabCount = m_tabs.Size();
		//dont remove the last element
		if (tabCount == 1)
			return;

		for(uint32 i = 0; i < tabCount; ++i)
		{
			if (m_tabs[i].id == id)
			{
				if (m_iActiveTab == m_tabs[i].id)
				{
					m_iActiveTab = -1;
				}
				if (m_iHoverTab == m_tabs[i].id)
					m_iHoverTab = -1;

				if (m_tabs[i].pTab)
					m_tabs[i].pTab->Destroy();

				m_tabs.Erase(m_tabs.Begin() + i);
				m_bForceTabUpdate = true;

				if (m_iActiveTab == -1)
					m_iActiveTab = m_tabs[0].id;

				break;
			}
		}
	}

	MashGUIComponent* CMashGUITabControl::GetElementByName(const MashStringc &name, bool searchChildren)
	{
		MashGUIComponent *element = MashGUIComponent::GetElementByName(name, searchChildren);
		if (element)
			return element;

		const uint32 itemCount = m_tabs.Size();
		for(uint32 i = 0; i < itemCount; ++i)
		{
			element = m_tabs[i].pTab->GetElementByName(name, searchChildren);
			if (element)
				return element;
		}

		return 0;
	}

	void CMashGUITabControl::OnEvent(const sInputEvent &eventData)
	{
		if (GetEventsEnabled())
		{
			if ((eventData.eventType == sInputEvent::aEVENTTYPE_MOUSE))
			{
				mash::MashVector2 vMousePos = m_inputManager->GetCursorPosition();

				switch(eventData.action)
				{
				case aMOUSEEVENT_AXISX:
				case aMOUSEEVENT_AXISY:
					{
						m_iHoverTab = -1;

						const uint32 iItemCount = m_tabs.Size();
						for(uint32 i = 0; i < iItemCount; ++i)
						{
							if (m_tabs[i].absoluteClippedRect.IntersectsGUI(vMousePos))
							{
								m_iHoverTab = m_tabs[i].id;
								break;
							}
						}

						break;
					}
				case aMOUSEEVENT_B1:
					{
						if (eventData.isPressed == 0)
						{
							const uint32 iItemCount = m_tabs.Size();
							for(uint32 i = 0; i < iItemCount; ++i)
							{
								if ((m_tabs[i].id == m_iHoverTab) && m_tabs[i].absoluteClippedRect.IntersectsGUI(vMousePos))
								{
									if (m_iActiveTab != m_tabs[i].id)
									{
										SetActiveTabByID(m_tabs[i].id);

										sGUIEvent newGUIMsg;
										
										newGUIMsg.GUIEvent = aGUIEVENT_TC_TAB_CHANGE;
										newGUIMsg.component = this;
										ImmediateBroadcast(newGUIMsg);
									}

									break;
								}
							}
						}

						break;
					}
				};
			}
		}
	}

	void CMashGUITabControl::Draw()
	{
		if (m_bForceTabUpdate)
			UpdateTabControl(false);

		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			MashGUIComponent::Draw();
			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();

			m_GUIManager->DrawSprite(m_absoluteTabRegion, m_absoluteClippedRegion, activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_BACKGROUND), m_overrideTransparency);

			MashGUISkin *activeTabSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_TAB_ACTIVE);
			MashGUISkin *inactiveTabSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_TAB_INACTIVE);
			MashGUISkin *hoverTabSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_TAB_HOVER);
			//seperate tab rendering to support batching
			const uint32 iItemCount = m_tabs.Size();
			for(uint32 i = 0; i < iItemCount; ++i)
			{
				if (!m_tabs[i].bCulled)
				{
					MashGUISkin *currentSkin = inactiveTabSkin;
					if (m_tabs[i].id == m_iActiveTab)
						currentSkin = activeTabSkin;
					else if (m_tabs[i].id == m_iHoverTab)
						currentSkin = hoverTabSkin;

					//draw the tab
					m_GUIManager->DrawSprite(m_tabs[i].absoluteRect, m_absoluteClippedRegion, currentSkin, m_overrideTransparency);
				}
			}

			for(uint32 i = 0; i < iItemCount; ++i)
			{
				if (!m_tabs[i].bCulled)
				{
					//draw the tab text
					//if (!m_tabs[i].sText.empty())
					{
						MashGUISkin *currentSkin = inactiveTabSkin;
						if (m_tabs[i].id == m_iActiveTab)
							currentSkin = activeTabSkin;
						else if (m_tabs[i].id == m_iHoverTab)
							currentSkin = hoverTabSkin;

						m_tabs[i].textHandler.Draw(m_GUIManager, currentSkin->fontColour, m_overrideTransparency);
					}
				}
			}

			//draw the active tab window and its children
			if (m_iActiveTab != -1)
			{
				sTab *activeTab = GetTab(m_iActiveTab);
				if (activeTab && !activeTab->bCulled)
					activeTab->pTab->Draw();
			}
		}
	}

}