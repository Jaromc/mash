//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_STATIC_TEXT_H_
#define _C_MASH_GUI_STATIC_TEXT_H_

#include "MashGUIStaticText.h"
#include "CMashTextHelper.h"

namespace mash
{
	class CMashGUIStaticText : public MashGUIStaticText
	{
	protected:
		MashStringc m_text;
		int32 m_styleElement;

		CMashTextHelper m_textHandler;
		bool m_bRenderBackground;

		bool m_autoResizeToFitText;
		bool m_fitToBoundsNeeded;

		void _AutoResizeToFitText();
		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
	public:
		CMashGUIStaticText(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement);

		~CMashGUIStaticText();

		void SetWordWrap(bool wrap);
		bool GetWordWrap()const;

		void Draw();
		eMASH_GUI_TYPE GetGUIType()const;

		void SetFont(MashGUIFont *pFont);

		void SetTextFloat(f32 val, uint32 precision = 5);
		void SetTextInt(int32 val);
		void SetText(const MashStringc &text);
		const MashStringc& GetText()const;
		void AddText(const MashStringc &text);

		void EnableWordWrap(bool bEnable);
		bool GetIsWordWrapEnabled()const;

		void SetRenderBackground(bool bValue);

		void AutoResizeToFitText(bool enable);
		bool GetAutoResizeToFitText()const;

		void OnStyleChange(MashGUIStyle *style);

		void SetTextAlignment(MashGUIFont::eFONT_ALIGNMENT eAlignment);
		MashGUIFont::eFONT_ALIGNMENT GetTextAlignment()const;
	};

	inline bool CMashGUIStaticText::GetAutoResizeToFitText()const
	{
		return m_autoResizeToFitText;	
	}

	inline const MashStringc& CMashGUIStaticText::GetText()const
	{
		return m_textHandler.GetString();
	}

	inline MashGUIFont::eFONT_ALIGNMENT CMashGUIStaticText::GetTextAlignment()const
	{
		return m_textHandler.GetAlignment();
	}

	inline void CMashGUIStaticText::SetTextAlignment(MashGUIFont::eFONT_ALIGNMENT eAlignment)
	{
		m_textHandler.SetAlignment(eAlignment);
	}

	inline void CMashGUIStaticText::SetRenderBackground(bool bValue)
	{
		m_bRenderBackground = bValue;
	}

	inline void CMashGUIStaticText::SetWordWrap(bool bEnable)
	{
		m_textHandler.SetWordWrap(bEnable);
	}

	inline bool CMashGUIStaticText::GetWordWrap()const
	{
		return m_textHandler.GetWordWrap();
	}

	inline void CMashGUIStaticText::SetFont(MashGUIFont *pFont)
	{
		m_textHandler.SetFont(pFont);
	}

	inline eMASH_GUI_TYPE CMashGUIStaticText::GetGUIType()const
	{
		return aGUI_STATIC_TEXT;
	}
}
#endif

