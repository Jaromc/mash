//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_FONT_H_
#define _MASH_GUI_FONT_H_

#include "MashReferenceCounter.h"
#include "MashTypes.h"
#include "MashEnum.h"
#include "MashDataTypes.h"
#include "MashString.h"

namespace mash
{
	class MashVector2;
	class MashRectangle2;
	class MashTexture;

	/*!
		Main class for generating text.
	*/
	class MashGUIFont : public MashReferenceCounter
	{
	public:
		enum eFONT_ALIGNMENT
		{
			aCENTER,
			aRIGHT_CENTER,
			aLEFT_CENTER,
			aTOP_CENTER,
			aBOTTOM_CENTER,
			aTOP_LEFT,
			aTOP_RIGHT,
			aBOTTOM_LEFT,
			aBOTTOM_RIGHT
		};
	public:
		MashGUIFont():MashReferenceCounter(){}
		virtual ~MashGUIFont(){}

		//! Returns ok if the the character is valid ascii char.
		/*!
			Ok will also be returned if the char is a space or new line char.

			\return Fail if the char is not included in this font.
		*/
		virtual eMASH_STATUS ValidateCharacter(int8 c)const = 0;

		//! Returns the rect that encapsulates the gicen character.
		/*!
			\param character The character to query.
			\param out the Rect for this character.
			\return failed if this character is not given in the current set.
		*/
		virtual eMASH_STATUS GetSourceRect(const int8 character, MashRectangle2 &out)const = 0;

		//! Returns the rendering location for a character.
		/*!
			\param character The character to query.
			\param cursorPos Current cursor pos in a given space.
			\param out returns the source rect in the cursors space.
			\return failed if this character is not given in the current set.
		*/
		virtual eMASH_STATUS GetDestinationRect(const int8 character, 
			const MashVector2 &cursorPos,
			MashRectangle2 &out)const = 0;

		//! Returns the area a string will fill.
		/*!
			\param text String to reference.
			\param maxLineLength This will be the length at which text will begin to wrap.
			\param x Final bounding width.
			\param y final bounding height.
		*/
		virtual void GetBoundingAreaForString(const MashStringc &text, uint32 maxLineLength, uint32 &x, uint32 &y) = 0;
		
		//! Returns the pixel length of a string if it were layed out on a single line.
		/*!
			This ignors new line characters.
			\param text String to reference.
		*/
		virtual uint32 GetStringLength(const MashStringc &text)const = 0;

		//! Largest character height.
		/*!
			\return Largest character height.
		*/
		virtual uint32 GetMaxCharacterHeight()const = 0;

		//! Largest character width.
		/*!
			\return Largest character width.
		*/
		virtual uint32 GetMaxCharacterWidth()const = 0;

		//! Determines how much memory is needed to render the given string.
		/*!
			This can be useful for preallocating before calling DrawText().
			Since this method is precise, it may be a little slow. If memory
			is not a concern then maybe try CalculateVertexBufferSizeQuick().

			\param text String to query.
			\return Vertex buffer size in bytes needed.
		*/
		virtual uint32 CalculateVertexBufferSizePrecise(const MashStringc &text) = 0;

		//! Determines how much memory is needed to render the given string.
		/*!
			Since this is a quick method, it will most likely return a size greater
			than whats needed. If you want an exact total then call CalculateVertexBufferSizePrecise().

			This can be useful for preallocating before calling DrawText().
			\param text String to query.
			\return Vertex buffer size in bytes needed.
		*/
		virtual uint32 CalculateVertexBufferSizeQuick(const MashStringc &text) = 0;

		//! Returns the width of a character.
		/*!
			\param character to query.
			\return Character width. Or 0 if the character is not within the current set.
		*/
		virtual uint32 GetCharacterWidth(const int8 character) = 0;

		//! Returns the font sheet.
		/*!
			\return Font sheet.
		*/
		virtual mash::MashTexture* GetFontTexture()const = 0;

		//! Returns the naem of the format file used for this font.
		/*!
			\return Loaded font file name.
		*/
		virtual const MashStringc& GetFontFormatFileName()const = 0;

		//! Creates a renderable version of the given text.
		/*!
			The result of this function can be used to render text to the screen. 
			This method can be slow and is best used infrequently or with small strings.

			This function assumes verticesOut has enough room to hold the final data.
			CalculateVertexBufferSizePrecise() or CalculateVertexBufferSizeQuick() can be called
			prior to calling this function to determine what size may be needed.

			\param text String to reference. 
			\param textAlignment How the text should be aligned to the drawing area.
			\param drawingArea Whole area for the text to fill.
			\param clippingArea Some area that determines if any text will be clipped from the drawingArea.
			\param verticesOut Final vertices.
			\param vertexCountOut Vertices used.
			\return Failed if any errors occured. Ok Otherwise.
		*/
		virtual eMASH_STATUS DrawText(const MashStringc &text,  
			bool bWordWrap, eFONT_ALIGNMENT textAlignment,
			const MashVector2 &textOffset,
			const MashRectangle2 &drawingArea,
			const MashRectangle2 &clippingArea,			
			MashVertexPosTex::sMashVertexPosTex *verticesOut, 
			uint32 &vertexCountOut) = 0;
	};
}

#endif