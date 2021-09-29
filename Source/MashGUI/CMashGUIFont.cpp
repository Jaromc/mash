//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIFont.h"
#include "MashFileStream.h"
#include "MashFileManager.h"
#include "MashTexture.h"
#include "MashVector2.h"
#include "MashRectangle2.h"
#include "MashLog.h"
#include <fstream>
#include <iostream>
#include "MashDevice.h"
namespace mash
{
	uint32 g_mashGUIMaxFontCharSize = 128;

	struct sCharsetData
	{
		int32 iCharacterCount;
		int32 iMaxCharacterHeight;
		int32 iCharacterSize;
	};

	CMashGUIFont::CMashGUIFont():m_iLineHeight(0),m_MaxCharacterHeight(0),
		m_pTexture(0), m_spaceSize(0), m_charArray(0)
	{
		
	}

	CMashGUIFont::~CMashGUIFont()
	{
		if (m_pTexture)
		{
			m_pTexture->Drop();
			m_pTexture = 0;
		}

		if (m_charArray)
		{
			MASH_FREE(m_charArray);
			m_charArray = 0;
		}
	}

	eMASH_STATUS CMashGUIFont::LoadFont(mash::MashTexture *pTexture, const MashStringc &fontConfigFileName)
	{
		if (!pTexture)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Invalid Texture, Failed to load font.", 
				"CMashGUIFont::LoadFont");

