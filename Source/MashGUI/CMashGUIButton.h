//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_BUTTON_H_
#define _C_MASH_GUI_BUTTON_H_

#include "MashGUIButton.h"
#include "MashGUIFont.h"
#include "CMashTextHelper.h"
namespace mash
{
	class CMashGUIButton : public MashGUIButton
	{
	private:
		enum eBUTTON_STATE
		{
			aSTATE_UP,
			aSTATE_DOWN
		};

	protected:
		bool m_isSwitch;
		bool m_switchIsPressed;

		int32 m_styleElement;

		eBUTTON_STATE m_buttonState;

		CMashTextHelper m_textHandler;

		eGUI_STYLE_ATTRIBUTE m_upStyle;
		eGUI_STYLE_ATTRIBUTE m_downStyle;
		eGUI_STYLE_ATTRIBUTE m_hoverStyle;

		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);

	public:
		CMashGUIButton(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement);
		virtual ~CMashGUIButton();

		void SetButtonStyles(int32 elementId, eGUI_STYLE_ATTRIBUTE up, eGUI_STYLE_ATTRIBUTE down, eGUI_STYLE_ATTRIBUTE hover);

		void OnEvent(const sInputEvent &eventData);

		eMASH_GUI_TYPE GetGUIType()const;

		void SetIsSwitch(bool state);
		bool IsSwitch()const;

		void SetSwitchState(bool isPressed);
		bool IsButtonDown()const;

		void Draw();

		void OnStyleChange(MashGUIStyle *style);

		void SetText(const MashStringc &text);
		const MashStringc& GetText()const;
	};

	inline void CMashGUIButton::SetIsSwitch(bool state)
	{
		m_isSwitch = state;
	}

	inline bool CMashGUIButton::IsSwitch()const
	{
		return m_isSwitch;
	}

	inline bool CMashGUIButton::IsButtonDown()const
	{
		return m_switchIsPressed;
	}

	inline const MashStringc& CMashGUIButton::GetText()const
	{
		return m_textHandler.GetString();
	}

	inline eMASH_GUI_TYPE CMashGUIButton::GetGUIType()const
	{
		return aGUI_BUTTON;
	}
}

#endif