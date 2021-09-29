//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashWinDevice.h"
#include "MashDataTypes.h"

#ifdef MASH_WINDOWS

#include "MashVideo.h"

#include "MashEventTypes.h"
#include "CMashInputManager.h"
#include "MashGUIManager.h"
#include "MashRectangle2.h"
#include "MashLog.h"
#include "MashMathHelper.h"
#include "CMashMemoryTracker.h"
#include "MashStringHelper.h"

namespace mash
{
	static CMashWinDevice *g_pMash_Device = 0;
	BYTE *g_pInputBufferArray = 0;
	int32 g_iInputBufferSize = 0;

	uint8 CMashWinDevice::m_keyMap[];
	BYTE CMashWinDevice::m_keyState[];
	BYTE CMashWinDevice::m_currentKeyState[];
	int32 g_lastMousePosX = 0;
	int32 g_lastMousePosY = 0;
	mash::MashRectangle2 g_windowSize;
	bool g_windowWasResized = false;
	bool g_windowWasMinOrMaximized = false;
	bool g_windowsInitComplete = false;

#define SAFE_DELETE(p)  { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

	CMashWinDevice::CMashWinDevice(const MashStringc &debugFilePath):CMashDevice(debugFilePath),
		m_hWnd(0), m_cageMouse(false), m_screenWidth(0), m_screenHeight(0), m_keyboardMouseControllerID(0), m_mouseCursorHidden(false), m_hInstance(0)
	{
		g_pMash_Device = this;

		memset(m_keyState, 0, sizeof(m_keyState));
		memset(m_currentKeyState, 0, sizeof(m_currentKeyState));
	}

	CMashWinDevice::~CMashWinDevice()
	{
		LockMouseToScreenCenter(false);
	}

	void MashResolutionChange()
	{
		if (g_pMash_Device->GetRenderer())
			g_pMash_Device->GetRenderer()->OnResolutionChange(g_pMash_Device->m_screenWidth,g_pMash_Device->m_screenHeight);

		if (g_pMash_Device->IsMouseLockedToScreenCenter())
		{
			//reset the screen region
			g_pMash_Device->LockMouseToScreenCenter(false);
			g_pMash_Device->LockMouseToScreenCenter(true);
		}

		if (g_pMash_Device->GetGUIManager())
			g_pMash_Device->GetGUIManager()->OnResize();
	}

	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		PAINTSTRUCT ps;
		HDC hdc;
		switch (message) 
		{
			case WM_PAINT:
				{
					hdc = BeginPaint(hWnd, &ps);
					RECT rt;
					GetClientRect(hWnd, &rt);
					EndPaint(hWnd, &ps);
					break;
				}
			case WM_DESTROY:
				{
				PostQuitMessage(0);
				return 0;
				}
			case WM_EXITSIZEMOVE:
				{
					//this message waits for the WM_SIZE message.
					//WM_SIZE will be called for every drag movement which means if we update when that is called
					//there will be a very high number of d3d rebuilds. WM_EXITSIZEMOVE is called when resizing by mouse is complete.
					if (g_pMash_Device && g_windowWasResized)
					{
						MashResolutionChange();
						g_windowWasResized = false;
					}
					break;
				}
			case WM_SIZE:
				{
					if (g_pMash_Device)
					{
						g_pMash_Device->m_screenWidth = LOWORD(lParam);
						g_pMash_Device->m_screenHeight = HIWORD(lParam);

						/*
							trigger update flags so that we dont rebuild video related things while the
							mouse is dragging.
						*/
						//resize on min and maximize
						if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_MINIMIZED))
						{
							MashResolutionChange();
							g_windowWasMinOrMaximized = true;
						}
						/*
							A restored message is sent when returning from min/maximize and for
							every mouse drag. We only want to update from the former.
						*/
						else if (g_windowWasMinOrMaximized && (wParam == SIZE_RESTORED))
						{
							MashResolutionChange();
							g_windowWasMinOrMaximized = false;
						}
						/*
							Assume the user is dragging the window resize tab. Rebuild is delayed until
							the WM_EXITSIZEMOVE msg is received.

							WM_SIZE is sent when the window is created. g_windowsInitComplete just suppresses
							the message until after the init.
						*/
						else if (g_windowsInitComplete)
							g_windowWasResized = true;
					}

