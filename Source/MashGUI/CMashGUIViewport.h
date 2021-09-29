//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_VIEWPORT_H_
#define _C_MASH_GUI_VIEWPORT_H_

#include "MashGUIViewport.h"
#include "MashVideo.h"

namespace mash
{
	class CMashGUIViewport : public MashGUIViewport
	{
	private:
		mash::MashVideo *m_renderer;
		sMashViewPort m_originalViewport;
		void OnFocusGained();

		MashInputEventFunctor m_inputEventCallback;

		void OnStyleChange(MashGUIStyle *style){}
		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0){}
	public:
		CMashGUIViewport(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			mash::MashVideo *renderer);

		~CMashGUIViewport();

		void GetViewport(sMashViewPort &out);
		void SetInputEventCallback(const MashInputEventFunctor &callback);
		void OnEvent(const sInputEvent &eventData);
		void GetViewportWidthHeight(f32 &x, f32 &y)const;
		void SetViewport();
		void RestoreOriginalViewport();
		void Draw();
		eMASH_GUI_TYPE GetGUIType()const;
	};

	inline eMASH_GUI_TYPE CMashGUIViewport::GetGUIType()const
	{
		return aGUI_VIEWPORT;
	}
}

#endif