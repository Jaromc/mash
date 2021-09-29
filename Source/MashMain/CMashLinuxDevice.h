//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef CMASHLINUXDEVICE_H
#define CMASHLINUXDEVICE_H

#include "CMashDevice.h"

#ifdef MASH_LINUX

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#include <X11/extensions/xf86vmode.h>

#include <map>
namespace mash
{
    class CMashLinuxDevice : public CMashDevice
    {
    private:
        struct sJoystick
        {
            enum
            {
                MAX_AXIS = 8
            };

            int32 id;
            int32 buttons;
            int32 axis;
            int32 controllerId;
            eINPUT_EVENT lastHorizontalPOV;
            eINPUT_EVENT lastVerticalPOV;

            //2 thumbsticks * xy + 2 throttles + dpad
            int16 lastAxisValues[MAX_AXIS];

            sJoystick():id(0), buttons(0), axis(0), controllerId(0),
                lastHorizontalPOV(aJOYEVENT_POVRIGHT), lastVerticalPOV(aJOYEVENT_POVUP)
            {
                for(uint32 i = 0; i < MAX_AXIS; ++i)
                {
                    lastAxisValues[i] = 0;
                }
            }
        };
    protected:
        bool PollMessages();
        void SyncInputDeviceWithCurrentState();
        void BeginUpdate();
    private:
        Display *m_glXdisplay;
        GLXContext m_glXctx;
        Window m_glXwin;
        Colormap m_glXcmap;
        Cursor m_invsCursor;

        uint32 m_windowSizeX;
        uint32 m_windowSizeY;
        bool m_windowHasFocus;

        uint32 m_keyboardMouseControllerID;
        std::map<int32, eINPUT_EVENT> m_keyMap;
        int m_lastMousePosX;
        int m_lastMousePosY;
        int m_lastMouseWarpPosX;
        int m_lastMouseWarpPosY;
        bool m_mouseLeftWindow;
        bool m_grabMouseWarp;
        MashArray<sJoystick> m_joysticks;
        //int32 m_joystickButtonMap[];
        //int32 m_joystickAxisMap;

        bool m_lockMouse;
        bool m_mouseCursorHidden;
        bool m_isFullscreen;
        int m_defaultScreen;
        XF86VidModeModeInfo m_oldVideoMode;

        bool isExtensionSupported(const char *extList, const char *extension);

        void GrabInput();
        void UngrabDevices();
        void InitFullscreen(bool reset);
        void WarpMouse();
    public:
        CMashLinuxDevice(const MashStringc &debugFilePath);
        ~CMashLinuxDevice();

        eMASH_STATUS Initialise(const mash::sMashDeviceSettings &settings);
        void SetWindowCaption(const int8 *text);
        void GetWindowSize(uint32 &x, uint32 &y)const;
        void _Draw();

        void LockMouseToScreenCenter(bool state);
        bool IsMouseLockedToScreenCenter()const;

        void HideMouseCursor(bool state);
        bool IsMouseCursorHidden()const;
        void Sleep(uint32 ms)const;

        eMASH_DEVICE_TYPE GetDeviceType()const;

        eMASH_STATUS LoadComponents(const mash::sMashDeviceSettings &settings);
    };

    inline bool CMashLinuxDevice::IsMouseLockedToScreenCenter()const
    {
        return m_mouseCursorHidden;
    }

    inline bool CMashLinuxDevice::IsMouseCursorHidden()const
    {
        return m_mouseCursorHidden;
    }

    inline eMASH_DEVICE_TYPE CMashLinuxDevice::GetDeviceType()const
    {
        return aDEVICE_TYPE_LINUX;
    }
}

#endif
#endif // CMASHLINUXDEVICE_H
