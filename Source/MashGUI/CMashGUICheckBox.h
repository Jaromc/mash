//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_CHECKBOX_H_
#define _C_MASH_GUI_CHECKBOX_H_

#include "MashGUICheckBox.h"
#include "MashGUIFont.h"

namespace mash
{
	class CMashGUICheckBox : public MashGUICheckBox
	{
	private:
		bool m_bIsChecked;
		int32 m_styleElement;

		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0){}
	public:
		CMashGUICheckBox(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement);

		virtual ~CMashGUICheckBox();

		void OnEvent(const sInputEvent &eventData);

		bool IsChecked()const;
		void SetChecked(bool bIsChecked);

		void OnStyleChange(MashGUIStyle *style){}

		eMASH_GUI_TYPE GetGUIType()const;

		void Draw();
	};

	inline eMASH_GUI_TYPE CMashGUICheckBox::GetGUIType()const
	{
		return aGUI_CHECK_BOX;
	}

	inline bool CMashGUICheckBox::IsChecked()const
	{
		return m_bIsChecked;
	}
}

#endif