					return 0;
				}
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
				{
					BYTE asciiChar[2];
					GetKeyboardState(CMashWinDevice::m_currentKeyState);

					//reset virtual keys we dont care about with keyboard messages.
					//This can affect repeating messages.
					CMashWinDevice::m_currentKeyState[VK_LBUTTON] = 0;
					CMashWinDevice::m_currentKeyState[VK_RBUTTON] = 0;
					CMashWinDevice::m_currentKeyState[VK_MBUTTON] = 0;
					CMashWinDevice::m_currentKeyState[VK_XBUTTON1] = 0;
					CMashWinDevice::m_currentKeyState[VK_XBUTTON2] = 0;

					//stop key repeating
					if (memcmp(CMashWinDevice::m_currentKeyState, CMashWinDevice::m_keyState, 256) != 0)
					{
						sInputEvent newEvent;
						newEvent.character = 0;
						newEvent.action = aKEYEVENT_NONE;
						newEvent.controllerID = g_pMash_Device->m_keyboardMouseControllerID;
						newEvent.eventType = mash::sInputEvent::aEVENTTYPE_KEYBOARD;

						if (message == WM_KEYUP)
						{
							newEvent.value = 0.0f;
							newEvent.isPressed = false;
						}
						else
						{
							newEvent.value = 1.0f;
							newEvent.isPressed = true;
						}

						//first check mapped virtual keys
						int8 keyMapChar = CMashWinDevice::m_keyMap[wParam];
						if ((wParam <= 255) && keyMapChar)
						{
							newEvent.character = 0;
							newEvent.action = (eINPUT_EVENT)keyMapChar;
						}
						//next check ascii keys
						else
						{
							if (ToAscii(wParam, lParam, CMashWinDevice::m_currentKeyState, (LPWORD)asciiChar, 0))
							{
								if (asciiChar[0] >= 0x20 && asciiChar[0] <= 0x7E)
								{
									newEvent.character = asciiChar[0];
									newEvent.action = (eINPUT_EVENT)toupper(asciiChar[0]);
								}
							}
						}

						if (newEvent.action != aKEYEVENT_NONE)
						{
							g_pMash_Device->GetInputManager()->ImmediateBroadcast(newEvent);
						}

						memcpy(CMashWinDevice::m_keyState, CMashWinDevice::m_currentKeyState, sizeof(CMashWinDevice::m_currentKeyState));
					}

					//pass on alt msgs
					//if (CMashWinDevice::m_currentKeyState[VK_MENU] & 0x80)
					//	break;
					//else
					return 0;
				}
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
				{
					SetCapture((HWND)g_pMash_Device->GetHwnd());

					sInputEvent newEvent;
					newEvent.eventType = mash::sInputEvent::aEVENTTYPE_MOUSE;
					newEvent.controllerID = g_pMash_Device->m_keyboardMouseControllerID;

					tagPOINT screenPos;
					screenPos.x = 0;
					screenPos.y = 0;
					ClientToScreen((HWND)g_pMash_Device->GetHwnd(), &screenPos);

					tagPOINT p;
					GetCursorPos(&p);
					int32 ix = p.x - screenPos.x;
					int32 iy = p.y - screenPos.y;

					if (ix != g_lastMousePosX)
					{
						newEvent.action = aMOUSEEVENT_AXISX;
						newEvent.value = ix - g_lastMousePosX;
						g_pMash_Device->GetInputManager()->ImmediateBroadcast(newEvent);
						g_lastMousePosX = ix;
					}
					if (iy != g_lastMousePosY)
					{
						newEvent.action = aMOUSEEVENT_AXISY;
						newEvent.value = iy - g_lastMousePosY;
						g_pMash_Device->GetInputManager()->ImmediateBroadcast(newEvent);
						g_lastMousePosY = iy;
					}

					switch(message)
					{
					case WM_LBUTTONDOWN:
						newEvent.action = aMOUSEEVENT_B1;
						break;
					case WM_RBUTTONDOWN:
						newEvent.action = aMOUSEEVENT_B2;
						break;
					case WM_MBUTTONDOWN:
						newEvent.action = aMOUSEEVENT_B3;
						break;
					};

					newEvent.isPressed = 1;
					newEvent.value = 1.0f;

					g_pMash_Device->GetInputManager()->ImmediateBroadcast(newEvent);

					return 0;
				}
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
				{
					ReleaseCapture();

					sInputEvent newEvent;
					newEvent.eventType = mash::sInputEvent::aEVENTTYPE_MOUSE;
					newEvent.controllerID = g_pMash_Device->m_keyboardMouseControllerID;

					tagPOINT screenPos;
					screenPos.x = 0;
					screenPos.y = 0;
					ClientToScreen((HWND)g_pMash_Device->GetHwnd(), &screenPos);

					tagPOINT p;
					GetCursorPos(&p);
					int32 ix = p.x - screenPos.x;
					int32 iy = p.y - screenPos.y;

					if (ix != g_lastMousePosX)
					{
						newEvent.action = aMOUSEEVENT_AXISX;
						newEvent.value = ix - g_lastMousePosX;
						g_pMash_Device->GetInputManager()->ImmediateBroadcast(newEvent);
						g_lastMousePosX = ix;
					}
					if (iy != g_lastMousePosY)
					{
						newEvent.action = aMOUSEEVENT_AXISY;
						newEvent.value = iy - g_lastMousePosY;
						g_pMash_Device->GetInputManager()->ImmediateBroadcast(newEvent);
						g_lastMousePosY = iy;
					}

					switch(message)
					{
					case WM_LBUTTONUP:
						newEvent.action = aMOUSEEVENT_B1;
						break;
					case WM_RBUTTONUP:
						newEvent.action = aMOUSEEVENT_B2;
						break;
					case WM_MBUTTONUP:
						newEvent.action = aMOUSEEVENT_B3;
						break;
					};

					newEvent.value = 0.0f;
					newEvent.isPressed = 0;
					g_pMash_Device->GetInputManager()->ImmediateBroadcast(newEvent);

					return 0;
				}
			case WM_MOUSEWHEEL:
				{
					sInputEvent newEvent;
					newEvent.eventType = mash::sInputEvent::aEVENTTYPE_MOUSE;
					newEvent.controllerID = g_pMash_Device->m_keyboardMouseControllerID;
					
					tagPOINT mousePos;
					GetCursorPos(&mousePos);
					tagPOINT screenPos;
					screenPos.x = 0;
					screenPos.y = 0;
					ClientToScreen((HWND)g_pMash_Device->GetHwnd(), &screenPos);

					int32 ix = mousePos.x - screenPos.x;
					int32 iy = mousePos.y - screenPos.y;

					if (ix != g_lastMousePosX)
					{
						newEvent.action = aMOUSEEVENT_AXISX;
						newEvent.value = ix - g_lastMousePosX;
						g_pMash_Device->GetInputManager()->ImmediateBroadcast(newEvent);
						g_lastMousePosX = ix;
					}
					if (iy != g_lastMousePosY)
					{
						newEvent.action = aMOUSEEVENT_AXISY;
						newEvent.value = iy - g_lastMousePosY;
						g_pMash_Device->GetInputManager()->ImmediateBroadcast(newEvent);
						g_lastMousePosY = iy;
					}

					newEvent.action = aMOUSEEVENT_AXISZ;
					if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
						newEvent.value = -1.0f;
					else
						newEvent.value = 1.0f;

					g_pMash_Device->GetInputManager()->ImmediateBroadcast(newEvent);
					return 0;
				}
			case WM_MOUSEMOVE:
				{
					tagPOINT screenPos;
					screenPos.x = 0;
					screenPos.y = 0;
					ClientToScreen((HWND)g_pMash_Device->GetHwnd(), &screenPos);

					tagPOINT p;
					GetCursorPos(&p);
					int32 ix = p.x - screenPos.x;
					int32 iy = p.y - screenPos.y;

					if (ix != g_lastMousePosX)
					{
						sInputEvent newEvent;
						newEvent.eventType = mash::sInputEvent::aEVENTTYPE_MOUSE;
						newEvent.controllerID = g_pMash_Device->m_keyboardMouseControllerID;
						newEvent.action = aMOUSEEVENT_AXISX;
						newEvent.value = ix - g_lastMousePosX;
						g_pMash_Device->GetInputManager()->ImmediateBroadcast(newEvent);
						g_lastMousePosX = ix;
					}
					if (iy != g_lastMousePosY)
					{
						sInputEvent newEvent;
						newEvent.eventType = mash::sInputEvent::aEVENTTYPE_MOUSE;
						newEvent.controllerID = g_pMash_Device->m_keyboardMouseControllerID;
						newEvent.action = aMOUSEEVENT_AXISY;
						newEvent.value = iy - g_lastMousePosY;
						g_pMash_Device->GetInputManager()->ImmediateBroadcast(newEvent);
						g_lastMousePosY = iy;
					}

					break;
				}
			case WM_SYSCOMMAND:
				{
					//stop screemashver or monitor powersave mode from starting
					if ((wParam & 0xFFF0) == SC_SCREEMASHVE ||
						(wParam & 0xFFF0) == SC_MONITORPOWER)
					{
						return 0;
					}
				}
	   }

		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	void CMashWinDevice::SyncInputDeviceWithCurrentState()
	{
		tagPOINT p;
		LPPOINT mousePos = &p;
		GetCursorPos(mousePos);

		// Call this function to convert to client coords
		tagPOINT screenPos;
		screenPos.x = 0;
		screenPos.y = 0;
		ScreenToClient( m_hWnd, &screenPos );

		g_lastMousePosX = (int32)screenPos.x + (int32)mousePos->x;
		g_lastMousePosY = (int32)screenPos.y + (int32)mousePos->y;
		m_pInputManager->_SetCursorPosition(g_lastMousePosX, g_lastMousePosY);
	}

	void CMashWinDevice::BeginUpdate()
	{
		if (m_cageMouse)
		{
			RECT rcClip;
			GetWindowRect(m_hWnd, &rcClip);
			tagPOINT screenPos;
			screenPos.x = 0;
			screenPos.y = 0;
			ScreenToClient(m_hWnd, &screenPos );

			LONG halfWidth = m_screenWidth / 2;
			LONG halfHeight = m_screenHeight / 2;

			rcClip.left -= screenPos.x;
			rcClip.top -= screenPos.y;
			rcClip.left += halfWidth;
			rcClip.top += halfHeight;
			
			//this is in client space
			g_lastMousePosX = halfWidth;
			g_lastMousePosY = halfHeight;

			SetCursorPos(rcClip.left, rcClip.top);
		}
	}

	void CMashWinDevice::HideMouseCursor(bool state)
	{
		if (m_mouseCursorHidden == state)
			return;
		
		m_mouseCursorHidden = state;
		ShowCursor(!m_mouseCursorHidden);
	}

	void CMashWinDevice::LockMouseToScreenCenter(bool state)
	{
		if (m_cageMouse == state)
			return;

		if (state)
		{
			RECT rcClip;
			GetWindowRect(m_hWnd, &rcClip);
			tagPOINT screenPos;
			screenPos.x = 0;
			screenPos.y = 0;
			ScreenToClient( m_hWnd, &screenPos );
			rcClip.left -= screenPos.x;
			rcClip.top -= screenPos.y;
			rcClip.right = rcClip.left + m_screenWidth;
			rcClip.bottom = rcClip.top + m_screenHeight;
			ClipCursor(&rcClip); 
		}
		else
		{
			ClipCursor(NULL);
		}

		m_cageMouse = state;
	}

	bool IsXInputDevice(WORD pid)
	{
		bool isXinputDevice = false;
		#ifndef __MINGW32__
		IWbemServices* pIWbemServices = NULL;
		IEnumWbemClassObject* pEnumDevices = NULL;
		IWbemLocator* pIWbemLocator = NULL;
		IWbemClassObject* pDevices[20] = {0};
		BSTR bstrDeviceID = NULL;
		BSTR bstrClassName = NULL;
		BSTR bstrNamespace = NULL;
		DWORD uReturned = 0;
		bool bCleanupCOM = false;
		UINT iDevice = 0;
		VARIANT var;
		HRESULT hr;

		// CoInit if needed
		hr = CoInitialize( NULL );
		bCleanupCOM = SUCCEEDED( hr );

		// Create WMI
		hr = CoCreateInstance( __uuidof( WbemLocator ),
							   NULL,
							   CLSCTX_INPROC_SERVER,
							   __uuidof( IWbemLocator ),
							   ( LPVOID* )&pIWbemLocator );
		if( FAILED( hr ) || pIWbemLocator == NULL )
			goto LCleanup;

		// Create BSTRs for WMI
		bstrNamespace = SysAllocString( L"\\\\.\\root\\cimv2" ); if( bstrNamespace == NULL ) goto LCleanup;
		bstrDeviceID = SysAllocString( L"DeviceID" );           if( bstrDeviceID == NULL )  goto LCleanup;
		bstrClassName = SysAllocString( L"Win32_PNPEntity" );    if( bstrClassName == NULL ) goto LCleanup;

		// Connect to WMI 
		hr = pIWbemLocator->ConnectServer( bstrNamespace, NULL, NULL, 0L,
										   0L, NULL, NULL, &pIWbemServices );
		if( FAILED( hr ) || pIWbemServices == NULL )
			goto LCleanup;

		// Switch security level to IMPERSONATE
		CoSetProxyBlanket( pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
						   RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, 0 );

		// Get list of Win32_PNPEntity devices
		hr = pIWbemServices->CreateInstanceEnum( bstrClassName, 0, NULL, &pEnumDevices );
		if( FAILED( hr ) || pEnumDevices == NULL )
			goto LCleanup;

		// Loop over all devices
		for(; ; )
		{
			// Get 20 at a time
			hr = pEnumDevices->Next( 10000, 20, pDevices, &uReturned );
			if( FAILED( hr ) )
				goto LCleanup;
			if( uReturned == 0 )
				break;

			for( iDevice = 0; iDevice < uReturned; iDevice++ )
			{
				// For each device, get its device ID
				hr = pDevices[iDevice]->Get( bstrDeviceID, 0L, &var, NULL, NULL );
				if( SUCCEEDED( hr ) && var.vt == VT_BSTR && var.bstrVal != NULL )
				{
					// Check if the device ID contains "IG_".  If it does, then it�s an XInput device
					// Unfortunately this information can not be found by just using DirectInput 
					if( wcsstr( var.bstrVal, L"IG_" ) )
					{
						// If it does, then get the VID/PID from var.bstrVal
						DWORD dwPid = 0, dwVid = 0;
						WCHAR* strVid = wcsstr( var.bstrVal, L"VID_" );
						if( strVid && swscanf( strVid, L"VID_%4X", &dwVid ) != 1 )
							dwVid = 0;
						WCHAR* strPid = wcsstr( var.bstrVal, L"PID_" );
						if( strPid && swscanf( strPid, L"PID_%4X", &dwPid ) != 1 )
							dwPid = 0;

						if (dwPid == pid)
						{
							isXinputDevice = true;
							goto LCleanup;
						}
					}
				}
				SAFE_RELEASE( pDevices[iDevice] );
			}
		}

	LCleanup:
		if( bstrNamespace )
			SysFreeString( bstrNamespace );
		if( bstrDeviceID )
			SysFreeString( bstrDeviceID );
		if( bstrClassName )
			SysFreeString( bstrClassName );
		for( iDevice = 0; iDevice < 20; iDevice++ )
		SAFE_RELEASE( pDevices[iDevice] );
		SAFE_RELEASE( pEnumDevices );
		SAFE_RELEASE( pIWbemLocator );
		SAFE_RELEASE( pIWbemServices );

		#endif
		return isXinputDevice;
	}

	eMASH_STATUS CMashWinDevice::Initialise(const mash::sMashDeviceSettings &settings)
	{
		if (!m_hInstance)
			m_hInstance = GetModuleHandle(0);			

		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);

		const int8* sClassName = "MashWin32Device";

		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= (WNDPROC)WndProc;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.hInstance		= m_hInstance;
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= (HBRUSH)GetStockObject(WHITE_BRUSH);
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= sClassName;
		wcex.hIconSm		= 0;

		/*
			Adjust the back buffer size so that the client area fits the window.
			This stops the backbuffer shrinking to fit a windows that is a tad
			smaller than desired due to boarders, tollbar, etc...
		*/
		RECT rect;
		rect.left = 0;
		rect.top = 0;
		rect.right = settings.screenWidth;
		rect.bottom = settings.screenHeight;
		DWORD dwstyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		const DWORD dwexstyle = WS_EX_APPWINDOW;

		if (settings.fullScreen)
			dwstyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		AdjustWindowRectEx(&rect, dwstyle, FALSE, dwexstyle);

		RegisterClassEx(&wcex);

		if (m_hWnd == 0)
		{
		   m_hWnd = CreateWindowEx(dwexstyle,
			   sClassName,
			   "",
			  dwstyle,
			   0,
			   0,
			  rect.right - rect.left,
			  rect.bottom - rect.top,
			   NULL,
			   NULL,
			   m_hInstance,
			   NULL);

		   ShowWindow(m_hWnd, SW_SHOW);
		   UpdateWindow(m_hWnd);
		}

		

	   if (!m_hWnd)
	   {
		   MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                          "Failed to create window.", 
                          "CMashWinDevice::CMashWinDevice");

		  return aMASH_FAILED;
	   }

	   m_screenWidth = settings.screenWidth;
	   m_screenHeight = settings.screenHeight;
	   
	   g_windowsInitComplete = true;

	   //map window keys
	   memset(m_keyMap, 0, sizeof(m_keyMap));
	   m_keyMap[VK_F1] = aKEYEVENT_F1;
	   m_keyMap[VK_F2] = aKEYEVENT_F2;
	   m_keyMap[VK_F3] = aKEYEVENT_F3;
	   m_keyMap[VK_F4] = aKEYEVENT_F4;
	   m_keyMap[VK_F5] = aKEYEVENT_F5;
	   m_keyMap[VK_F6] = aKEYEVENT_F6;
	   m_keyMap[VK_F7] = aKEYEVENT_F7;
	   m_keyMap[VK_F8] = aKEYEVENT_F8;
	   m_keyMap[VK_F9] = aKEYEVENT_F9;
	   m_keyMap[VK_F10] = aKEYEVENT_F10;
	   m_keyMap[VK_F11] = aKEYEVENT_F11;
	   m_keyMap[VK_F12] = aKEYEVENT_F12;

	   m_keyMap[VK_SHIFT] = aKEYEVENT_SHIFT;
	   m_keyMap[VK_CONTROL] = aKEYEVENT_CTRL;
	   m_keyMap[VK_MENU] = aKEYEVENT_MENU;
	   m_keyMap[VK_ESCAPE] = aKEYEVENT_ESCAPE;
	   m_keyMap[VK_TAB] = aKEYEVENT_TAB;
	   m_keyMap[VK_RETURN] = aKEYEVENT_RETURN;
	   m_keyMap[VK_LEFT] = aKEYEVENT_LEFT;
	   m_keyMap[VK_RIGHT] = aKEYEVENT_RIGHT;
	   m_keyMap[VK_UP] = aKEYEVENT_UP;
	   m_keyMap[VK_DOWN] = aKEYEVENT_DOWN;
	   m_keyMap[VK_PRIOR] = aKEYEVENT_PGUP;
	   m_keyMap[VK_NEXT] = aKEYEVENT_PGDOWN;
	   m_keyMap[VK_DELETE] = aKEYEVENT_DELETE;
	   m_keyMap[VK_BACK] = aKEYEVENT_BACKSPACE;
	   m_keyMap[VK_HOME] = aKEYEVENT_HOME;
	   m_keyMap[VK_END] = aKEYEVENT_END;

		m_winJoystickMap[0].joyButton = JOY_BUTTON1;
		m_winJoystickMap[0].action = aJOYEVENT_B0;
		m_winJoystickMap[1].joyButton = JOY_BUTTON2;
		m_winJoystickMap[1].action = aJOYEVENT_B1;
		m_winJoystickMap[2].joyButton = JOY_BUTTON3;
		m_winJoystickMap[2].action = aJOYEVENT_B2;
		m_winJoystickMap[3].joyButton = JOY_BUTTON4;
		m_winJoystickMap[3].action = aJOYEVENT_B3;
		m_winJoystickMap[4].joyButton = JOY_BUTTON5;
		m_winJoystickMap[4].action = aJOYEVENT_B4;
		m_winJoystickMap[5].joyButton = JOY_BUTTON6;
		m_winJoystickMap[5].action = aJOYEVENT_B5;
		m_winJoystickMap[6].joyButton = JOY_BUTTON7;
		m_winJoystickMap[6].action = aJOYEVENT_B6;
		m_winJoystickMap[7].joyButton = JOY_BUTTON8;
		m_winJoystickMap[7].action = aJOYEVENT_B7;
		m_winJoystickMap[8].joyButton = JOY_BUTTON9;
		m_winJoystickMap[8].action = aJOYEVENT_B8;

		m_xInputMap[0].joyButton = XINPUT_GAMEPAD_DPAD_UP;
		m_xInputMap[0].action = aJOYEVENT_POVUP;
		m_xInputMap[1].joyButton = XINPUT_GAMEPAD_DPAD_DOWN;
		m_xInputMap[1].action = aJOYEVENT_POVDOWN;
		m_xInputMap[2].joyButton = XINPUT_GAMEPAD_DPAD_LEFT;
		m_xInputMap[2].action = aJOYEVENT_POVLEFT;
		m_xInputMap[3].joyButton = XINPUT_GAMEPAD_DPAD_RIGHT;
		m_xInputMap[3].action = aJOYEVENT_POVRIGHT;
		m_xInputMap[4].joyButton = XINPUT_GAMEPAD_START;
		m_xInputMap[4].action = aJOYEVENT_B0;
		m_xInputMap[5].joyButton = XINPUT_GAMEPAD_BACK;
		m_xInputMap[5].action = aJOYEVENT_B1;
		m_xInputMap[6].joyButton = XINPUT_GAMEPAD_LEFT_THUMB;
		m_xInputMap[6].action = aJOYEVENT_B2;
		m_xInputMap[7].joyButton = XINPUT_GAMEPAD_RIGHT_THUMB;
		m_xInputMap[7].action = aJOYEVENT_B3;
		m_xInputMap[8].joyButton = XINPUT_GAMEPAD_LEFT_SHOULDER;
		m_xInputMap[8].action = aJOYEVENT_B4;

		m_xInputMap[9].joyButton = XINPUT_GAMEPAD_RIGHT_SHOULDER;
		m_xInputMap[9].action = aJOYEVENT_B5;
		m_xInputMap[10].joyButton = XINPUT_GAMEPAD_A;
		m_xInputMap[10].action = aJOYEVENT_B6;
		m_xInputMap[11].joyButton = XINPUT_GAMEPAD_B;
		m_xInputMap[11].action = aJOYEVENT_B7;
		m_xInputMap[12].joyButton = XINPUT_GAMEPAD_X;
		m_xInputMap[12].action = aJOYEVENT_B8;
		m_xInputMap[13].joyButton = XINPUT_GAMEPAD_Y;
		m_xInputMap[13].action = aJOYEVENT_B9;

		 MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
                          "Windows device created.", 
                          "CMashWinDevice::CMashWinDevice");

	   return aMASH_OK;
	}

	uint32 CMashWinDevice::CalculateLegacyPOVState(uint32 POVState)
	{
		/*
			The following code is in a very particular order. If any problems arise,
			make sure the key enums have not been tampered with.

			The POV buttons will only tell us whats pressed but not 
			if they have been released. So we need to figure that out
			on our own.
		*/
		uint32 currentPOVState = 0;
		switch(POVState)
		{
		case JOY_POVFORWARD:
			currentPOVState |= 0x01;
			break;
		case JOY_POVBACKWARD:
			currentPOVState |= 0x02;
			break;
		case JOY_POVLEFT:
			currentPOVState |= 0x04;
			break;
		case JOY_POVRIGHT:
			currentPOVState |= 0x08;
			break;
		};

		return currentPOVState;
	}

	eMASH_STATUS CMashWinDevice::LoadComponents(const mash::sMashDeviceSettings &settings)
	{
		if (CMashDevice::LoadComponents(settings) == aMASH_FAILED)
			return aMASH_FAILED;

		m_keyboardMouseControllerID = m_pInputManager->_CreateController(aINPUTCONTROLLER_KEYBOARD_MOUSE, true);

		JOYINFO joyinfo;
	   UINT numDevices = joyGetNumDevs();
	   uint32 xinputId = 0;
	   if (numDevices > 0)
	   {
		   for(uint32 i = 0; i < numDevices; ++i)
		   {
			   /*
					Note : joyGetDevCaps and joyGetPos seem to have a memory leak
					or access error when app verifier is used and a controller is
					connected. There is not much that can be done about it. It seems
					to run fine however.
			   */
			   JOYCAPS joycaps;
			   if (joyGetDevCaps(i, &joycaps, sizeof(JOYCAPS)) == JOYERR_NOERROR)
			   {
				   sJoystickInfo newJoystick;
					if (IsXInputDevice(joycaps.wPid))
					{
						newJoystick.isXInput = true;
						newJoystick.windowsID = xinputId++;
						newJoystick.maxAxis1 = 1.0f / 32767.0f;
						newJoystick.maxAxis2 = 1.0f / 32767.0f;
						newJoystick.maxAxisT1 = 1.0f / 255.0f;
						newJoystick.maxAxisT2 = 1.0f / 255.0f;

						XINPUT_STATE currentState;
						if (XInputGetState(newJoystick.windowsID, &currentState) == ERROR_SUCCESS)
						{
							currentState.Gamepad.sThumbLX = math::FilterIntValue<int16>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, currentState.Gamepad.sThumbLX);
							currentState.Gamepad.sThumbLY = math::FilterIntValue<int16>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, currentState.Gamepad.sThumbLY);
							currentState.Gamepad.sThumbRX = math::FilterIntValue<int16>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, currentState.Gamepad.sThumbRX);
							currentState.Gamepad.sThumbRY = math::FilterIntValue<int16>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, currentState.Gamepad.sThumbRY);
							memcpy(&newJoystick.xboxController, &currentState, sizeof(XINPUT_STATE));
						}
					}
					else
					{
						newJoystick.isXInput = false;
						newJoystick.windowsID = i;
						newJoystick.maxAxis1 = 1.0f / 32767.0f;
						newJoystick.maxAxis2 = 1.0f / 32767.0f;
						newJoystick.maxAxisT1 = 1.0f / 32767.0f;
						newJoystick.maxAxisT2 = 1.0f / 32767.0f;

						//initlaize data
						JOYINFOEX currentState;
						currentState.dwSize = sizeof(JOYINFOEX);
						currentState.dwFlags = JOY_RETURNALL | JOY_RETURNCENTERED;
						MMRESULT result = joyGetPosEx(newJoystick.windowsID, &currentState);
						if (result == JOYERR_NOERROR )
						{
							memcpy(&newJoystick.legacyController, &currentState, sizeof(JOYINFOEX));
							newJoystick.legacyPOVState = CalculateLegacyPOVState(newJoystick.legacyController.dwPOV);
						}
					}

					newJoystick.controllerID = m_pInputManager->_CreateController(aINPUTCONTROLLER_JOYSTICK, true);
					m_joystickInfo.PushBack(newJoystick);
			   }
		   }
	   }

		return aMASH_OK;
	}

	bool CMashWinDevice::PollMessages()
	{
		MSG msg;
		while(::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			
			::DispatchMessage(&msg);

			if(msg.message == WM_QUIT)
				return true;
		}

		//poll joysticks
		const uint32 joystickCount = m_joystickInfo.Size();
		for(uint32 i = 0; i < joystickCount; ++i)
		{
			sJoystickInfo *currentJoystickInfo = &m_joystickInfo[i];
			if (currentJoystickInfo->isXInput)
			{
				XINPUT_STATE currentState;
				if (XInputGetState(currentJoystickInfo->windowsID, &currentState) == ERROR_SUCCESS)
				{
					if (!currentJoystickInfo->isConnected)
					{
						currentJoystickInfo->isConnected = true;
						sInputEvent newEvent;
						newEvent.eventType = mash::sInputEvent::aEVENTTYPE_CONTROLLER_CONNECT;
						newEvent.controllerID = currentJoystickInfo->controllerID;
						newEvent.action = aKEYEVENT_NONE;
						m_pInputManager->ImmediateBroadcast(newEvent);
					}

					if (currentState.dwPacketNumber != currentJoystickInfo->xboxController.dwPacketNumber)
					{
						const sJoystickThreshold *controllerThresholds = m_pInputManager->GetControllerThresholds(currentJoystickInfo->controllerID);

						//process new input
						sInputEvent newEvent;
						newEvent.eventType = mash::sInputEvent::aEVENTTYPE_JOYSTICK;
						newEvent.controllerID = currentJoystickInfo->controllerID;

						const XINPUT_GAMEPAD *oldControllerState = &currentJoystickInfo->xboxController.Gamepad;

						if (currentState.Gamepad.bLeftTrigger != oldControllerState->bLeftTrigger)
						{
							f32 t1 = math::FilterFloatValue(controllerThresholds->throttle1, currentState.Gamepad.bLeftTrigger * currentJoystickInfo->maxAxisT1);
							{
								newEvent.action = aJOYEVENT_THROTTLE_1;
								newEvent.value = t1;
								m_pInputManager->ImmediateBroadcast(newEvent);
							}
						}

						if (currentState.Gamepad.bRightTrigger != oldControllerState->bRightTrigger)
						{
							f32 t2 = math::FilterFloatValue(controllerThresholds->throttle2, currentState.Gamepad.bRightTrigger * currentJoystickInfo->maxAxisT2);
							{
								newEvent.action = aJOYEVENT_THROTTLE_2;
								newEvent.value = t2;
								m_pInputManager->ImmediateBroadcast(newEvent);
							}
						}

						/*
							The thumbsticks need to be filtered otherwise we get false positives
						*/
						currentState.Gamepad.sThumbLX = math::FilterIntValue<int16>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, currentState.Gamepad.sThumbLX);
						currentState.Gamepad.sThumbLY = math::FilterIntValue<int16>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, currentState.Gamepad.sThumbLY);
						currentState.Gamepad.sThumbRX = math::FilterIntValue<int16>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, currentState.Gamepad.sThumbRX);
						currentState.Gamepad.sThumbRY = math::FilterIntValue<int16>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, currentState.Gamepad.sThumbRY);

						if (currentState.Gamepad.sThumbLX != oldControllerState->sThumbLX)
						{
							f32 a = math::FilterFloatValue(controllerThresholds->axis1, currentState.Gamepad.sThumbLX * currentJoystickInfo->maxAxis1);
							{
								newEvent.action = aJOYEVENT_AXIS_1_X;
								newEvent.value = a;
								m_pInputManager->ImmediateBroadcast(newEvent);
							}
						}

						if (currentState.Gamepad.sThumbLY != oldControllerState->sThumbLY)
						{
							f32 a = math::FilterFloatValue(controllerThresholds->axis1, currentState.Gamepad.sThumbLY * currentJoystickInfo->maxAxis1);
							{
								newEvent.action = aJOYEVENT_AXIS_1_Y;
								newEvent.value = a;
								m_pInputManager->ImmediateBroadcast(newEvent);
							}
						}

						if (currentState.Gamepad.sThumbRX != oldControllerState->sThumbRX)
						{
							f32 a = math::FilterFloatValue(controllerThresholds->axis2, currentState.Gamepad.sThumbRX * currentJoystickInfo->maxAxis2);
							{
								newEvent.action = aJOYEVENT_AXIS_2_X;
								newEvent.value = a;
								m_pInputManager->ImmediateBroadcast(newEvent);
							}
						}

						if (currentState.Gamepad.sThumbRY != oldControllerState->sThumbRY)
						{
							f32 a = math::FilterFloatValue(controllerThresholds->axis2, currentState.Gamepad.sThumbRY * currentJoystickInfo->maxAxis2);
							{
								newEvent.action = aJOYEVENT_AXIS_2_Y;
								newEvent.value = a;
								m_pInputManager->ImmediateBroadcast(newEvent);
							}
						}

						//check buttons
						if (currentState.Gamepad.wButtons != oldControllerState->wButtons)
						{
							for(uint32 i = 0; i < g_xInputButtonCount; ++i)
							{
								if ((currentState.Gamepad.wButtons & m_xInputMap[i].joyButton) != (oldControllerState->wButtons & m_xInputMap[i].joyButton))
								{
									newEvent.action = m_xInputMap[i].action;
									newEvent.isPressed = (bool)(currentState.Gamepad.wButtons & m_xInputMap[i].joyButton);
									newEvent.value = (f32)newEvent.isPressed;
									m_pInputManager->ImmediateBroadcast(newEvent);
								}
							}
						}

						memcpy(&m_joystickInfo[i].xboxController, &currentState, sizeof(XINPUT_STATE));
					}
				}
				else
				{
					if (currentJoystickInfo->isConnected)
					{
						currentJoystickInfo->isConnected = false;
						sInputEvent newEvent;
						newEvent.eventType = sInputEvent::aEVENTTYPE_CONTROLLER_DISCONNECT;
						newEvent.controllerID = currentJoystickInfo->controllerID;
						newEvent.action = aKEYEVENT_NONE;
						m_pInputManager->ImmediateBroadcast(newEvent);
					}
				}
			}
			else
			{
				JOYINFOEX currentState;
				currentState.dwSize = sizeof(JOYINFOEX);
				currentState.dwFlags = JOY_RETURNALL | JOY_RETURNCENTERED;// | JOY_USEDEADZONE;
				MMRESULT result = joyGetPosEx(currentJoystickInfo->windowsID, &currentState);
				if (result == JOYERR_NOERROR )
				{
					if (!currentJoystickInfo->isConnected)
					{
						currentJoystickInfo->isConnected = true;
						sInputEvent newEvent;
						newEvent.eventType = mash::sInputEvent::aEVENTTYPE_CONTROLLER_CONNECT;
						newEvent.controllerID = currentJoystickInfo->controllerID;
						newEvent.action = aKEYEVENT_NONE;
						m_pInputManager->ImmediateBroadcast(newEvent);
					}

					if (memcmp(&currentState, &currentJoystickInfo->legacyController, sizeof(JOYINFOEX)) != 0)
					{
						const sJoystickThreshold *controllerThresholds = m_pInputManager->GetControllerThresholds(currentJoystickInfo->controllerID);

						sInputEvent newEvent;
						newEvent.eventType = mash::sInputEvent::aEVENTTYPE_JOYSTICK;
						newEvent.controllerID = currentJoystickInfo->controllerID;

						//range = 0...65536. Move to -32768...32768
						if (currentState.dwZpos != currentJoystickInfo->legacyController.dwZpos)
						{
							f32 t1 = math::FilterFloatValue(controllerThresholds->throttle1, currentState.dwZpos * currentJoystickInfo->maxAxisT1);
							{
								newEvent.action = aJOYEVENT_THROTTLE_1;
								newEvent.value = t1;
								m_pInputManager->ImmediateBroadcast(newEvent);
							}
						}

						if (currentState.dwVpos != currentJoystickInfo->legacyController.dwVpos)
						{
							f32 t2 = math::FilterFloatValue(controllerThresholds->throttle2, currentState.dwVpos * currentJoystickInfo->maxAxisT2);
							{
								newEvent.action = aJOYEVENT_THROTTLE_2;
								newEvent.value = t2;
								m_pInputManager->ImmediateBroadcast(newEvent);
							}
						}

						if (currentState.dwXpos != currentJoystickInfo->legacyController.dwXpos)
						{
							f32 a = math::FilterFloatValue(controllerThresholds->axis1, currentState.dwXpos * currentJoystickInfo->maxAxis1);
							{
								newEvent.action = aJOYEVENT_AXIS_1_X;
								newEvent.value = a;
								m_pInputManager->ImmediateBroadcast(newEvent);
							}
						}

						if (currentState.dwYpos != currentJoystickInfo->legacyController.dwYpos)
						{
							f32 a = math::FilterFloatValue(controllerThresholds->axis1, currentState.dwYpos * currentJoystickInfo->maxAxis1);
							{
								newEvent.action = aJOYEVENT_AXIS_1_Y;
								newEvent.value = a;
								m_pInputManager->ImmediateBroadcast(newEvent);
							}
						}

						if (currentState.dwRpos != currentJoystickInfo->legacyController.dwRpos)
						{
							f32 a = math::FilterFloatValue(controllerThresholds->axis2, currentState.dwRpos * currentJoystickInfo->maxAxis2);
							{
								newEvent.action = aJOYEVENT_AXIS_2_X;
								newEvent.value = a;
								m_pInputManager->ImmediateBroadcast(newEvent);
							}
						}

						if (currentState.dwUpos != currentJoystickInfo->legacyController.dwUpos)
						{
							f32 a = math::FilterFloatValue(controllerThresholds->axis2, currentState.dwUpos * currentJoystickInfo->maxAxis2);
							{
								newEvent.action = aJOYEVENT_AXIS_2_Y;
								newEvent.value = a;
								m_pInputManager->ImmediateBroadcast(newEvent);
							}
						}

						if (currentState.dwButtons != currentJoystickInfo->legacyController.dwButtons)
						{
							for(uint32 i = 0; i < g_joystickButtonCount; ++i)
							{
								if ((currentState.dwButtons & m_winJoystickMap[i].joyButton) != (currentJoystickInfo->legacyController.dwButtons & m_winJoystickMap[i].joyButton))
								{
									newEvent.action = m_winJoystickMap[i].action;
									newEvent.isPressed = (bool)(currentState.dwButtons & m_winJoystickMap[i].joyButton);
									newEvent.value = (f32)newEvent.isPressed;
									m_pInputManager->ImmediateBroadcast(newEvent);
								}
							}
						}

						if (currentJoystickInfo->legacyController.dwPOV != currentState.dwPOV)
						{
							/*
								The following code is in a very particular order. If any problems arise,
								make sure the key enums have not been tampered with.

								The POV buttons will only tell us whats pressed but not 
								if they have been released. So we need to figure that out
								on our own.
							*/
							uint32 currentPOVState = CalculateLegacyPOVState(currentState.dwPOV);
							for(uint32 i = 0; i < 4; ++i)
							{
								if ((currentPOVState & (1<<i)) != (currentJoystickInfo->legacyPOVState & (1<<i)))
								{
									newEvent.action = (eINPUT_EVENT) (aJOYEVENT_POVUP + i);
									newEvent.isPressed = (bool)(currentPOVState & (1<<i));
									newEvent.value = (f32)newEvent.isPressed;
									m_pInputManager->ImmediateBroadcast(newEvent);
								}
							}

							currentJoystickInfo->legacyPOVState = currentPOVState;
						}

						memcpy(&currentJoystickInfo->legacyController, &currentState, sizeof(JOYINFOEX));
					}
				}
				else if (result == JOYERR_UNPLUGGED)
				{
					if (currentJoystickInfo->isConnected)
					{
						currentJoystickInfo->isConnected = false;
						sInputEvent newEvent;
						newEvent.eventType = sInputEvent::aEVENTTYPE_CONTROLLER_DISCONNECT;
						newEvent.controllerID = currentJoystickInfo->controllerID;
						newEvent.action = aKEYEVENT_NONE;
						m_pInputManager->ImmediateBroadcast(newEvent);
					}
				}
			}
		}

		return false;
	}

	void CMashWinDevice::Sleep(uint32 ms)const
	{
		Sleep(ms);
	}

	void CMashWinDevice::SetWindowCaption(const int8 *text)
	{
		SetWindowTextA(m_hWnd, (LPCSTR)text);
	}

	_MASH_EXPORT MashDevice* CreateDevice (mash::sMashDeviceSettings &settings)
	{
		if (!settings.rendererFunctPtr)
			return 0;

		if (settings.preferredLightingMode == aLIGHT_TYPE_DEFERRED)
			settings.enableDeferredRender = true;

		/*
			TODO : Until a deferred AA solution is implimented, we force AA to none
			when the deferred renderer is enabled.
		*/
		if (settings.enableDeferredRender)
			settings.antiAliasType = aANTIALIAS_TYPE_NONE;

		CMashWinDevice *device = MASH_NEW_COMMON CMashWinDevice(settings.debugFilePath);
		if (device->Initialise(settings) == aMASH_FAILED)
		{
			MASH_DELETE device;
			return 0;
		}

		if (device->LoadComponents(settings) == aMASH_FAILED)
		{
			MASH_DELETE device;
			return 0;
		}

		return device;
	}
}
#endif
