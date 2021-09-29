//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIStaticText.h"
#include "MashGUIManager.h"
#include "MashHelper.h"

namespace mash
{
	CMashGUIStaticText::CMashGUIStaticText(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement):MashGUIStaticText(pGUIManager, pInputManager, pParent, destination), m_bRenderBackground(false), m_styleElement(styleElement),
			m_autoResizeToFitText(false), m_fitToBoundsNeeded(false)
	{
		MashGUIStyle *activeStyle = pGUIManager->GetActiveGUIStyle();
		if (activeStyle)
		{
			m_textHandler.SetFormat(activeStyle->GetFont(), MashGUIFont::aTOP_LEFT, false);
		}
		
		m_textHandler.SetRegion(m_absoluteRegion, m_absoluteClippedRegion);
	}

	CMashGUIStaticText::~CMashGUIStaticText()
	{
	}

	void CMashGUIStaticText::OnStyleChange(MashGUIStyle *style)
	{
		m_textHandler.SetFormat(style->GetFont(), MashGUIFont::aTOP_LEFT, false);
	}
    
	void CMashGUIStaticText::AutoResizeToFitText(bool enable)
	{
		if (enable != m_autoResizeToFitText)
		{
			if (enable)
			{
				m_fitToBoundsNeeded = true;
			}

			m_autoResizeToFitText = enable;
		}
	}

	void CMashGUIStaticText::AddText(const MashStringc &text)
	{
		if (m_autoResizeToFitText)
		{
			m_fitToBoundsNeeded = true;

			MashStringc newString = m_textHandler.GetString() + text;
			uint32 x = 0;
			uint32 y = 0;
			m_textHandler.GetFont()->GetBoundingAreaForString(newString, mash::math::MaxUInt32(), x, y);

			MashGUIRect rect(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, (f32)x), MashGUIUnit(0.0f, (f32)y));
			SetDestinationRegion(rect);
		}

		m_textHandler.AddString(text);
	}

	void CMashGUIStaticText::SetText(const MashStringc &text)
	{
		m_textHandler.SetString(text);
	}

	void CMashGUIStaticText::SetTextInt(int32 val)
	{
		int8 buffer[256];
		mash::helpers::PrintToBuffer(buffer, 256, "%d", val);
		SetText(buffer);
	}

	void CMashGUIStaticText::SetTextFloat(f32 val, uint32 precision)
	{
		int8 buffer[256];
		mash::helpers::PrintToBuffer(buffer, 256, "%.*f", precision, val);
		SetText(buffer);
	}

	void CMashGUIStaticText::_AutoResizeToFitText()
	{
		uint32 x = 0;
		uint32 y = 0;
		m_textHandler.GetFont()->GetBoundingAreaForString(m_textHandler.GetString(), mash::math::MaxUInt32(), x, y);

		f32 sx = GetDestinationRegion().left.offset;
		f32 sy = GetDestinationRegion().top.offset;
		MashGUIRect rect(MashGUIUnit(0.0f, sx), MashGUIUnit(0.0f, sy), MashGUIUnit(0.0f, (f32)x + sx), MashGUIUnit(0.0f, (f32)y + sy));
		SetDestinationRegion(rect);
		
		m_fitToBoundsNeeded = false;
	}

	void CMashGUIStaticText::OnResize(bool positionChangeOnly, f32 deltaX, f32 deltaY)
	{
		if (positionChangeOnly)
		{
			m_textHandler.AddPosition(deltaX, deltaY);
		}
		else
		{
			m_textHandler.SetRegion(m_absoluteRegion, m_absoluteClippedRegion);
		}
	}

	void CMashGUIStaticText::Draw()
	{
		if (m_renderEnabled && (m_cullState != aCULL_CULLED))
		{
			MashGUIComponent::Draw();

			if (m_fitToBoundsNeeded)
				_AutoResizeToFitText();

			MashGUIStyle *activeStyle = m_GUIManager->GetActiveGUIStyle();
			MashGUISkin *activeSkin = activeStyle->GetAttributeSkin(m_styleElement, aGUI_ATTRIB_BACKGROUND);
			if (m_bRenderBackground)
			{
				m_GUIManager->DrawSprite(m_absoluteRegion, m_absoluteClippedRegion, activeSkin, m_overrideTransparency);
			}

			m_textHandler.Draw(m_GUIManager, activeSkin->fontColour, m_overrideTransparency);
		}
	}
}