//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_STYLE_H_
#define _C_MASH_GUI_STYLE_H_

#include "MashGUIStyle.h"
#include "MashGUISkin.h"
#include <map>

namespace mash
{
	class CMashGUIStyle : public MashGUIStyle
	{
	private:
		struct sSkin
		{
			eGUI_STYLE_ATTRIBUTE attrib;
			MashGUISkin *skin;

			sSkin(){}
			sSkin(eGUI_STYLE_ATTRIBUTE _attrib):attrib(_attrib), skin(0){}
		};	
	public:
		std::map<int32, MashArray<sSkin> > m_attributeSkins;
		MashStringc m_sStyleName;
		MashGUIFont *m_pFont;

		//for use in GetActiveElementAttributeSkin().TODO : Change to a vector?
		std::map<int32, MashArray<sSkin> >::iterator m_activeElement;

		MashGUISkin* _GetAttributeSkin(std::map<int32, MashArray<sSkin> >::iterator iter, eGUI_STYLE_ATTRIBUTE attrib);
	public:
		CMashGUIStyle(const MashStringc &name)
		{
			m_sStyleName = name;
			m_pFont = 0;
		}
		~CMashGUIStyle();

		void SetStyleName(const MashStringc &name);
		const MashStringc& GetStyleName()const;
		MashGUISkin* GetAttributeSkin(int32 elementType, eGUI_STYLE_ATTRIBUTE attrib);
		MashGUISkin* AddAttribute(int32 elementType, eGUI_STYLE_ATTRIBUTE attrib);

		//can be used for efficent fetching of attributes belonging to the same attribute
		eMASH_STATUS SetActiveElement(int32 elementType);
		MashGUISkin* GetActiveElementAttributeSkin(eGUI_STYLE_ATTRIBUTE attrib);

		MashGUIFont* GetFont()const;
		void SetFont(MashGUIFont *pFont);

		void CollectStyleData(MashArray<sCollectionStyle> &output);
	};

	inline MashGUIFont* CMashGUIStyle::GetFont()const
	{
		return m_pFont;
	}

	inline void CMashGUIStyle::SetStyleName(const MashStringc &name)
	{
		m_sStyleName = name;
	}

	inline const MashStringc& CMashGUIStyle::GetStyleName()const
	{
		return m_sStyleName;
	}
}

#endif