//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_TREE_H_
#define _MASH_GUI_TREE_H_

#include "MashGUIComponent.h"

namespace mash
{
	class MashGUITree : public MashGUIComponent
	{
	public:
		struct sItemData
		{
			MashStringc text;
			int32 id;

			MashArray<sItemData> children;
		};
	public:
		MashGUITree(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, pInputManager, pParent, destination){}
		
		virtual ~MashGUITree(){}

		//! Add a new item.
		/*!
			\param text Item text.
			\param parent Id of the parent this item should be parented to. Use -1 to attach to the root.
			\param userValue A user defined value.
			\return The new items id.
		*/
		virtual int32 AddItem(const MashStringc &text, int32 parent = -1, int32 userValue = 0) = 0;

		//! Removes an item.
		/*!
			\param id Item id.
		*/
		virtual void RemoveItem(int32 id) = 0;

		//! Removes all items from the tree.
		virtual void RemoveAllItems() = 0;

		//! Returns the selected items id.
		/*!
			\return Selected items id. -1 if no item is selected.
		*/
		virtual int32 GetSelectedItemID()const = 0;

		//! Sets an items text.
		/*!
			\param id Item id.
			\param text Items new text.
		*/
		virtual void SetItemText(int32 id, const MashStringc &text) = 0;

		//! Sets the active item.
		/*!
			Manually sets the active item rather than by mouse input.
			
			\param id Item id.
		*/
		virtual void SetActiveItem(int32 id) = 0;

		//! Sets the active item by user value.
		/*!
			Manually sets the active item rather than by mouse input.
			If multiple items have the same user value then the first
			item found will be set as the active item.
			
			\param userValue An items user value.
		*/
		virtual void SetActiveItemByUserId(int32 userValue) = 0;
		
		//! Add all the items into an array.
		/*!
			Used for saving.
			\param out Array to fill with tree data.
		*/
		virtual void GetItems(MashArray<sItemData> &out) = 0;

		//! Sets the user value for an item.
		/*!
			\param id Item id.
			\param userValue Items new user value.
		*/
		virtual void SetItemUserId(int32 id, int32 userValue) = 0;

		//! Returns an items text.
		/*!
			\param id Item id.
			\return Items text.
		*/
		virtual const MashStringc& GetItemText(int32 id) = 0;

		//! Returns the user value of an item.
		/*!
			\param id Item id.
			\return Items user value.
		*/
		virtual int32 GetItemUserId(int32 id) = 0;

		//! Returns the id of an item by user value.
		/*!
			This will return the first item found with the
			given user id.

			\param userValue Items user value.
			\return Item id. -1 if the item was not found.
		*/
		virtual int32 GetItemIdByUserValue(int32 userValue) = 0;


	};
}

#endif