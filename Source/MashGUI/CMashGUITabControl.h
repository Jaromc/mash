//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_TAB_CONTROL_H_
#define _C_MASH_GUI_TAB_CONTROL_H_

#include "MashGUITabControl.h"
#include "MashGUIView.h"
#include "MashGUIFont.h"
#include "CMashTextHelper.h"
namespace mash
{
	class CMashGUITabControl : public MashGUITabControl//, public CMashGUIEvent
	{
		struct sTab
		{
			int32 id;
			bool bCulled;
			mash::MashRectangle2 absoluteRect;
			mash::MashRectangle2 absoluteClippedRect;

			MashGUIView *pTab;
			CMashTextHelper textHandler;

			sTab():absoluteRect(), absoluteClippedRect(), bCulled(false), pTab(0), id(-1){}
		};
	private:
		f32 m_fTabWidthTotal;

		int32 m_styleElement;

		mash::MashRectangle2 m_absoluteTabRegion;
		bool m_bForceTabUpdate;

		int32 m_iActiveTab;
		int32 m_iHoverTab;
		MashArray<sTab> m_tabs;

		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
		uint32 m_tabIDCount;
		sTab* GetTab(int32 id)const;
		void UpdateTabControl(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
	public:
		CMashGUITabControl(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement);

		~CMashGUITabControl();

		int32 AddTab(const MashStringc &text);

		void SetActiveTabByID(int32 iTabID);
		void Draw();

		void OnStyleChange(MashGUIStyle *style);
		void SetOverrideTransparency(bool state, uint8 alpha, bool affectFont = true, f32 alphaMaskThreshold = 0.0f);
		MashGUIComponent* GetElementByName(const MashStringc &name, bool searchChildren = true);

		MashGUIView* GetView();
		void RemoveTab(int32 id);
		void SetText(int32 id, const MashStringc &text);
		int32 GetSelectedTabID()const;

		const MashStringc& GetTabText(int32 id)const;
		MashGUIView* GetTabView(int32 id)const;

		void SetVerticalScrollState(int32 tab, bool state);
		void SetHorizontalScrollState(int32 tab, bool state);
		eMASH_STATUS AddChildToTab(int32 iTab, MashGUIComponent *pChild);
		void OnEvent(const sInputEvent &eventData);

		eMASH_GUI_TYPE GetGUIType()const;
		MashGUIComponent* GetClosestIntersectingChild(const mash::MashVector2 &vScreenPos, bool bTestAllChildren = false);
	};

	inline int32 CMashGUITabControl::GetSelectedTabID()const
	{
		return m_iActiveTab;
	}

	inline eMASH_GUI_TYPE CMashGUITabControl::GetGUIType()const
	{
		return aGUI_TAB_CONTROL;
	}
}

#endif