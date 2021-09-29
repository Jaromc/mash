//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashTextHelper.h"
#include "MashDevice.h"
namespace mash
{
	CMashTextHelper::CMashTextHelper():m_reservedVertexBufferSize(0), m_textVerticesPtr(0),
		m_currentTextVerticesCount(0), m_font(0), m_text(""),
		m_textAlignment(MashGUIFont::aCENTER), m_absRegion(0, 0, 0, 0), m_absClippingRegion(0, 0, 0, 0),
		m_caratCharacterIndex(0), m_updateFlags(0), m_wordWrap(false), m_moveAmountX(0), m_moveAmountY(0),
		m_caratTextOffset(0.0f, 0.0f), m_caratPosition(0.0f, 0.0f), m_caratEnabled(false)
	{

	}

	CMashTextHelper::CMashTextHelper(const CMashTextHelper &copyFrom):m_textVerticesPtr(copyFrom.m_textVerticesPtr), m_reservedVertexBufferSize(copyFrom.m_reservedVertexBufferSize),
		m_currentTextVerticesCount(copyFrom.m_currentTextVerticesCount), m_font(copyFrom.m_font), m_text(copyFrom.m_text),
		m_textAlignment(copyFrom.m_textAlignment), m_absRegion(copyFrom.m_absRegion), m_absClippingRegion(copyFrom.m_absClippingRegion), m_absClippedRegion(copyFrom.m_absClippedRegion),
		m_caratCharacterIndex(copyFrom.m_caratCharacterIndex), m_updateFlags(copyFrom.m_updateFlags), m_wordWrap(copyFrom.m_wordWrap),
		m_caratTextOffset(copyFrom.m_caratTextOffset), m_caratPosition(copyFrom.m_caratPosition), m_caratEnabled(copyFrom.m_caratEnabled), 
		m_moveAmountX(copyFrom.m_moveAmountX), m_moveAmountY(copyFrom.m_moveAmountY)
	{
		if (m_font)
			m_font->Grab();
	}

	CMashTextHelper::~CMashTextHelper()
	{
		if (m_font)
		{
			m_font->Drop();
			m_font = 0;
		}
	}

	void CMashTextHelper::SetFormat(MashGUIFont *font, MashGUIFont::eFONT_ALIGNMENT textAlignment, bool wordWrap)
	{
		if ((m_font != font) || 
			(m_textAlignment != textAlignment) || 
			(m_wordWrap != wordWrap))
		{
			SetFont(font);
			SetAlignment(textAlignment);
			SetWordWrap(wordWrap);
		}
	}

	void CMashTextHelper::SetString(const MashStringc &text)
	{
		if (m_text != text)
		{
			m_caratTextOffset.Zero();
			m_caratCharacterIndex = 0;

			m_text.Clear();
			InsertString(0, text.GetCString());
		}
	}

	void CMashTextHelper::Update()
	{
		bool updateText = m_updateFlags & eTEXT_UPDATE_FLAG_FULL;

		if (!updateText)
		{
			if (m_caratEnabled)
			{
				m_caratPosition.x += m_moveAmountX;
				m_caratPosition.y += m_moveAmountY;
			}

			if (m_textVerticesPtr.Get())
			{
				for(uint32 i = 0; i < m_currentTextVerticesCount; ++i)
				{
					m_textVerticesPtr.Get()[i].position.x += m_moveAmountX;
					m_textVerticesPtr.Get()[i].position.y += m_moveAmountY;
				}
			}
		}

		if (updateText)
		{
			if (!m_text.Empty())
			{
				//full update needed
				uint32 iNewVertexBufferSize = m_font->CalculateVertexBufferSizeQuick(m_text.GetCString());
				if ((!m_textVerticesPtr.Get() && (iNewVertexBufferSize > 0)) || (m_reservedVertexBufferSize < iNewVertexBufferSize))
				{
					m_textVerticesPtr = (mash::MashVertexPosTex::sMashVertexPosTex*)MASH_ALLOC_COMMON(iNewVertexBufferSize);
					m_reservedVertexBufferSize = iNewVertexBufferSize;
				}

				if (m_font->DrawText(m_text.GetCString(), m_wordWrap, m_textAlignment, m_caratTextOffset, m_absRegion,  
					m_absClippingRegion, m_textVerticesPtr.Get(), m_currentTextVerticesCount) == aMASH_FAILED)
				{
					//return aMASH_FAILED;
				}
			}
			else
			{
				m_currentTextVerticesCount = 0;
			}
		}
	}

