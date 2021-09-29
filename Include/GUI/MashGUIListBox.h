//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_LIST_BOX_H_
#define _MASH_GUI_LIST_BOX_H_

#include "MashGUIComponent.h"

namespace mash
{
	class MashGUIListBox : public MashGUIComponent
	{
	public:
		MashGUIListBox(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, pInputManager, pParent, destination){}

		virtual ~MashGUIListBox(){}

		//! Sets an icon for an item.
		/*!
			\param id Id of the item.
			\param icon Texture file.
			\param source Area of the texture to render in pixels. Eg, (0, 0, textureWidth, textureHeight).
		*/
		virtual void SetItemIcon(int32 id, mash::MashTexture *icon, const mash::MashRectangle2 *source = 0) = 0;

		//! Sets the height for all items.
		/*!
			param height Height of all items.
		*/
		virtual void SetItemHeight(uint32 height) = 0;

		//! Sets the source region of a single item in the listbox.
		/*!
			\param id Id of the item.
			\param source Area of the texture to render in pixels. Eg, (0, 0, textureWidth, textureHeight).
		*/
		virtual void SetItemIconSourceRegion(int32 id, const mash::MashRectangle2 &source) = 0;

		//! Sets the position an icom will be rendered for each item.
		/*!
			\param destination Icon region for each item.
		*/
		virtual void SetItemIconRegion(const MashGUIRect &destination) = 0;

		//! Sets the position item text will be rendered for each item.
		/*!
			\param destination Text region for each item.
		*/
		virtual void SetItemTextRegion(const MashGUIRect &destination) = 0;

		//! Sets the text for a single item.
		/*!
			\param id Id of the item.
			\param text Text rendered for an item.
		*/
		virtual void SetItemText(int32 id, const MashStringc &text) = 0;

		//! Sets the user value for a single item.
		/*!
			\param id Id of the item.
			\param val User value for this item.
		*/
		virtual void SetItemUserValue(int32 id, int32 val) = 0;

		//! Removes an item from the listbox.
		/*!
			\param id Id of the item to remove.
		*/
		virtual void RemoveItem(int32 id) = 0;

		//! Returns the number of items in the listbox
		/*!
			This value should not be used to iterate through each item.
			Item ids arn't necessarily 0 - (GetItemCount() -1).
			
			\return Number of items in this listbox.
		*/
		virtual uint32 GetItemCount()const = 0;

		//! Adds an item to the listbox.
		/*!
			\param text Item text.
			\param userValue A user defined value that can be used for identification.
			\param icon Icon to display for this icon. Icon rendering needs to be enabled using EnableIcons(). Param can be NULL. 
			\param iconSourceRegion Area of the texture to render in pixels. Eg, (0, 0, textureWidth, textureHeight). Can be null.
			\return Item id that can be used to access its data later.
		*/
		virtual int32 AddItem(const MashStringc &text, int32 userValue, mash::MashTexture *icon = 0, const mash::MashRectangle2 *iconSourceRegion = 0) = 0;

		//! Clears all items from this listbox.
		virtual void ClearAllItems() = 0;

		//! Returns the currently selected item.
		/*!
			\return Id of the selected item. -1 if no item is selected.
		*/
		virtual int32 GetSelectedItemId()const = 0;

		//! Returns the user value for an item.
		/*!
			\param id Id of the item.
			\return User value for this item.
		*/
		virtual int32 GetItemUserValue(int32 id) = 0;

		//! Returns the text that is set for an item.
		/*!
			\param id Id of the item.
			\return Item text.
		*/
		virtual const MashStringc& GetItemText(int32 id) = 0;

		//! Returns the icon set for an item.
		/*!
			\param id Id of the item.
			\return Item icon. NULL if nothing is set.
		*/
		virtual mash::MashTexture* GetItemIcon(int32 id) = 0;

		//! Enables for disables icon rendering
		/*!
			\param state Enable or disable icon rendering for all items.
			\param setIconAndTextRegionsToDefault Sets the text and icon rendering positions to their default location.
		*/
		virtual void EnableIcons(bool state, bool setIconAndTextRegionsToDefault = false) = 0;

		//! Is icon rendering enabled.
		/*!
			\return Is icon rendering enabled.
		*/
		virtual bool GetIconsEnabled()const = 0;

		//! Sets the active item.
		/*!
			This can be used to manually set the active item rather than
			by user mouse input.

			\param id Item id to make active.
			\param sendConfirmedMessage True sends a aGUIEVENT_LB_SELECTION_CONFIRMED. Else aGUIEVENT_LB_SELECTION_CHANGE is sent.
		*/
		virtual void SetActiveItem(int32 id, bool sendConfirmedMessage = false) = 0;

		//! Sets the active item by user value.
		/*!
			If multiple items have the same user value then the first item
			found with the given value will be activated.

			\param Item to select by user value.
			\param sendConfirmedMessage True sends a aGUIEVENT_LB_SELECTION_CONFIRMED. Else aGUIEVENT_LB_SELECTION_CHANGE is sent.
		*/
		virtual void SetActiveItemByUserValue(int32 val, bool sendConfirmedMessage = false) = 0;

		//! Gets the position icons are rendered on each item.
		/*!
			\return Item icon region.
		*/
		virtual const MashGUIRect& GetItemIconRegion()const = 0;

		//! Gets the position text is rendered on each item.
		/*!
			\return Item text region.
		*/
		virtual const MashGUIRect& GetItemTextRegion()const = 0;

		//! Converts a regions offset and scale into just offset coords.
		/*!
			Calculates the local position of icon rendering for each item.
			\return Item icon region.
		*/
		virtual mash::MashRectangle2 GetItemIconOffsetRegion()const = 0;

		//! Converts a regions offset and scale into just offset coords.
		/*!
			Calculates the local position of text rendering for each item.
			\return Item text region.
		*/
		virtual mash::MashRectangle2 GetItemTextOffsetRegion()const = 0;

		//! Returns the height of each item.
		/*!
			\return Item height.
		*/
		virtual uint32 GetItemHeight()const = 0;

		//! Returns the icon pixel region.
		/*!
			\param id Id of the item.
			\return Icon pixel region for an item.
		*/
		virtual const mash::MashRectangle2& GetItemIconSourceRegion(int32 id) = 0;
	};
}

#endif