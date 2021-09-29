//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_POPUP_MENU_H_
#define _MASH_GUI_POPUP_MENU_H_

#include "MashGUIComponent.h"

namespace mash
{
	class MashGUIPopupMenu : public MashGUIComponent
	{
	public:
		struct sPopupItem
		{
			MashStringc text;
			int32 returnValue;
			int32 id;
			bool hasSubMenu;
		};
	public:
		MashGUIPopupMenu(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, pInputManager, pParent, destination){}

		virtual ~MashGUIPopupMenu(){}

		//! Adds an item to this popup.
		/*!
			\param text Item text.
			\param returnValue A user value used for identification.
			\param subMenu A popup menu that opens from this item.
			\return Item id.
		*/
		virtual int32 AddItem(const MashStringc &text, int32 returnValue = 0, MashGUIPopupMenu *subMenu = 0) = 0;

		//! Enables this popup.
		/*!
			\param topLeft The popups new topleft location.
		*/
		virtual void Activate(const mash::MashVector2 &topLeft) = 0;
		
		//! Deactivates this popup.
		virtual void Deactivate() = 0;

		//! Returns the selected item id.
		/*!
			\return Selected item id.
		*/
		virtual int32 GetSelectedItemId()const = 0;

		//! Returns the user value of an item.
		/*!
			\param id Item id.
			\return Item user value.
		*/
		virtual int32 GetItemUserValue(int32 id) = 0;

		//! Removes an item from this popup.
		/*!
			\param id Item id.
		*/
		virtual void RemoveItem(int32 id) = 0;

		//! Detaches this popup from its owner popup.
		virtual void DetachFromPopupOwner() = 0;

		//! Is this popop open.
		/*!
			\return True if this item is active.
		*/
		virtual bool IsActive()const = 0;

		//! Sets an items text.
		/*!
			\param id Item id.
			\param text New item text.
		*/
		virtual void SetItemText(int32 id, const MashStringc &text) = 0;

		//! Sets an items user value.
		/*!
			\param id Item id.
			\param value Items new user value.
		*/
		virtual void SetItemUserValue(int32 id, int32 value) = 0;

		//! Arranges all the items into an array.
		/*!
			This is mainly used for saving.
			\param out Array that will hold all items in this popup.
		*/
		virtual void GetItems(MashArray<sPopupItem> &out)const = 0;

		//! Returns an items submenu.
		/*!
			\param id Item id.
			\return An items submenu. NULL if no submenu exists.
		*/
		virtual MashGUIPopupMenu* GetItemSubMenu(int32 id) = 0;

		//! Returns the popup owner.
		/*!
			If this popup is a submenu then its owner will be returned.
			\return Popup owner.
		*/
		virtual MashGUIPopupMenu* GetPopupOwner()const = 0;
		
		//! Returns the Id of this popups owner.
		/*!
			If this is a submenu, then the returned value is the id of the popup owner.
			\return Popup owner id.
		*/
		virtual int32 GetAttachmentIndex()const = 0;

		//! Returns the number of items in this popup.
		/*!
			An items id is not necessarily 0 - (GetItemCount() - 1). So this value
			should not be used to determine item ids.

			\return Item count.
		*/
		virtual uint32 GetItemCount()const = 0;

		//! Detaches and drops a submenu.
		/*!
			\param attachmentIndex Submenu to detach.
		*/
		virtual void DetachSubMenu(int32 attachmentIndex) = 0;

		//! Attaches a submenu.
		/*!
			\param id Item id to attach the submenu.
			\param subMenu Submenu to attach.
		*/
		virtual void AttachSubMenu(int32 id, MashGUIPopupMenu *subMenu) = 0;

		//! Called when detached as a submenu.
		virtual void OnDetach() = 0;

		//! Called when attached as a submenu.
		/*!
			\param owner New popup owner.
			\param attachmentIndex Popup owners attachment id.
		*/
		virtual void OnAttach(MashGUIPopupMenu *owner, int32 attachmentIndex) = 0;
	};
}

#endif