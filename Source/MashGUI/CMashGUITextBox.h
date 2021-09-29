//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_TEXT_BOX_H_
#define _C_MASH_GUI_TEXT_BOX_H_

#include "MashGUITextBox.h"
#include "CMashTextHelper.h"
namespace mash
{
	class CMashGUITextBox : public MashGUITextBox
	{
	private:
		MashGUISkin m_backgroundSkin;
		
		bool m_acceptReturn;
		CMashTextHelper m_textHandler;
		eGUI_TEXT_FORMAT m_textFormat;
		MashStringc m_lastConfirmedString;

		bool m_drawCarret;
		uint32 m_lastCursorBlinkTime;
		uint32 m_accumCursorBlinkTimer;
		uint32 m_cursorBlinkTime;
		mash::MashVector2 m_vCursorPos;
		int32 m_styleElement;
		void OnLostFocus();
		void OnFocusGained();

		int8 m_repeatChar;
		eINPUT_EVENT m_repeatEventType;
		bool m_repearCharEnabled;
		int32 m_backspaceKeyTimer;
		int32 m_backspaceKeyLastTime;
		int32 m_backspaceKeyDelay;

		uint32 m_floatPrecision;
		bool m_integerButtonsEnabled;
		MashGUIButton *m_incrementButton;
		MashGUIButton *m_decrementButton;
		bool m_numberKeyHeld;
		bool m_numberIncrementKeyAction;
		int32 m_numberKeyTimer;
		int32 m_numberKeyLastTime;
		int32 m_numberKeyDelay;
		f32 m_numberMaxVal;
		f32 m_numberMinVal;

		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
		void ValidateString();

		void OnTextConfirmed(const MashStringc &text);
		void IncrementNumber();
		void DecrementNumber();
		void OnIntegerButtonIncrementChange(const sGUIEvent &eventData);
		void OnIntegerButtonDecrementChange(const sGUIEvent &eventData);
		f32 GetNumberIncrement(bool increment);
	public:
		CMashGUITextBox(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement);

		~CMashGUITextBox();

		void SetEventsEnabled(bool state);

		MashGUIComponent* GetClosestIntersectingChild(const mash::MashVector2 &vScreenPos, bool bTestAllChildren);
		void OnEvent(const sInputEvent &eventData);

		void SetFloatPrecision(uint32 precision);
		uint32 GetFloatPrecision()const;

		void SetNumberButtonState(bool enable);
		bool GetNumberButtonState()const;

		void Draw();
		eMASH_GUI_TYPE GetGUIType()const;

		int32 GetTextAsInt();
		f32 GetTextAsFloat();

		void SetTextFormat(eGUI_TEXT_FORMAT format);
		eGUI_TEXT_FORMAT GetTextFormat()const;
		void AddString(const MashStringc &text);
		void AddCharacter(int8 c);
		void SetFont(MashGUIFont *pFont);
		void SetTextColour(const sMashColour &colour);
		void SetText(const MashStringc &text);
		void SetTextInt(int32 num);
		void SetTextFloat(f32 num);
		void RemoveCharacters(uint32 iCount);
		const MashStringc& GetText()const;
		void SetCursorPos(uint32 iIndex);
		void InsertCharacter(uint32 iLocation, int8 newChar);
		void InsertString(uint32 iLocation, const int8 *sString);
		 //! Sets the word wrap state.
		/*!
            If true then the text will move to a new line when a line reaches
            this objects bounds.
         
            \param wrap Word wrap state.
         */
		void SetWordWrap(bool bEnable);
		//! Returns the word wrap state.
		/*!
            \return Word wrap state.
        */
		bool GetWordWrap()const;
		//! Sets the text alignment.
		/*!
            Sets how the text should be align in the text boxes region.
         
            \param alignment text alignment.
         */
		void SetTextAlignment(MashGUIFont::eFONT_ALIGNMENT eAlignment);

		void SetAcceptReturn(bool value);
		bool GetAcceptReturn()const;

		bool GetScaleTextToFitBounds()const;

		void OnStyleChange(MashGUIStyle *style);

		f32 GetMinNumber()const;
		f32 GetMaxNumber()const;
		void SetNumberMinMax(f32 min, f32 max);
		
	};

	inline f32 CMashGUITextBox::GetMinNumber()const
	{
		return m_numberMinVal;
	}

	inline f32 CMashGUITextBox::GetMaxNumber()const
	{
		return m_numberMaxVal;
	}

	inline bool CMashGUITextBox::GetNumberButtonState()const
	{
		return m_integerButtonsEnabled;
	}

	inline uint32 CMashGUITextBox::GetFloatPrecision()const
	{
		return m_floatPrecision;
	}

	inline eGUI_TEXT_FORMAT CMashGUITextBox::GetTextFormat()const
	{
		return m_textFormat;
	}

	inline void CMashGUITextBox::SetTextFormat(eGUI_TEXT_FORMAT format)
	{
		m_textFormat = format;
	}

	inline bool CMashGUITextBox::GetAcceptReturn()const
	{
		return m_acceptReturn;
	}

	inline void CMashGUITextBox::SetAcceptReturn(bool value)
	{
		m_acceptReturn = value;
	}

	inline void CMashGUITextBox::SetTextAlignment(MashGUIFont::eFONT_ALIGNMENT eAlignment)
	{
		m_textHandler.SetAlignment(eAlignment);
	}

	inline const MashStringc& CMashGUITextBox::GetText()const
	{
		return m_textHandler.GetString();
	}

	inline bool CMashGUITextBox::GetWordWrap()const
	{
		return m_textHandler.GetWordWrap();
	}

	inline void CMashGUITextBox::SetFont(MashGUIFont *pFont)
	{
		m_textHandler.SetFont(pFont);
	}

	inline eMASH_GUI_TYPE CMashGUITextBox::GetGUIType()const
	{
		return aGUI_TEXT_BOX;
	}
}
#endif