	void CMashTextHelper::MoveCaratLeft()
	{
		if (m_caratEnabled)
		{
			if (m_caratCharacterIndex > 0)
			{
				UpdateCaratOffset(m_caratCharacterIndex-1);
			}
		}
	}

	void CMashTextHelper::MoveCaratRight()
	{
		if (m_caratEnabled)
		{
			if (m_caratCharacterIndex < m_text.Size())
			{
				UpdateCaratOffset(m_caratCharacterIndex+1);
			}
		}
	}

	void CMashTextHelper::SendCaratToFront()
	{
		if (m_caratEnabled)
		{
			while(m_caratCharacterIndex > 0)
			{
				MoveCaratLeft();
			}
		}
	}

	void CMashTextHelper::SendCaratToBack()
	{
		if (m_caratEnabled)
		{
			while(m_caratCharacterIndex < m_text.Size())
			{
				MoveCaratRight();
			}
		}
	}

	void CMashTextHelper::UpdateCaratOffset(int32 newCaratIndex)
	{
		if (newCaratIndex == 0)
		{
			m_caratTextOffset.Zero();
			m_updateFlags |= eTEXT_UPDATE_FLAG_FULL;

			m_caratPosition.x = m_absRegion.left;
			m_caratPosition.y = m_absRegion.top;
		}
		else
		{
			if (m_font)
			{
				uint32 lineWidth = 0;
				for(uint32 i = 0; i < newCaratIndex; ++i)
					lineWidth += m_font->GetCharacterWidth(m_text[i]);

				uint32 newCharWidth = m_font->GetCharacterWidth(m_text[newCaratIndex-1]);

				if ((lineWidth + m_caratTextOffset.x + m_absRegion.left) < m_absRegion.left)
				{
					m_caratTextOffset.x += newCharWidth;
					m_updateFlags |= eTEXT_UPDATE_FLAG_FULL;
				}
				else if ((lineWidth + m_caratTextOffset.x + m_absRegion.left) > m_absRegion.right)
				{
					m_caratTextOffset.x -= newCharWidth;
					m_updateFlags |= eTEXT_UPDATE_FLAG_FULL;
				}

				m_caratPosition.x = m_absRegion.left + m_caratTextOffset.x + lineWidth;
				m_caratPosition.y = m_absRegion.top + m_caratTextOffset.y;
			}
		}
		
		/*
			Hacked in...currently only textbox use carats and their text is
			always aligned to the left side. So by default the carat is already positioned
			to the top left. This fixes the position if the text is centered.
		*/
		if (m_textAlignment == MashGUIFont::aLEFT_CENTER)
		{
			f32 caratHeight = m_font->GetMaxCharacterHeight();
			m_caratPosition.y += ((m_absRegion.bottom - m_absRegion.top) - caratHeight) * 0.5f;
		}

		m_caratCharacterIndex = newCaratIndex;
	}

	const mash::MashVector2& CMashTextHelper::GetCaratAbsPosition()const
	{
		return m_caratPosition;
	}

	void CMashTextHelper::AddPosition(f32 x, f32 y)
	{
		m_absRegion.left += x;
		m_absRegion.right += x;
		m_absRegion.top += y;
		m_absRegion.bottom += y;

		m_absClippingRegion.left += x;
		m_absClippingRegion.right += x;
		m_absClippingRegion.top += y;
		m_absClippingRegion.bottom += y;

		m_absClippedRegion.left += x;
		m_absClippedRegion.right += x;
		m_absClippedRegion.top += y;
		m_absClippedRegion.bottom += y;

		m_moveAmountX += x;
		m_moveAmountY += y;

		m_updateFlags |= eTEXT_UPDATE_POSITION_ONLY;
	}

