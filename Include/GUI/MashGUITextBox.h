//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_TEXT_BOX_H_
#define _MASH_GUI_TEXT_BOX_H_

#include "MashGUIComponent.h"
#include "MashGUITypes.h"

namespace mash
{
	class MashGUIFont;

	/*!
		Textboxes allow for single lined text input. The text will be
		aligned to the top left of the box.
	*/
	class MashGUITextBox : public MashGUIComponent
	{
	public:
		MashGUITextBox(MashGUIManager *GUIManager,
			MashInputManager *inputManager,
			MashGUIComponent *parent,
			const MashGUIRect &destination):MashGUIComponent(GUIManager, inputManager, parent, destination){}

		virtual ~MashGUITextBox(){}

        //! Enables or disabled the inrement and decrement buttons.
        /*!
            \param enable Enables the side buttons.
        */
		virtual void SetNumberButtonState(bool enable) = 0;
        
        //! Returns true if the increment and decrement buttons are enabled.
		virtual bool GetNumberButtonState()const = 0;

        //! Sets the precision of floats.
        /*!
            Only valid if this textbox is of f32 type.
         
            \param precision Decimal point precision.
        */
		virtual void SetFloatPrecision(uint32 precision) = 0;
        
        //! Gets the f32 precision.
		virtual uint32 GetFloatPrecision()const = 0;

        //! Gets the min number.
        /*!
            This is only valid if this textbox is a number type.
            
            \return Min number.
        */
		virtual f32 GetMinNumber()const = 0;
        
        //! Gets the max number.
        /*!
            This is only valid if this textbox is a number type.
         
            \return Max number.
         */
		virtual f32 GetMaxNumber()const = 0;
        
        //! Sets the min and max number that can be displayed.
        /*!
            Only valid for number type textboxes.
            
            Numbers entered beyond this range will be wrapped.
         
            \param min Min number.
            \param max Max number.
            
        */
		virtual void SetNumberMinMax(f32 min, f32 max) = 0;

		//! Sets the text format.
        /*!
            This decides how text is formated and clamped.
            Using number formats allows you to enable increment
            side buttons. Strings cannot be set for number formats,
            instead use SetTextInt() or similar.
         
            \param format Text format.
        */
		virtual void SetTextFormat(eGUI_TEXT_FORMAT format) = 0;
        
        //! Gets the text format.
		virtual eGUI_TEXT_FORMAT GetTextFormat()const = 0;
        
        //! Adds a string to the current text.
        /*!
            \param text Text to append.
        */
		virtual void AddString(const MashStringc &text) = 0;
        
        //! Adds a character to the current text.
        /*!
            \param c Character to append.
        */
		virtual void AddCharacter(int8 c) = 0;
        
        //! Sets the colour all text will be rendered in.
        /*!
            \param colour Text colour.
        */
		virtual void SetTextColour(const sMashColour &colour) = 0;
        
        //! Sets the current text.
        /*!
            This will overwrite any text previously set.
         
            \param text Text to set.
        */
		virtual void SetText(const MashStringc &text) = 0;
        
        //! Sets the current text as an int32.
        /*!
            This will overwrite any text previously set.
         
            \param num Number to set.
        */
		virtual void SetTextInt(int32 num) = 0;
        
        //! Sets the current text as a f32.
        /*!
            This will overwrite any text previously set.
         
            \param num Number to set.
         */
		virtual void SetTextFloat(f32 num) = 0;
        
        //! Removes characters from the end of the list.
        /*!
            \param count Number of characters to remove.
        */
		virtual void RemoveCharacters(uint32 count) = 0;
        
        //! Gets the current text.
		virtual const MashStringc& GetText()const = 0;
        
        //! Inserts a character into the current text.
        /*!
            \param location Character will be inserted before the current int8 at this location.
            \param newChar Character to insert.
        */
		virtual void InsertCharacter(uint32 location, int8 newChar) = 0;
        
        //! Inserts a string into the current text.
        /*!
            \param location String will be inserted before the current int8 at this location.
            \param newString String to insert.
        */
		virtual void InsertString(uint32 location, const int8 *newString) = 0;

        //! Gets the text converted into an int32.
		virtual int32 GetTextAsInt() = 0;
        
        //! Gets the text converted into a f32.
		virtual f32 GetTextAsFloat() = 0;

        //! Enables accept when enter/return is pressed.
        /*!
            The text will be validated and a message set to any listeners.
            
            \param value Enable or disable accept on return.
        */
		virtual void SetAcceptReturn(bool value) = 0;
        
        //! Gets the accept on return state.
		virtual bool GetAcceptReturn()const = 0;
	};
}


#endif