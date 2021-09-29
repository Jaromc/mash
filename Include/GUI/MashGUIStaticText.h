//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_STATIC_TEXT_H_
#define _MASH_GUI_STATIC_TEXT_H_

#include "MashGUIComponent.h"
#include "MashGUIFont.h"

namespace mash
{
	class MashGUIStaticText : public MashGUIComponent
	{
	public:
		MashGUIStaticText(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, pInputManager, pParent, destination){}

		virtual ~MashGUIStaticText(){}

		//! Sets the word wrap state.
		/*!
			If true then the text will move to a new line when a line reaches
			this objects bounds.

			\param wrap Word wrap state.
		*/
		virtual void SetWordWrap(bool wrap) = 0;

		//! Returns the word wrap state.
		/*!
			\return Word wrap state.
		*/
		virtual bool GetWordWrap()const = 0;

		//! Adds text to the current string.
		/*!
			\param text text to add.
		*/
		virtual void AddText(const MashStringc &text) = 0;

		//! Sets the current text.
		/*!
			This will override any text previously set.
			\param text New text.
		*/
		virtual void SetText(const MashStringc &text) = 0;

		//! Sets the current text in int32 form.
		/*!
			This will override any text previously set.
			This number will be converted to a string.

			\param val Number to display.
		*/
		virtual void SetTextInt(int32 val) = 0;

		//! Sets the current text in f32 form.
		/*!
			This will override any text previously set.
			This number will be converted to a string.

			\param val Number to display.
			\param precision Maximum number of decimal places to render.
		*/
		virtual void SetTextFloat(f32 val, uint32 precision = 5) = 0;

		//! Returns the current text.
		/*!
			\return Current text.
		*/
		virtual const MashStringc& GetText()const = 0;

		//! Disable background rendering
		/*!
			This will make the text box transparent and only render the text.

			\param value Render background state.
		*/
		virtual void SetRenderBackground(bool value) = 0;

		//! Resizes the destination region to fit the text.
		/*!
			Each time the text is updated, the destination region will also update
			to a value that makes all the text visible.

			\param enable Enable or disables auto resize.
		*/
		virtual void AutoResizeToFitText(bool enable) = 0;

		//! Returns if auto resize is enabled.
		/*!
			\return True if auto resize is enabled, false otherwise.
		*/
		virtual bool GetAutoResizeToFitText()const = 0;

		//! Sets the text alignment.
		/*!
			Sets how the text should be align in the text boxes region.

			\param alignment text alignment.
		*/
		virtual void SetTextAlignment(MashGUIFont::eFONT_ALIGNMENT alignment) = 0;

		//! Gets the text alignment.
		/*!
			\return Text alignment.
		*/
		virtual MashGUIFont::eFONT_ALIGNMENT GetTextAlignment()const = 0;
	};
}

#endif