			return aMASH_FAILED;
		}

		pTexture->Grab();

		if (m_pTexture)
			m_pTexture->Drop();

		m_pTexture = pTexture;
		m_pTexture->GetSize(m_iTextureWidth, m_iTextureHeight);

		const int8 *pFileContents = 0;
		int32 iCurrentLocation = 0;

		MashFileStream *openFileStream = MashDevice::StaticDevice->GetFileManager()->CreateFileStream();
		if (openFileStream->LoadFile(fontConfigFileName.GetCString(), aFILE_IO_BINARY) == aMASH_FAILED)
		{
			openFileStream->Destroy();

			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to open font file.", 
				"CMashGUIFont::LoadFont");

			return aMASH_FAILED;
		}

		pFileContents = (const int8*)openFileStream->GetData();

		sCharsetData charset;

		memcpy(&charset, &pFileContents[iCurrentLocation], sizeof(sCharsetData));
		iCurrentLocation += sizeof(sCharsetData);

		if (charset.iCharacterCount >= g_mashGUIMaxFontCharSize)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Invalid character count.", 
						"CMashGUIFont::LoadFont");

			return aMASH_FAILED;
		}

		sGlyphData *pGlyphData = (sGlyphData*)MASH_ALLOC_COMMON(sizeof(sGlyphData) * charset.iCharacterCount);
		memcpy(pGlyphData, &pFileContents[iCurrentLocation], sizeof(sGlyphData) * charset.iCharacterCount);
		iCurrentLocation += sizeof(sGlyphData) * charset.iCharacterCount;

		if (!m_charArray)
			m_charArray = (sGlyphData*)MASH_ALLOC_COMMON(sizeof(sGlyphData) * g_mashGUIMaxFontCharSize);

		for(int32 i = 0; i < charset.iCharacterCount; ++i)
			m_charArray[i].character = -1;//invalid int8

		for(int32 i = 0; i < charset.iCharacterCount; ++i)
		{
			if (pGlyphData[i].character < g_mashGUIMaxFontCharSize)
				m_charArray[pGlyphData[i].character] = pGlyphData[i];;
		}

		if (pGlyphData)
			MASH_FREE(pGlyphData);

		m_MaxCharacterHeight = charset.iMaxCharacterHeight;
		m_iCharacterSize = charset.iCharacterSize;
		m_spaceSize = ceil((f32)m_iCharacterSize / 2.0f);

		m_fontFormatFile = fontConfigFileName;

		openFileStream->Destroy();

		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIFont::GetSourceRect(const int8 character, mash::MashRectangle2 &out)const
	{
		int32 iAscii = (int32)character;
		if (iAscii < 0 || iAscii > g_mashGUIMaxFontCharSize)
			return aMASH_FAILED;
		if (m_charArray[iAscii].character == -1)
			return aMASH_FAILED;

		out.left = m_charArray[iAscii].rectLeft;
		out.top = m_charArray[iAscii].rectTop;
		out.right = m_charArray[iAscii].rectRight;
		out.bottom = m_charArray[iAscii].rectBottom;

		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIFont::GetScaledDestinationRect(const int8 character,
			f32 scale,
			mash::MashRectangle2 &out)const
	{
		int32 iAscii = (int32)character;
		if (iAscii < 0 || iAscii > g_mashGUIMaxFontCharSize)
			return aMASH_FAILED;
		if (m_charArray[iAscii].character == -1)
			return aMASH_FAILED;

		out.left = 0.0f;
		out.top = m_charArray[iAscii].offsetFromTop * scale;
		out.right = (out.left + (m_charArray[iAscii].rectRight - m_charArray[iAscii].rectLeft)) * scale;
		out.bottom = (out.top + (m_charArray[iAscii].rectBottom - m_charArray[iAscii].rectTop)) * scale;

		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIFont::GetDestinationRect(const int8 character, 
		const mash::MashVector2 &vCursorPos,
		mash::MashRectangle2 &out)const
	{
		int32 iAscii = (int32)character;
		if (iAscii < 0 || iAscii > g_mashGUIMaxFontCharSize)
			return aMASH_FAILED;
		if (m_charArray[iAscii].character == -1)
			return aMASH_FAILED;

		out.left = vCursorPos.x;
		out.top = vCursorPos.y + m_charArray[iAscii].offsetFromTop;
		out.right = out.left + (m_charArray[iAscii].rectRight - m_charArray[iAscii].rectLeft);
		out.bottom = out.top + (m_charArray[iAscii].rectBottom - m_charArray[iAscii].rectTop);

		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIFont::ValidateCharacter(int8 character)const
	{
		if (character == 32)
			return aMASH_OK;
		if (character == '\n')
			return aMASH_OK;

		int32 iAscii = (int32)character;
		if (iAscii < 0 || iAscii > g_mashGUIMaxFontCharSize)
			return aMASH_FAILED;
		if (m_charArray[iAscii].character == -1)
			return aMASH_FAILED;

		return aMASH_OK;
	}

	uint32 CMashGUIFont::GetCharacterWidth(const int8 character)
	{
		if (character == 32)
			return m_spaceSize;

		int32 iAscii = (int32)character;
		if (iAscii < 0 || iAscii > g_mashGUIMaxFontCharSize)
			return 0;
		if (m_charArray[iAscii].character == -1)
			return 0;

		return (m_charArray[iAscii].rectRight - m_charArray[iAscii].rectLeft);
	}

	//returns true if completly culled
	bool CMashGUIFont::ScissorTest(const mash::MashRectangle2 &clippingRect,
		mash::MashRectangle2 &texCoords,
		mash::MashRectangle2 &destRect)const
	{
		if (destRect.ClassifyGUI(clippingRect) == aCULL_CULLED)
			return true;

		mash::MashRectangle2 oldTexCoords = texCoords;

		if (destRect.left < clippingRect.left)
		{
			f32 fLerp = (clippingRect.left - destRect.left) / (destRect.right - destRect.left);
			texCoords.left = mash::math::Lerp(oldTexCoords.left, oldTexCoords.right, fLerp);
			destRect.left = clippingRect.left;
		}
		if (destRect.right > clippingRect.right)
		{
			f32 fLerp = (destRect.right - clippingRect.right) / (destRect.right - destRect.left);
			texCoords.right = mash::math::Lerp(oldTexCoords.right, oldTexCoords.left, fLerp);
			destRect.right = clippingRect.right;
		}
		if (destRect.top < clippingRect.top)
		{
			f32 fLerp = (clippingRect.top - destRect.top) / (destRect.bottom - destRect.top);
			texCoords.top = mash::math::Lerp(oldTexCoords.top, oldTexCoords.bottom, fLerp);
			destRect.top = clippingRect.top;
		}
		if (destRect.bottom > clippingRect.bottom)
		{
			f32 fLerp = (destRect.bottom - clippingRect.bottom) / (destRect.bottom - destRect.top);
			texCoords.bottom = mash::math::Lerp(oldTexCoords.bottom, oldTexCoords.top, fLerp);
			destRect.bottom = clippingRect.bottom;
		}

		return false;
	}

	uint32 CMashGUIFont::GetStringLength(const MashStringc &text)const
	{
		uint32 fLength = 0;
		const uint32 iStringLen = text.Size();
		for(int32 iChar = 0; iChar < iStringLen; ++iChar)
		{
			//get uv coords in texels
			mash::MashRectangle2 area;
			if (GetSourceRect(text[iChar], area) == aMASH_FAILED)
			{
				//handle space character
				if ((int32)text[iChar] == 32)
					fLength += m_spaceSize;
				else
					continue;
			}
			else
			{
				fLength += (area.right - area.left);
			}
		}

		return fLength;
	}

	void CMashGUIFont::GetBoundingAreaForString(const MashStringc &text, uint32 maxLineLength, uint32 &x, uint32 &y)
	{
		uint32 tempX = 0;
		uint32 tempY = 0;

		uint32 longestLineLength = 0;

		const uint32 iStringLen = text.Size();
		for(uint32 iChar = 0; iChar < iStringLen; ++iChar)
		{
			if (tempX > longestLineLength)
				longestLineLength = tempX;

			if (tempX >= maxLineLength)
			{
				tempY += m_MaxCharacterHeight;
				tempX = 0;
			}

			//get uv coords in texels
			mash::MashRectangle2 UVs;
			if (GetSourceRect(text[iChar], UVs) == aMASH_FAILED)
			{
				//handle space character
				if ((int32)text[iChar] == 32)
					tempX += m_spaceSize;
				else if ((int32)text[iChar] == '\n')//new line
				{
					tempY += m_MaxCharacterHeight;
					tempX = 0;
				}
				else
					continue;
			}
			else
			{
				tempX += (UVs.right - UVs.left);
			}
		}

		/*
			If there is more than one line then set the
			x val to the max line length.
		*/
		if (tempY > 0)
			x = longestLineLength;
		else
			x = tempX;

		/*
			Add one height to the y val.
		*/
		if (x > 0)
			y = m_MaxCharacterHeight + tempY;
		else
			y = 0;
	}

	uint32 CMashGUIFont::CalculateVertexBufferSizePrecise(const MashStringc &text)
	{
		uint32 iSizeInBytes = 0;
		const uint32 iStringLen = text.Size();
		for(int32 iChar = 0; iChar < iStringLen; ++iChar)
		{
			//TODO : Handle tab
			if (text[iChar] != '\n' &&
				text[iChar] != 32)
			{
				iSizeInBytes += 6;
			}
		}

		iSizeInBytes *= sizeof(mash::MashVertexPosTex::sMashVertexPosTex);
		return iSizeInBytes;
	}

	uint32 CMashGUIFont::CalculateVertexBufferSizeQuick(const MashStringc &text)
	{
		return (text.Size() * 6 * sizeof(mash::MashVertexPosTex::sMashVertexPosTex));
	}

	void CMashGUIFont::GenerateStringData(sCachedStringData &cacheData, const MashStringc &text, bool wordWrap, const mash::MashRectangle2 &drawingArea)
	{
		const uint32 drawingAreaWidth = drawingArea.right - drawingArea.left;
		uint32 currentLineWidth = 0;
		uint32 currentLineCharCount = 0;
		uint32 currentLineCount = 0;
		const uint32 stringLength = text.Size();

		cacheData.allChars.Clear();
		cacheData.lineData.Clear();

		cacheData.allChars.Reserve(stringLength);

		for(uint32 i = 0; i < stringLength; ++i)
		{
			if (text[i] == 32)//space char
			{
				if (wordWrap && ((currentLineWidth + m_iCharacterSize) > drawingAreaWidth))
				{
					cacheData.lineData.PushBack(sCachedStringData::sLineData(currentLineWidth, currentLineCharCount));
					currentLineWidth = 0;
					currentLineCharCount = 0;
					currentLineCount++;
				}
				else
				{
					currentLineWidth += m_spaceSize;
				}
			}
			else if (text[i] == '\n')
			{
				cacheData.lineData.PushBack(sCachedStringData::sLineData(currentLineWidth, currentLineCharCount));
				currentLineWidth = 0;
				currentLineCharCount = 0;
				currentLineCount++;
			}
			else
			{
				int32 iAscii = (int32)text[i];
				if ((iAscii > -1) && (iAscii <= g_mashGUIMaxFontCharSize) && (m_charArray[iAscii].character != -1))
				{	
					uint32 currentCharWidth = m_charArray[iAscii].rectRight - m_charArray[iAscii].rectLeft;

					if (wordWrap)
					{
						if ((currentLineWidth + currentCharWidth) > drawingAreaWidth)
						{
							cacheData.lineData.PushBack(sCachedStringData::sLineData(currentLineWidth, currentLineCharCount));
							currentLineWidth = 0;
							currentLineCharCount = 0;
							currentLineCount++;
						}
					}

					cacheData.allChars.PushBack(sCachedStringData::sCharData(&m_charArray[iAscii], currentLineWidth, currentLineCount * (uint32)m_MaxCharacterHeight));
					currentLineWidth += currentCharWidth;
					++currentLineCharCount;
				}
			}
		}

		if (currentLineCharCount > 0)
		{
			cacheData.lineData.PushBack(sCachedStringData::sLineData(currentLineWidth, currentLineCharCount));
		}
	}

	eMASH_STATUS CMashGUIFont::DrawText(const MashStringc &text,  
			bool bWordWrap, eFONT_ALIGNMENT eTextAlignment,
			const MashVector2 &textOffset,
			const mash::MashRectangle2 &drawingArea,
			const mash::MashRectangle2 &clippingArea,			
			mash::MashVertexPosTex::sMashVertexPosTex *pVerticesOut, uint32 &iVertexCountOut)
	{
		iVertexCountOut = 0;
		if (text.Empty())
			return aMASH_OK;

		const f32 texWidth = 1.0f / (f32)m_iTextureWidth;
		const f32 texHeight = 1.0f / (f32)m_iTextureHeight;

		sCachedStringData cacheData;
		GenerateStringData(cacheData, text, bWordWrap, drawingArea);

		mash::MashRectangle2 validatedClippingArea = clippingArea;
		if (drawingArea.left > validatedClippingArea.left)
			validatedClippingArea.left = drawingArea.left;
		if (drawingArea.right < validatedClippingArea.right)
			validatedClippingArea.right = drawingArea.right;
		if (drawingArea.top > validatedClippingArea.top)
			validatedClippingArea.top = drawingArea.top;
		if (drawingArea.bottom < validatedClippingArea.bottom)
			validatedClippingArea.bottom = drawingArea.bottom;
		
		const uint32 lineCount = cacheData.lineData.Size();

		//find the alignment on the Y axis
		f32 alignAdjustmentY = 0.0f;
		switch(eTextAlignment)
		{
		case aCENTER:
		case aRIGHT_CENTER:
		case aLEFT_CENTER:
			{
				alignAdjustmentY = ((f32)(drawingArea.bottom - drawingArea.top) * 0.5f) - ((f32)(m_MaxCharacterHeight * lineCount) * 0.5f);
				break;
			}
		case aBOTTOM_CENTER:
		case aBOTTOM_LEFT:
		case aBOTTOM_RIGHT:
			{
				alignAdjustmentY = (drawingArea.bottom - drawingArea.top) - (m_MaxCharacterHeight * lineCount);
				break;
			}
		default:
			{
				uint32 maxLineCount = (drawingArea.bottom - drawingArea.top) / m_MaxCharacterHeight;
				if (lineCount > maxLineCount)
					alignAdjustmentY = (f32)((lineCount - maxLineCount) * m_MaxCharacterHeight) * -1.0f;
				break;
			}
		};

		mash::MashVector2 tempVerts[6];
		uint32 currentCharCount = 0;
		for(uint32 line = 0; line < lineCount; ++line)
		{
			//find the alignment on the X axis
			f32 alignAdjustmentX = 0.0f;
			switch(eTextAlignment)
			{
			case aRIGHT_CENTER:
			case aBOTTOM_RIGHT:
			case aTOP_RIGHT:
				{
					alignAdjustmentX = (drawingArea.right - drawingArea.left) - cacheData.lineData[line].lineWidth;
					break;
				}
			case aTOP_CENTER:
			case aBOTTOM_CENTER:
			case aCENTER:
				{
					alignAdjustmentX = ((drawingArea.right - drawingArea.left) * 0.5f) - (cacheData.lineData[line].lineWidth * 0.5f);
					break;
				}
			default:
				{
					//the font must be left justified so no action is needed.
					break;
				}
			};

			const uint32 charsOnLine = cacheData.lineData[line].charCount;
			for(uint32 c = 0; c < charsOnLine; ++c)
			{
				sCachedStringData::sCharData *currentChar = &cacheData.allChars[currentCharCount];

				/*
					Pointers set to null are space chars.
				*/
				if (currentChar->data)
				{
					mash::MashRectangle2 uv(currentChar->data->rectLeft,
						currentChar->data->rectTop, 
						currentChar->data->rectRight, 
						currentChar->data->rectBottom);

					f32 offset = currentChar->data->offsetFromTop;// * 0.5f;
					mash::MashRectangle2 destRect(drawingArea.left + alignAdjustmentX + currentChar->localx,
						drawingArea.top + alignAdjustmentY + offset + currentChar->localy,
						drawingArea.left + alignAdjustmentX + currentChar->localx + (currentChar->data->rectRight - currentChar->data->rectLeft),
						drawingArea.top + alignAdjustmentY + currentChar->localy + offset + (currentChar->data->rectBottom - currentChar->data->rectTop));

					destRect.left += textOffset.x;
					destRect.right += textOffset.x;
					destRect.top += textOffset.y;
					destRect.bottom += textOffset.y;

					//do scissor test
					if (!ScissorTest(validatedClippingArea, uv, destRect))
					{
						pVerticesOut[iVertexCountOut].texCoord.x = uv.left * texWidth;
						pVerticesOut[iVertexCountOut].texCoord.y = uv.bottom * texHeight;
						pVerticesOut[iVertexCountOut+1].texCoord.x = uv.left * texWidth;
						pVerticesOut[iVertexCountOut+1].texCoord.y = uv.top * texHeight;
						pVerticesOut[iVertexCountOut+2].texCoord.x = uv.right * texWidth;
						pVerticesOut[iVertexCountOut+2].texCoord.y = uv.top * texHeight;

						pVerticesOut[iVertexCountOut+3].texCoord.x = uv.left * texWidth;
						pVerticesOut[iVertexCountOut+3].texCoord.y = uv.bottom * texHeight;
						pVerticesOut[iVertexCountOut+4].texCoord.x = uv.right * texWidth;
						pVerticesOut[iVertexCountOut+4].texCoord.y = uv.top * texHeight;
						pVerticesOut[iVertexCountOut+5].texCoord.x = uv.right * texWidth;
						pVerticesOut[iVertexCountOut+5].texCoord.y = uv.bottom * texHeight;

						destRect.GetPointsAsTris(tempVerts);
						for(uint32 pos = 0; pos < 6; ++pos)
						{
							pVerticesOut[iVertexCountOut+pos].position.x = (int32)(tempVerts[pos].x);
							pVerticesOut[iVertexCountOut+pos].position.y = (int32)(tempVerts[pos].y);
							pVerticesOut[iVertexCountOut+pos].position.z = (int32)0.1f;
						}

						iVertexCountOut += 6;
					}
				}

				++currentCharCount;
			}
		}
	
		return aMASH_OK;
	}
}