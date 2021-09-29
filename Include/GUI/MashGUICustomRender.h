//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_CUSTOM_RENDER_H_
#define _MASH_GUI_CUSTOM_RENDER_H_

#include "MashReferenceCounter.h"

namespace mash
{
	class MashGUIComponent;

	/*!
		This can be used, for example, for drawing a grid in a view, or any other custom object.
	*/
	class MashGUICustomRender : public MashReferenceCounter
	{
	public:
		MashGUICustomRender(){}
		virtual ~MashGUICustomRender(){}

		//! Implement a custom draw function.
		/*!
			\param component The component that owns this renderer.
		*/
		virtual void Draw(MashGUIComponent *component) = 0;
	};
}

#endif