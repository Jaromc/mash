//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef MacOSX_CMashMacDevice_h
#define MacOSX_CAMashMacDevice_h

#include "MashDataTypes.h"
#ifdef MASH_APPLE

#include "CMashDevice.h"
#include <OpenGL/OpenGL.h>
#import <Cocoa/Cocoa.h>
#include <map>
#include "MashEventDispatch.h" 

namespace mash
{
   
	class CMashMacDevice : public CMashDevice
	{
    public:
//        struct sJoystick
//        {
//            IOHIDDeviceInterface **interface;
//            
//            IOHIDElementCookie cookie;
//            long min; // min val possible
//            long max; // max val possible
//            
//            long minRead; //min read value
//            long maxRead; //max read value
//        };
	protected:
		bool PollMessages();
    
        std::map<int32, eINPUT_EVENT> m_keyMap;
        CGLContextObj m_cglContext;
        NSOpenGLContext *m_oglContext;
        
        NSWindow *m_window;
        
        int32 m_windowOriginX;
        int32 m_windowOriginY;
        int32 m_windowSizeX;
        int32 m_windowSizeY;
        int32 m_deviceDisplayWidth;
        int32 m_deviceDisplayHeight;
        
        bool m_isControlDown;
        bool m_isShiftDown;
        
        //abs mouse pos
        NSPoint m_lastMousePos;
        
        int32 m_keyboardMouseControllerID;
        bool m_mouseCursorHidden;
        bool m_isMouseLockedToCenter;
        
        void SyncInputDeviceWithCurrentState();
        bool ProcessKeyEvent(NSEvent *event, int8 isPressed);
        
        void UpdateWindowFrame();
        void UpdateWindowFrameAndMouseCage();
        void SendMouseToScreenCenter();
	public:
		CMashMacDevice(const MashStringc &debugFilePath);
		~CMashMacDevice();
		
        eMASH_STATUS LoadComponents(const sMashDeviceSettings &settings);
		eMASH_STATUS Initialise(const sMashDeviceSettings &settings);
		void SetWindowCaption(const int8 *text);
        
        eMASH_DEVICE_TYPE GetDeviceType()const;
        void _Draw();
        
        void OnResize();
        void OnWindowWillMove();
        void OnWindowMoved();
        
        void SetMouseCage(bool state);
        void GetWindowSize(uint32 &x, uint32 &y)const;
        
        void LockMouseToScreenCenter(bool state);
		bool IsMouseLockedToScreenCenter()const;
        void HideMouseCursor(bool state);
		bool IsMouseCursorHidden()const;
        void Sleep(uint32 ms)const;

	};
    
    inline bool CMashMacDevice::IsMouseLockedToScreenCenter()const
    {
        return m_isMouseLockedToCenter;
    }
    
    inline bool CMashMacDevice::IsMouseCursorHidden()const
    {
        return m_mouseCursorHidden;
    }
    
    inline eMASH_DEVICE_TYPE CMashMacDevice::GetDeviceType()const
    {
        return aDEVICE_TYPE_APPLE;
    }
}

#endif


#endif
