//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_MENU_BAR_H_
#define _MASH_GUI_MENU_BAR_H_

#include "MashGUIComponent.h"

namespace mash
{
	class MashGUIPopupMenu;

	class MashGUIMenuBar : public MashGUIComponent
	{
	public:
		struct sMenuBarItem
		{
			MashStringc text;
			int32 id;
			int32 userValue;
		};
	public:
		MashGUIMenuBar(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, pInputManager, pParent, destination){}

		virtual ~MashGUIMenuBar(){}

		//! Adds an item to the menu bar.
		/*!
			\param text Item text.
			\param subMenu The popup that will be disaplyed when this item is selected.
			\param userValue Item value.
			\return Item id.
		*/
		virtual int32 AddItem(const MashStringc &text, MashGUIPopupMenu *subMenu, int32 userValue = 0) = 0;

		//! Returns the submenu for an item on the menu bar.
		/*!
			\param id Item id.
			\return Item popup menu. NULL if the item id doesn't exist.
		*/
		virtual MashGUIPopupMenu* GetItemSubMenu(int32 id) = 0;

		//! Sets the text for an item.
		/*!
			\param id Item id.
			\param text Item text.
		*/
		virtual void SetItemText(int32 id, const MashStringc &text) = 0;

		//! Sets an items user value.
		/*!
			\param id Item id.
			\param val User value.
		*/
		virtual void SetItemValue(int32 id, int32 val) = 0;

		//! Returns all the items data in an array.
		/*!
			Used for saving.
			\param out Item list.
		*/
		virtual void GetItems(MashArray<sMenuBarItem> &out)const = 0;

		//! Returns the number of items in this menu bar.
		/*!
			An items id is not necessarily 0 - (GetItemCount() - 1). So this value
			should not be used to determine item ids.

			\return Item count.
		*/
		virtual uint32 GetItemCount()const = 0;

		//! Removes an item.
		/*!
			\param id Item id.
		*/
		virtual void RemoveItem(int32 id) = 0;

		//! Gets the submenu of a selected item.
		/*!
			This is only valid after aGUIEVENT_MENUBAR_SELECTION.
			\return Selected items popup menu.
		*/
		virtual MashGUIPopupMenu* GetSelectedSubMenu()const = 0;

		//! Gets the id of the selected item.
		/*!
			\return Selected item id. -1 if nothing is selected.
		*/
		virtual int32 GetSelectedItemId()const = 0;

		//! Gets the user value from an item.
		/*!
			\param item id.
			\return Item user value.
		*/
		virtual int32 GetItemUserValue(int32 id) = 0;
	};
}

#endif