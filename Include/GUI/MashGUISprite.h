//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_SPRITE_H_
#define _MASH_GUI_SPRITE_H_

#include "MashGUIComponent.h"

namespace mash
{
	class MashGUISprite : public MashGUIComponent
	{
	public:
		MashGUISprite(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination):MashGUIComponent(pGUIManager, pInputManager, pParent, destination){}

		virtual ~MashGUISprite(){}

		//! Gets the rotation amount.
		/*!
			\return Rotation in radians.
		*/
		virtual f32 GetRotation()const = 0;

		//! Sets the rotation.
		/*!
			\param rotation Rotation in radians.
		*/
		virtual void SetRotation(f32 rotation) = 0;

		//! Gets the current skin.
		/*!
			This skin can be manipulated to change how this sprite looks.
			\return Sprite skin.
		*/
		virtual MashGUISkin* GetSkin() = 0;
	};
}

#endif