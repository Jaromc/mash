//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_SPRITE_H_
#define _C_MASH_GUI_SPRITE_H_

#include "MashGUISprite.h"
#include "MashGUISkin.h"

namespace mash
{
	class CMashGUISprite : public MashGUISprite
	{
	private:
		f32 m_fRotation;
		int32 m_styleElement;

		MashGUISkin m_skin;
		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0){}
	public:
		CMashGUISprite(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement);

		~CMashGUISprite();

		void Draw();
		f32 GetRotation()const;
		void SetRotation(f32 fRadians);

		void OnStyleChange(MashGUIStyle *style){}

		MashGUISkin* GetSkin();

		eMASH_GUI_TYPE GetGUIType()const;
	};

	inline MashGUISkin* CMashGUISprite::GetSkin()	
	{
		return &m_skin;
	}

	//inline MashGUISkin* CMashGUISprite::GetBackgroundSkin()
	//{
	//	return &m_backgroundSkin;
	//}

	inline eMASH_GUI_TYPE CMashGUISprite::GetGUIType()const
	{
		return aGUI_SPRITE;
	}

	inline void CMashGUISprite::SetRotation(f32 fRadians)
	{
		m_fRotation = fRadians;
	}

	inline f32 CMashGUISprite::GetRotation()const
	{
		return m_fRotation;
	}
}

#endif