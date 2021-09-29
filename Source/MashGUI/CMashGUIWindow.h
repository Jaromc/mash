//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_WINDOW_H_
#define _C_MASH_GUI_WINDOW_H_

#include "MashGUIWindow.h"
#include "CMashTextHelper.h"
namespace mash
{
	class CMashGUIWindow : public MashGUIWindow
	{
	private:
		enum eWINDOW_BUTTONS
		{
			aWIN_BTN_CLOSE,
			aWIN_BTN_MINIMIZE,
			aWIN_BTN_NONE
		};

		enum eBUTTON_STATE
		{
			aSTATE_UP,
			aSTATE_DOWN
		};
	private:
		mash::MashRectangle2 m_titleBarClippedAbs;
		mash::MashRectangle2 m_titleBarAbs;

		mash::MashRectangle2 m_closeButtonAbs;
		mash::MashRectangle2 m_closeButtonClippedAbs;

		mash::MashRectangle2 m_minimizeButtonAbs;
		mash::MashRectangle2 m_minimizeButtonClippedAbs;

		bool m_isMinimized;

		f32 m_menuBarHeight;
		bool m_titleBarCulled;
		bool m_closeButtonCulled;
		bool m_minimizeButtonCulled;

		bool m_activateCloseButton;
		bool m_activateMinimizeButton;
		f32 m_buttonHeight;

		bool m_allowMouseDrag;
		bool m_mouseDragEnabled;

		eWINDOW_BUTTONS m_hoverButton;
		eBUTTON_STATE m_buttonState;

		int32 m_styleElement;

		eCLOSE_BUTTON_EVENT m_closeButtonEvent;

		CMashTextHelper m_textHandler;
		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
		MashGUIView *m_view;

		void UpdateTitleBarRegion();
		void UpdateCloseButtonRegion();
		void UpdateMinimizeButtonRegion();
		void OnMouseExit(const mash::MashVector2 &vScreenPos);
		void OnLostFocus();
		void OnTransparencyChange();
	public:
		CMashGUIWindow(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement);

		~CMashGUIWindow();

		void AddChild(MashGUIComponent *pChild);

		MashGUIComponent* GetElementByName(const MashStringc &name, bool searchChildren = true);

		void SetOverrideTransparency(bool state, uint8 alpha, bool affectFont = true, f32 alphaMaskThreshold = 0.0f);

		void SetTitleBarText(const MashStringc &text);
		const MashStringc& GetTitleBarText()const;

		void EnableCloseButton(bool enable);
		void EnableMinimizeButton(bool enable);

		bool GetCloseButtonEnabled()const;
		bool GetMinimizeButtonEnabled()const;

		void EnableMouseDrag(bool state);
		bool GetMouseDragEnabled()const;

		void SetCloseButtonEvent(eCLOSE_BUTTON_EVENT e);
		eCLOSE_BUTTON_EVENT GetCloseButtonEvent()const;

		MashGUIView* GetView();
		void OnEvent(const sInputEvent &eventData);

		eMASH_GUI_TYPE GetGUIType()const;
		void OnStyleChange(MashGUIStyle *style);
		
		void Draw();
		MashGUIComponent* GetClosestIntersectingChild(const mash::MashVector2 &vScreenPos, bool bTestAllChildren = false);
		MashGUIView* GetClosestParentableObject(const mash::MashVector2 &screenPosition);

		void SetLockFocusWhenActivated(bool lockFocus);
		void SetAlwaysOnTop(bool enable);
        
        void SetMinimizeState(bool enable);
        void CloseWindow();
	};

	inline void CMashGUIWindow::SetCloseButtonEvent(MashGUIWindow::eCLOSE_BUTTON_EVENT e)
	{
		m_closeButtonEvent = e;
	}

	inline MashGUIWindow::eCLOSE_BUTTON_EVENT CMashGUIWindow::GetCloseButtonEvent()const
	{
		return m_closeButtonEvent;
	}

	inline bool CMashGUIWindow::GetCloseButtonEnabled()const
	{
		return m_activateCloseButton;
	}

	inline bool CMashGUIWindow::GetMinimizeButtonEnabled()const
	{
		return m_activateMinimizeButton;
	}

	inline void CMashGUIWindow::EnableMouseDrag(bool state)
	{
		m_allowMouseDrag = state;
	}

	inline bool CMashGUIWindow::GetMouseDragEnabled()const
	{
		return m_allowMouseDrag;
	}

	inline const MashStringc& CMashGUIWindow::GetTitleBarText()const
	{
		return m_textHandler.GetString();
	}

	inline MashGUIView* CMashGUIWindow::GetView()
	{
		return m_view;
	}

	inline eMASH_GUI_TYPE CMashGUIWindow::GetGUIType()const
	{
		return aGUI_WINDOW;
	}
}

#endif