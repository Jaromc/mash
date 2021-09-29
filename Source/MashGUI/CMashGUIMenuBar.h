//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _CMASH_GUI_MENU_BAR_H_
#define _CMASH_GUI_MENU_BAR_H_

#include "MashGUIMenuBar.h"
#include "CMashTextHelper.h"

namespace mash
{
	class CMashGUIMenuBar : public MashGUIMenuBar
	{
	private:
		struct sItem
		{
			int32 id;
			int32 userValue;
			CMashTextHelper textHandler;
			bool bHasFocus;
			mash::MashRectangle2 absoluteRect;
			MashGUIPopupMenu *pSubMenu;

			sItem():bHasFocus(false),pSubMenu(0), id(-1),
				absoluteRect(0.0f, 0.0f, 0.0f, 0.0f), userValue(0){}
		};

		int32 m_styleElement;

		f32 m_fMenuBarHeight;
		bool m_bForceItemUpdate;

		uint32 m_itemCounter;

		int32 m_focusedItemId;
		MashGUIPopupMenu *m_lastSelectedPopup;

		MashArray<sItem> m_itemList;
		void UpdateHoverElement(const mash::MashVector2 &vMousePos);

		void OnLostFocus();
		void OnMouseExit(const mash::MashVector2 &vScreenPos);
		void OnMouseEnter(const mash::MashVector2 &vScreenPos);
		void OnSubMenuLostFocus(const sGUIEvent &eventData);
		void OnSubMenuSelection(const sGUIEvent &eventData);
		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
		sItem* GetItem(int32 id);
		void UpdateItems(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
	public:
		CMashGUIMenuBar(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			MashGUIRect &destination,
			f32 menuBarHeight,
			int32 styleElement);

		virtual ~CMashGUIMenuBar();

		void SetOverrideTransparency(bool state, uint8 alpha, bool affectFont = true, f32 alphaMaskThreshold = 0.0f);
		int32 AddItem(const MashStringc &text, MashGUIPopupMenu *pSubMenu, int32 userValue = 0);

		MashGUIPopupMenu* GetSelectedSubMenu()const;
		MashGUIPopupMenu* GetItemSubMenu(int32 id);
		void SetItemText(int32 id, const MashStringc &text);
		void SetItemValue(int32 id, int32 val);
		void GetItems(MashArray<sMenuBarItem> &out)const;
		uint32 GetItemCount()const;
		void RemoveItem(int32 id);

		int32 GetSelectedItemId()const;
		int32 GetItemUserValue(int32 id);

		void OnStyleChange(MashGUIStyle *style);

		void OnEvent(const sInputEvent &eventData);
		void Draw();

		eMASH_GUI_TYPE GetGUIType()const;
		
	};

	inline int32 CMashGUIMenuBar::GetSelectedItemId()const
	{
		return m_focusedItemId;
	}

	inline MashGUIPopupMenu* CMashGUIMenuBar::GetSelectedSubMenu()const
	{
		return m_lastSelectedPopup;
	}

	inline uint32 CMashGUIMenuBar::GetItemCount()const
	{
		return m_itemList.Size();
	}

	inline eMASH_GUI_TYPE CMashGUIMenuBar::GetGUIType()const
	{
		return aGUI_MENUBAR;
	}
}

#endif