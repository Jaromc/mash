//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_STYLE_H_
#define _MASH_GUI_STYLE_H_

#include "MashReferenceCounter.h"
#include "MashGUITypes.h"
#include "MashString.h"
#include "MashArray.h"

namespace mash
{
	class MashGUIFont;
	class MashGUISkin;

	/*!
		Styles determines how the gui components will look.
		They store all the skins for each elements attributes (eg, button up, button down, mouse hover...).

		Styles are a common hub for all components, so changing something in a style will affect all 
		active gui components.

		Default elements can be of type eGUI_STYLE_ELEMENT or a custom value. Custom values allow you to create
		multiple components of the same type but have different skins. This is handy for say an editor where each
		button could have a different texture to describe what it does.
	*/
	class MashGUIStyle : public MashReferenceCounter
	{
	public:
		struct sCollectionSkinAttrib
		{
			eGUI_STYLE_ATTRIBUTE attrib;
			MashGUISkin *skin;
		};
		struct sCollectionStyle
		{
			MashStringc styleName;
			MashArray<sCollectionSkinAttrib> attributes;
		};
	public:
		MashGUIStyle():MashReferenceCounter(){}
		virtual ~MashGUIStyle(){}

		//! Returns this style name.
		/*!
			\return Style name.
		*/
		virtual const MashStringc& GetStyleName()const = 0;

		//! Returns the skin for an elements attribute.
		/*!
			\param elementType Element to query.
			\param attrib Attribute skin to return.
			\return Attribute skin.
		*/
		virtual MashGUISkin* GetAttributeSkin(int32 elementType, eGUI_STYLE_ATTRIBUTE attrib) = 0;

		//! Adds an element and attribute to this style.
		/*!
			If the combination already exists then that skin will be returned.

			\param elementType Element to add.
			\param attrib Attribute to add.
			\return The skin for the combination. 
		*/
		virtual MashGUISkin* AddAttribute(int32 elementType, eGUI_STYLE_ATTRIBUTE attrib) = 0;

		//! Sets the active element for quick searching.
		/*!
			Can be used for efficent fetching of attributes belonging to the same attribute.
			Use with GetActiveElementAttributeSkin().

			\param elementType Element to set.
			\return Failed if the element is not within this style. Ok otherwise.
		*/
		virtual eMASH_STATUS SetActiveElement(int32 elementType) = 0;

		//! Gets an attributes skin from the active element.
		/*!
			This must be called after SetActiveElement().

			\param attrib Attribute to return.
			\return Skin of the attribute.
		*/
		virtual MashGUISkin* GetActiveElementAttributeSkin(eGUI_STYLE_ATTRIBUTE attrib) = 0;

		//! Get the font assigned to this style.
		/*!
			\return Font.
		*/
		virtual MashGUIFont* GetFont()const = 0;

		//! Sets the font assigned to this style.
		/*!
			If a previous font existed then it will be dropped.

			\param font Font.
		*/
		virtual void SetFont(MashGUIFont *font) = 0;

		//! Returns this styles data for saving.
		/*!
			Used for saving to a file.
			\param output Style data will be saved here.
		*/
		virtual void CollectStyleData(MashArray<sCollectionStyle> &output) = 0;
	};
}

#endif