//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_WIN_DEVICE_H_
#define _C_MASH_WIN_DEVICE_H_

#include "MashDataTypes.h"
#ifdef MASH_WINDOWS

#include "CMashDevice.h"

#include <windows.h>
#include <oleauto.h>
#include <wbemidl.h>
#include <XInput.h>
namespace mash
{
	const uint32 g_maxKeyboardChars = 256;
	const uint32 g_joystickButtonCount = 9;
	const uint32 g_xInputButtonCount = 14;
	class CMashWinDevice : public CMashDevice
	{
	public:
		static uint8 m_keyMap[g_maxKeyboardChars];
		static BYTE m_currentKeyState[256];
		static BYTE m_keyState[256];
		
		uint32 m_keyboardMouseControllerID;

		uint32 m_screenWidth;
		uint32 m_screenHeight;
	private:
		struct sJoystickInfo
		{
			union
			{
				JOYINFOEX legacyController;
				XINPUT_STATE xboxController;
			};

			uint32 windowsID;
			int32 controllerID;
			bool isXInput;
			bool isConnected;

			f32 maxAxis1;
			f32 maxAxis2;
			f32 maxAxisT1;
			f32 maxAxisT2;

			uint32 legacyPOVState;
		};

		struct sWinJoystickKeyMap
		{
			uint32 joyButton;
			eINPUT_EVENT action;
		};
		sWinJoystickKeyMap m_winJoystickMap[g_joystickButtonCount];
		sWinJoystickKeyMap m_xInputMap[g_xInputButtonCount];

		//! Window handle
		HWND m_hWnd;
		HINSTANCE m_hInstance;
		bool m_cageMouse;
		bool m_mouseCursorHidden;

		MashArray<sJoystickInfo> m_joystickInfo;
		
		void BeginUpdate();
		uint32 CalculateLegacyPOVState(uint32 POVState);
	protected:

		bool PollMessages();
		void SyncInputDeviceWithCurrentState();
	public:
		CMashWinDevice(const MashStringc &debugFilePath);
		~CMashWinDevice();
		
		void* GetHwnd()const;
		eMASH_STATUS Initialise(const mash::sMashDeviceSettings &settings);
		void SetWindowCaption(const int8 *text);
		void GetWindowSize(uint32 &x, uint32 &y)const;
		void _Draw(){};//nothing to do here
		void Sleep(uint32 ms)const;

		void LockMouseToScreenCenter(bool state);
		bool IsMouseLockedToScreenCenter()const;

		void HideMouseCursor(bool state);
		bool IsMouseCursorHidden()const;

		eMASH_DEVICE_TYPE GetDeviceType()const;

		eMASH_STATUS LoadComponents(const mash::sMashDeviceSettings &settings);
	};

	inline bool CMashWinDevice::IsMouseCursorHidden()const
	{
		return m_mouseCursorHidden;
	}

	inline bool CMashWinDevice::IsMouseLockedToScreenCenter()const
	{
		return m_cageMouse;
	}

	inline void CMashWinDevice::GetWindowSize(uint32 &x, uint32 &y)const
	{
		x = m_screenWidth;
		y = m_screenHeight;
	}

	inline eMASH_DEVICE_TYPE CMashWinDevice::GetDeviceType()const
	{
		return aDEVICE_TYPE_WINDOWS;
	}

	inline void* CMashWinDevice::GetHwnd()const
	{
		return m_hWnd;
	}
}

#endif

#endif
