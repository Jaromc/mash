//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIStyle.h"
#include "MashGenericScriptReader.h"
#include "MashMemory.h"
#include "MashGUISkin.h"
#include "MashGUIFont.h"

namespace mash
{
	CMashGUIStyle::~CMashGUIStyle()
	{
		if (m_pFont)
		{
			m_pFont->Drop();
			m_pFont = 0;
		}

		std::map<int32, MashArray<sSkin> >::iterator attribIter = m_attributeSkins.begin();
		std::map<int32, MashArray<sSkin> >::iterator attribIterEnd = m_attributeSkins.end();
		for (; attribIter != attribIterEnd; ++attribIter)
		{
			MashArray<sSkin>::Iterator skinIter = attribIter->second.Begin();
			MashArray<sSkin>::Iterator skinIterEnd = attribIter->second.End();
			for(; skinIter != skinIterEnd; ++skinIter)
			{
				if (skinIter->skin)
				{
					MASH_DELETE_T(MashGUISkin, skinIter->skin);
				}
			}
		}

		m_attributeSkins.clear();
	}

	void CMashGUIStyle::SetFont(MashGUIFont *pFont)
	{
		if (pFont)
			pFont->Grab();

		if (m_pFont)
			m_pFont->Drop();

		m_pFont = pFont;
	}

	MashGUISkin* CMashGUIStyle::_GetAttributeSkin(std::map<int32, MashArray<sSkin> >::iterator iter, eGUI_STYLE_ATTRIBUTE attrib)
	{
		const uint32 attribSize = iter->second.Size();
		for(uint32 i = 0; i < attribSize; ++i)
		{
			if (iter->second[i].attrib == attrib)
			{
				/*
					Dynamically create the skin if it hasn't already been done
				*/
				if (!iter->second[i].skin)
					iter->second[i].skin = MASH_NEW_T_COMMON(MashGUISkin)();

				return iter->second[i].skin;
			}
		}

		return 0;
	}

	MashGUISkin* CMashGUIStyle::GetAttributeSkin(int32 elementType, eGUI_STYLE_ATTRIBUTE attrib)
	{
		std::map<int32, MashArray<sSkin> >::iterator iter = m_attributeSkins.find(elementType);
		if (iter != m_attributeSkins.end())
		{
			return _GetAttributeSkin(iter, attrib);
		}

		return 0;
	}

	MashGUISkin* CMashGUIStyle::AddAttribute(int32 elementType, eGUI_STYLE_ATTRIBUTE attrib)
	{
		MashGUISkin *attribSkin = GetAttributeSkin(elementType, attrib);
		if (!attribSkin)
		{
			m_attributeSkins[elementType].PushBack(sSkin(attrib));
			attribSkin = GetAttributeSkin(elementType, attrib);
		}

		return attribSkin;
	}

	void CMashGUIStyle::CollectStyleData(MashArray<sCollectionStyle> &output)
	{
		output.Resize(m_attributeSkins.size());
		uint32 currentSkinIndex = 0;

		std::map<int32, MashArray<sSkin> >::iterator iter = m_attributeSkins.begin();
		std::map<int32, MashArray<sSkin> >::iterator end = m_attributeSkins.end();
		for (; iter != end; ++iter)
		{
			output[currentSkinIndex].styleName = m_sStyleName;

			const uint32 attribSize = iter->second.Size();
			output[currentSkinIndex].attributes.Resize(attribSize);
			for(uint32 attrib = 0; attrib < attribSize; ++attrib)
			{
				output[currentSkinIndex].attributes[attrib].attrib = iter->second[attrib].attrib;
				output[currentSkinIndex].attributes[attrib].skin = iter->second[attrib].skin;
			}

			++currentSkinIndex;
		}
	}

	eMASH_STATUS CMashGUIStyle::SetActiveElement(int32 elementType)
	{
		m_activeElement = m_attributeSkins.find(elementType);
		if (m_activeElement != m_attributeSkins.end())
			return aMASH_OK;

		return aMASH_FAILED;
	}

	MashGUISkin* CMashGUIStyle::GetActiveElementAttributeSkin(eGUI_STYLE_ATTRIBUTE attrib)
	{
		if (m_activeElement != m_attributeSkins.end())
			return _GetAttributeSkin(m_activeElement, attrib);

		return 0;
	}
}