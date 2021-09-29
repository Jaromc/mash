//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_CHECKBOX_H_
#define _MASH_GUI_CHECKBOX_H_

#include "MashGUIComponent.h"

namespace mash
{
	class MashGUICheckBox : public MashGUIComponent
	{
	public:
		MashGUICheckBox(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, pInputManager, pParent, destination){}
		virtual ~MashGUICheckBox(){}

		//! Is the checkbox checked.
		/*!
			\return Checked state.
		*/
		virtual bool IsChecked()const = 0;

		//! Sets the checked state.
		/*!
			For manual operation rather than user mouse input.
			\param isChecked Checked state.
		*/
		virtual void SetChecked(bool isChecked) = 0;
	};
}

#endif