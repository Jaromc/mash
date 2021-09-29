//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_TAB_CONTROL_H_
#define _MASH_GUI_TAB_CONTROL_H_

#include "MashGUIComponent.h"

namespace mash
{
	/*!
		Tab controls can hold a number of tabs.
		Each tab contain a view and can therefore hold child objects.
	*/
	class MashGUITabControl : public MashGUIComponent
	{
	public:
		MashGUITabControl(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, pInputManager, pParent, destination){}
		virtual ~MashGUITabControl(){}

		//! Adds a new tab.
		/*!
			\param text Tab text.
			\return Tab id.
		*/
		virtual int32 AddTab(const MashStringc &text) = 0;

		//! Add a child to a tabs view.
		/*!
			\param tab Tab id.
			\param child Child to add.
			\return Ok on success, failed otherwise.
		*/
		virtual eMASH_STATUS AddChildToTab(int32 tab, MashGUIComponent *child) = 0;

		//! Removes a tab and its contents.
		/*!
			\param id Id of the tab to remove.
		*/
		virtual void RemoveTab(int32 id) = 0;

		//! Sets the text of a tab.
		/*!
			\param id Tab id.
			\param text New text for the tab.
		*/
		virtual void SetText(int32 id, const MashStringc &text) = 0;

		//! Returns the if of the selected tab.
		/*!
			\return Id of the selected tab. -1 if nothing is selected.
		*/
		virtual int32 GetSelectedTabID()const = 0;

		//! Gets the text of a tab.
		/*!
			\param id Tab id.
			\return Tab text.
		*/
		virtual const MashStringc& GetTabText(int32 id)const = 0;

		//! Gets the view for a tab.
		/*!
			\param id Tab id.
			\return Tabs view or NULL if it doesn't exist.
		*/
		virtual MashGUIView* GetTabView(int32 id)const = 0;
	};
}

#endif