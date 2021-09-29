//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashGUITypes.h"
#include "MashGenericScriptReader.h"

namespace mash
{
	eGUI_STYLE_ELEMENT GetGUIStyleElementFromString(const MashStringc &attribString)
	{
		for(uint32 i = 0; i < aGUI_ELEMENT_UNDEFINED; ++i)
		{
			if (scriptreader::CompareStrings(attribString.GetCString(), g_guiStyleElements[i]))
				return (eGUI_STYLE_ELEMENT)i;
		}

		return aGUI_ELEMENT_UNDEFINED;
	}

	eGUI_STYLE_ATTRIBUTE GetGUIStyleAttributeFromString(const MashStringc &attribString)
	{
		for(uint32 i = 0; i < aGUI_ATTRIB_COUNT; ++i)
		{
			if (scriptreader::CompareStrings(attribString.GetCString(), g_guiStyleElementAttributes[i]))
				return (eGUI_STYLE_ATTRIBUTE)i;
		}

		return aGUI_ATTRIB_COUNT;
	}

	const int8* GetGUIStyleAttributeString(eGUI_STYLE_ATTRIBUTE attrib)
	{
		if (attrib < 0 || attrib >= aGUI_ATTRIB_COUNT)
			return 0;

		return g_guiStyleElementAttributes[attrib];
	}

	const int8* GetGUITypeAsString(eMASH_GUI_TYPE type)
	{
		if (type >= 0 && type < aGUI_TYPE_COUNT)
			return g_guiTypeStrings[type];

		return 0;
	}

	eMASH_GUI_TYPE GetGUITypeFromString(const MashStringc &type)
	{
		for(uint32 i = 0; i < aGUI_TYPE_COUNT; ++i)
		{
			if (scriptreader::CompareStrings(type.GetCString(), g_guiTypeStrings[i]))
				return (eMASH_GUI_TYPE)i;
		}

		return aGUI_TYPE_COUNT;
	}

	eGUI_TEXT_FORMAT GetGUITextFormatFromString(const MashStringc &type)
	{
		for(uint32 i = 0; i < aGUI_TEXT_FORMAT_COUNT; ++i)
		{
			if (scriptreader::CompareStrings(type.GetCString(), g_guiTextFormatStrings[i]))
				return (eGUI_TEXT_FORMAT)i;
		}

		return aGUI_TEXT_FORMAT_COUNT;
	}

	const int8* GetGUITextFormatAsString(eGUI_TEXT_FORMAT type)
	{
		if ((type >= 0) && (type < aGUI_TEXT_FORMAT_COUNT))
			return g_guiTextFormatStrings[type];

		return 0;
	}
}