//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_TYPES_H_
#define _MASH_GUI_TYPES_H_

#include "MashString.h"
#include "MashDataTypes.h"
#include "MashEnum.h"
#include "MashFunctor.h"

namespace mash
{
	class MashGUIEventDispatch;

	struct sGUIOverrideTransparency
	{
		f32 alphaMaskThreshold;
		uint8 alphaValue;
		bool affectFontAlpha;
		bool enableOverrideTransparency;

		sGUIOverrideTransparency():alphaMaskThreshold(0.0f),
			alphaValue(255),
			affectFontAlpha(true),
			enableOverrideTransparency(false){}

		bool IsTransparent()
		{
			return (alphaValue < 255);
		}

		bool operator!=(const sGUIOverrideTransparency &other)const
		{
			return ((alphaMaskThreshold != other.alphaMaskThreshold) ||
				(alphaValue != other.alphaValue) ||
				(affectFontAlpha != other.affectFontAlpha) ||
				(enableOverrideTransparency != other.enableOverrideTransparency));
		}

		bool operator==(const sGUIOverrideTransparency &other)const
		{
			return ((alphaMaskThreshold == other.alphaMaskThreshold) &&
				(alphaValue == other.alphaValue) &&
				(affectFontAlpha == other.affectFontAlpha) &&
				(enableOverrideTransparency == other.enableOverrideTransparency));
		}
	};

	enum eMASH_GUI_TYPE
	{
		aGUI_SPRITE,
		aGUI_BUTTON,
		aGUI_CHECK_BOX,
		aGUI_WINDOW,
		aGUI_SCROLL_BAR,
		aGUI_TAB_CONTROL,
		aGUI_STATIC_TEXT,
		aGUI_TEXT_BOX,
		aGUI_LISTBOX,
		aGUI_VIEW,
		aGUI_SCROLL_BAR_VIEW,

		aGUI_MENUBAR,
		aGUI_POPUP_MENU,
		aGUI_TREE,

		aGUI_VIEWPORT,

		aGUI_TYPE_COUNT
	};

	static const int8 *const g_guiTypeStrings[] = {
		"sprite",
		"button",
		"checkbox",
		"window",
		"scrollbar",
		"tabcontrol",
		"statictext",
		"textbox",
		"listbox",
		"view",
		"scrollbarview",
		"menubar",
		"popup",
		"tree",
		"viewport",
		0
	};

	static const int8 *const g_guiStyleElements[] = {
		"button",
		"checkbox",
		"listbox",
		"menubar",
		"popup",
		"sprite",
		"tab",
		"tree",
		"window",
		"scrollbar",
		"textbox",
		"statictextbox",
		"filedialog",
		0
	};

	/*
		Note, windows and views are considered the same
		in styles.
	*/
	enum eGUI_STYLE_ELEMENT
	{
		aGUI_ELEMENT_BUTTON,
		aGUI_ELEMENT_CHECKBOX,
		aGUI_ELEMENT_LISTBOX,
		aGUI_ELEMENT_MENUBAR,
		aGUI_ELEMENT_POPUP,
		aGUI_ELEMENT_SPRITE,
		aGUI_ELEMENT_TAB,
		aGUI_ELEMENT_TREE,
		aGUI_ELEMENT_WINDOW,
		aGUI_ELEMENT_SCROLLBAR,
		aGUI_ELEMENT_TEXTBOX,
		aGUI_ELEMENT_STATIC_TEXTBOX,
		aGUI_ELEMENT_FILE_DIALOG,

		aGUI_ELEMENT_UNDEFINED
	};

	static const int8 *const g_guiStyleElementAttributes[] = {
		"buttonup",
		"buttondown",
		"buttonhover",

		"checkboxon",
		"checkboxoff",

		"increment",
		"decrement",

		"uparrowup",
		"uparrowdown",
		"uparrowhover",
		"downarrowup",
		"downarrowdown",
		"downarrowhover",
		"leftarrowup",
		"leftarrowdown",
		"leftarrowhover",
		"rightarrowup",
		"rightarrowdown",
		"rightarrowhover",
		"sliderup",
		"sliderdown",
		"sliderhover",
		"sliderbackground",
		"endcap",

		"tabactive",
		"tabinactive",
		"tabhover",

		"closeup",
		"closedown",
		"closehover",

		"minimizeup",
		"minimizedown",
		"minimizehover",

		"itemactive",
		"iteminactive",
		"background",
		"titlebar",

		"file",
		"folder",
		0
	};

