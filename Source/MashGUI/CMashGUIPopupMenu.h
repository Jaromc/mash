//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _CMASH_GUI_POPUP_MENU_H_
#define _CMASH_GUI_POPUP_MENU_H_

#include "MashGUIPopupMenu.h"
#include "CMashTextHelper.h"

namespace mash
{
	class CMashGUIPopupMenu : public MashGUIPopupMenu
	{
	private:
		struct sItem
		{
			int32 id;
			CMashTextHelper textHandler;
			bool bHasFocus;
			bool bChecked;//TODO : Remove
			int32 returnValue;
			mash::MashRectangle2 absoluteRect;
			MashGUIPopupMenu *pSubMenu;

			sItem():/*pTextVertices(0), iTextVerticesCount(0), iReservedVertexBufferSize(0),
				text(""),*/ bHasFocus(false), bChecked(false),pSubMenu(0),
				absoluteRect(0.0f, 0.0f, 0.0f, 0.0f), returnValue(0), id(-1){}

			~sItem(){}
		};

		/*
			note, this is different from a components parent as the popup
			owner does not define clipping regions.
			A valid pointer states this is a sub menu.
		*/
		MashGUIPopupMenu *m_pPopupOwner;
		int32 m_iAttachmentIndex;

		int32 m_styleElement;
		f32 m_itemHorizontalBuffer;

		int32 m_focusedItem;
		//we store the last focused item so it can be accessed after focus is lost
		int32 m_lastFocusedItem;

		MashArray<sItem*> m_itemList;
		f32 m_fCheckBoxSize;
		f32 m_fMinButtonSize;
		mash::MashVector2 m_vTopLeft;
		bool m_bForceUpdate;

		uint32 m_itemIdCounter;

		void OnStyleChange(MashGUIStyle *style);
		void OnDetach();
		void OnAttach(MashGUIPopupMenu *pOwner, int32 iAttachmentIndex);
		sItem* GetItem(int32 id);
		void UpdatePopup(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
	public:
		CMashGUIPopupMenu(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			int32 styleElement);

		virtual ~CMashGUIPopupMenu();

		int32 AddItem(const MashStringc &text, int32 returnValue = 0, MashGUIPopupMenu *pSubMenu = 0);
		void RemoveItem(int32 id);

		void DetachFromPopupOwner();
		void Activate(const mash::MashVector2 &vTopLeft);
		void Deactivate();

		bool IsActive()const;

		void _SetHasFocus(bool bHasFocus);
		void Draw();
		void OnEvent(const sInputEvent &eventData);

		bool GetIsSubMenu()const;
		
		int32 GetAttachmentIndex()const;
		void DetachSubMenu(int32 iAttachmentIndex);
		void AttachSubMenu(int32 id, MashGUIPopupMenu *pSubMenu);

		int32 GetSelectedItemId()const;
		int32 GetItemUserValue(int32 id);
		MashGUIPopupMenu* GetItemSubMenu(int32 id);
		uint32 GetItemCount()const;

		MashGUIPopupMenu* GetPopupOwner()const;

		void SetItemText(int32 id, const MashStringc &text);
		void SetItemUserValue(int32 id, int32 value);

		void GetItems(MashArray<sPopupItem> &out)const;

		eMASH_GUI_TYPE GetGUIType()const;
	};

	inline int32 CMashGUIPopupMenu::GetAttachmentIndex()const
	{
		return m_iAttachmentIndex;
	}

	inline uint32 CMashGUIPopupMenu::GetItemCount()const
	{
		return m_itemList.Size();
	}

	inline MashGUIPopupMenu* CMashGUIPopupMenu::GetPopupOwner()const
	{
		return m_pPopupOwner;
	}

	inline int32 CMashGUIPopupMenu::GetSelectedItemId()const
	{
		return m_lastFocusedItem;
	}

	inline eMASH_GUI_TYPE CMashGUIPopupMenu::GetGUIType()const
	{
		return aGUI_POPUP_MENU;
	}

	inline bool CMashGUIPopupMenu::GetIsSubMenu()const
	{
		if (m_pPopupOwner)
			return true;

		return false;
	}
}

#endif