	void CMashTextHelper::SetRegion(const mash::MashRectangle2 &absRect, const mash::MashRectangle2 &clippingRect)
	{
		if ((absRect == m_absRegion) && 
			(m_absClippingRegion == clippingRect))
		{
			return;
		}

		m_absRegion = absRect;
		m_absClippingRegion = clippingRect;

		m_absClippedRegion = m_absRegion;
		m_absClippedRegion.ClipGUI(m_absClippingRegion);

		m_updateFlags = eTEXT_UPDATE_FLAG_FULL;

		if (m_caratEnabled)
			UpdateCaratOffset(m_caratCharacterIndex);
	}

	void CMashTextHelper::SetFont(MashGUIFont *font)
	{
		if (m_font != font)
		{
			MashStringc tempText = m_text;
			RemoveCharacters(m_text.Size());

			if (font)
				font->Grab();

			if (m_font)
				m_font->Drop();

			m_font = font;
			m_updateFlags |= eTEXT_UPDATE_FLAG_FULL;

			SetString(tempText);
		}
	}

	void CMashTextHelper::SetAlignment(MashGUIFont::eFONT_ALIGNMENT val)
	{
		if (val != m_textAlignment)
		{
			m_textAlignment = val;
			m_updateFlags |= eTEXT_UPDATE_FLAG_FULL;
		}
	}

	void CMashTextHelper::SetWordWrap(bool val)
	{
		if (val != m_wordWrap)
		{
			m_wordWrap = val;
			m_updateFlags |= eTEXT_UPDATE_FLAG_FULL;
		}
	}

	void CMashTextHelper::AddString(const MashStringc &text)
	{
		InsertString(m_caratCharacterIndex, text.GetCString());
	}

	void CMashTextHelper::AddCharacter(int8 c)
	{
		InsertCharacter(m_caratCharacterIndex, c);
	}

	void CMashTextHelper::InsertString(uint32 location, const int8 *string)
	{
		uint32 stringLength = strlen(string);
		m_text.Reserve(stringLength);

		for(uint32 i = 0; i < stringLength; ++i)
			InsertCharacter(location+i, string[i]);
	}

	void CMashTextHelper::InsertCharacter(uint32 location, int8 c)
	{
		if (location <= m_text.Size())
		{
			m_text.Insert(location, c);
			m_updateFlags |= eTEXT_UPDATE_FLAG_FULL;

			MoveCaratRight();
		}
	}

	void CMashTextHelper::RemoveCharacters(uint32 count)
	{
		if (m_caratEnabled)
		{
			if (m_caratCharacterIndex >= count)
			{
				m_text.Erase(m_caratCharacterIndex - count, count);
				m_updateFlags |= eTEXT_UPDATE_FLAG_FULL;

				for(uint32 i = 0; i < count; ++i)
					MoveCaratLeft();
			}
		}
		else
		{
			if (count <= m_text.Size())
			{
				m_text.Erase(m_text.Size() - count, count);
				m_updateFlags |= eTEXT_UPDATE_FLAG_FULL;
			}
		}
	}

	eMASH_STATUS CMashTextHelper::Draw(MashGUIManager *manager, const mash::sMashColour &colour, const sGUIOverrideTransparency &overrideTransparency)
	{
		if (m_updateFlags != 0)
		{
			Update();
			m_moveAmountX = 0;
			m_moveAmountY = 0;
			m_updateFlags = 0;
		}

		if (!m_text.Empty())
		{
			sMashColour fontColour = colour;
			if (overrideTransparency.enableOverrideTransparency && overrideTransparency.affectFontAlpha)
				fontColour.SetAlpha(overrideTransparency.alphaValue);

            manager->DrawText(m_textVerticesPtr.Get(),
                m_currentTextVerticesCount,
                m_font->GetFontTexture(), fontColour);
		}

		return aMASH_OK;
	}
}