	enum eGUI_STYLE_ATTRIBUTE
	{
		aGUI_ATTRIB_BUTTON_UP,
		aGUI_ATTRIB_BUTTON_DOWN,
		aGUI_ATTRIB_BUTTON_HOVER,

		aGUI_ATTRIB_CHECKBOX_ON,
		aGUI_ATTRIB_CHECKBOX_OFF,

		aGUI_ATTRIB_INCREMENT,
		aGUI_ATTRIB_DECREMENT,

		aGUI_ATTRIB_SCROLLBAR_ARROWUP_UP,
		aGUI_ATTRIB_SCROLLBAR_ARROWUP_DOWN,
		aGUI_ATTRIB_SCROLLBAR_ARROWUP_HOVER,

		aGUI_ATTRIB_SCROLLBAR_ARROWDOWN_UP,
		aGUI_ATTRIB_SCROLLBAR_ARROWDOWN_DOWN,
		aGUI_ATTRIB_SCROLLBAR_ARROWDOWN_HOVER,

		aGUI_ATTRIB_SCROLLBAR_ARROWLEFT_UP,
		aGUI_ATTRIB_SCROLLBAR_ARROWLEFT_DOWN,
		aGUI_ATTRIB_SCROLLBAR_ARROWLEFT_HOVER,

		aGUI_ATTRIB_SCROLLBAR_ARROWRIGHT_UP,
		aGUI_ATTRIB_SCROLLBAR_ARROWRIGHT_DOWN,
		aGUI_ATTRIB_SCROLLBAR_ARROWRIGHT_HOVER,

		aGUI_ATTRIB_SCROLLBAR_SLIDER_UP,
		aGUI_ATTRIB_SCROLLBAR_SLIDER_DOWN,
		aGUI_ATTRIB_SCROLLBAR_SLIDER_HOVER,

		aGUI_ATTRIB_SCROLLBAR_BACKGROUND,
		aGUI_ATTRIB_SCROLLBAR_END_CAP,

		aGUI_ATTRIB_TAB_ACTIVE,
		aGUI_ATTRIB_TAB_INACTIVE,
		aGUI_ATTRIB_TAB_HOVER,

		aGUI_ATTRIB_WINDOW_CLOSE_UP,
		aGUI_ATTRIB_WINDOW_CLOSE_DOWN,
		aGUI_ATTRIB_WINDOW_CLOSE_HOVER,

		aGUI_ATTRIB_WINDOW_MINIMIZE_UP,
		aGUI_ATTRIB_WINDOW_MINIMIZE_DOWN,
		aGUI_ATTRIB_WINDOW_MINIMIZE_HOVER,

		aGUI_ATTRIB_ITEM_ACTIVE,
		aGUI_ATTRIB_ITEM_INACTIVE,
		aGUI_ATTRIB_BACKGROUND,
		aGUI_ATTRIB_TITLEBAR,

		aGUI_ATTRIB_FILE_ICON,
		aGUI_ATTRIB_FOLDER_ICON,

		aGUI_ATTRIB_COUNT
	};

	static const int8 *const g_guiTextFormatStrings[] = {
		"any",
		"int",
		"float",
		0
	};

	enum eGUI_TEXT_FORMAT
	{
		aGUI_TEXT_FORMAT_ANY,
		aGUI_TEXT_FORMAT_INT,
		aGUI_TEXT_FORMAT_FLOAT,

		aGUI_TEXT_FORMAT_COUNT
	};

	struct sGUIEvent
	{
		eGUI_EVENT GUIEvent;
		MashGUIEventDispatch *component;
		f32 value;
	};

	typedef MashFunctor<const sGUIEvent> MashGUIEventFunctor;

	eGUI_STYLE_ELEMENT GetGUIStyleElementFromString(const MashStringc &attribString);

	//helper function
	eGUI_STYLE_ATTRIBUTE GetGUIStyleAttributeFromString(const MashStringc &attribString);
	const int8* GetGUIStyleAttributeString(eGUI_STYLE_ATTRIBUTE attrib);

	//Helper function
	const int8* GetGUITypeAsString(eMASH_GUI_TYPE type);
	eMASH_GUI_TYPE  GetGUITypeFromString(const MashStringc &type);

	eGUI_TEXT_FORMAT GetGUITextFormatFromString(const MashStringc &type);
	const int8* GetGUITextFormatAsString(eGUI_TEXT_FORMAT type);
}

#endif