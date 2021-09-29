//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_LOAD_CALLBACK_H_
#define _MASH_GUI_LOAD_CALLBACK_H_

#include "MashGUIComponent.h"

namespace mash
{
	/*!
		This can be set in the GUI manager and is called when each item is created.
		Each item can then have values set for special applications (eg, gui editor.).
	*/
	class MashGUILoadCallback : public MashReferenceCounter
	{
	public:
		MashGUILoadCallback(){}
		virtual ~MashGUILoadCallback(){}

		//! Callback when an item is created.
		/*!
			\param component New component.
		*/
		virtual void OnCreateComponent(MashGUIComponent *component) = 0;
	};
}

#endif