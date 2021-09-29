//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_FONT_H_
#define _C_MASH_GUI_FONT_H_

#include "MashGUIFont.h"
#include <map>
#include "MashArray.h"

namespace mash
{
	class CMashGUIFont : public MashGUIFont
	{
	private:
		struct sGlyphData
		{
			int32 character;
			int32 rectLeft;
			int32 rectTop;
			int32 rectRight;
			int32 rectBottom;
			int32 offsetFromTop;
		};

		struct sCachedStringData
		{
			struct sCharData
			{
				sGlyphData *data;
				uint32 localx;
				uint32 localy;

				sCharData():data(0), localx(0), localy(0){}
				sCharData(sGlyphData *d, uint32 lx, uint32 ly):data(d), localx(lx), localy(ly){}
			};

			struct sLineData
			{
				uint32 lineWidth;
				uint32 charCount;

				sLineData():lineWidth(0), charCount(0){}
				sLineData(uint32 lw, uint32 cc):lineWidth(lw), charCount(cc){}
			};

			MashArray<sCharData> allChars;
			MashArray<sLineData> lineData;
		};

	private:
		sGlyphData *m_charArray;
		uint16 m_iLineHeight;
		uint16 m_MaxCharacterHeight;
		uint16 m_iCharacterSize;
		uint16 m_spaceSize;

		uint32 m_iTextureWidth;
		uint32 m_iTextureHeight;

		mash::MashTexture *m_pTexture;
		MashStringc m_fontFormatFile;

		bool ScissorTest(const mash::MashRectangle2 &clippingRect,
			mash::MashRectangle2 &texCoords,
			mash::MashRectangle2 &destRect)const;

		eMASH_STATUS GetScaledDestinationRect(const int8 cCharacter,
			f32 scale,
			mash::MashRectangle2 &out)const;
	public:
		CMashGUIFont();

		virtual ~CMashGUIFont();

		//! Generates character data and position for string rendering
		/*!
			This is called before DrawText() so this data can be passed into
			that function. This data can also be used for carat positioning.

			\param cacheDataOut Character data will be added to this container.
			\param text String to generate data for.
			\param wordWrap Enable word wrap.
			\param drawingArea Drawing area.
		*/
		void GenerateStringData(sCachedStringData &cacheData, const MashStringc &text, bool wordWrap, const mash::MashRectangle2 &drawingArea);

		eMASH_STATUS LoadFont(mash::MashTexture *pTexture, const MashStringc &fontConfigFileName);

		eMASH_STATUS ValidateCharacter(int8 c)const;
		eMASH_STATUS GetSourceRect(const int8 cCharacter, mash::MashRectangle2 &out)const;

		eMASH_STATUS GetDestinationRect(const int8 cCharacter, 
			const mash::MashVector2 &vCursorPos,
			mash::MashRectangle2 &out)const;

		uint32 CalculateVertexBufferSizePrecise(const MashStringc &text);
		uint32 CalculateVertexBufferSizeQuick(const MashStringc &text);

		void GetBoundingAreaForString(const MashStringc &text, uint32 maxLineLength, uint32 &x, uint32 &y);

		uint32 GetCharacterWidth(const int8 character);
		uint32 GetMaxCharacterWidth()const;

		mash::MashTexture* GetFontTexture()const;
		const MashStringc& GetFontFormatFileName()const;

		uint32 GetStringLength(const MashStringc &text)const;
		uint32 GetMaxCharacterHeight()const;

		eMASH_STATUS DrawText(const MashStringc &text,  
			bool bWordWrap, eFONT_ALIGNMENT textAlignment,
			const MashVector2 &textOffset,
			const mash::MashRectangle2 &drawingArea,
			const mash::MashRectangle2 &clippingArea,
			mash::MashVertexPosTex::sMashVertexPosTex *verticesOut, uint32 &vertexCountOut);

	};

	inline uint32 CMashGUIFont::GetMaxCharacterWidth()const
	{
		return m_iCharacterSize;
	}

	inline const MashStringc& CMashGUIFont::GetFontFormatFileName()const
	{
		return m_fontFormatFile;
	}

	inline uint32 CMashGUIFont::GetMaxCharacterHeight()const
	{
		return m_MaxCharacterHeight;
	}

	inline mash::MashTexture* CMashGUIFont::GetFontTexture()const
	{
		return m_pTexture;
	}
}

#endif