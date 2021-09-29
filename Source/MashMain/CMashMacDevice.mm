//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#include "CMashMacDevice.h"
#ifdef MASH_APPLE

#import "AppDelegate.h"
#include "CMashInputManager.h"
#include "MashRectangle2.h"
#include "MashLog.h"
#include "MashVideo.h"
#include <Carbon/Carbon.h>
namespace mash
{
//    static IOReturn CloseJoystick(CMashMacDevice::sJoystick* js)
//    {
//        IOReturn result = kIOReturnSuccess;
//        if (js && js->interface)
//        {
//            result = (*(js->interface))->close (js->interface);
//            if (kIOReturnNotOpen == result)
//            {
//                //device wasn't opened in the first place
//            }
//            else if (kIOReturnSuccess != result)
//            {
//                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
//                                 "IOHIDDevice failed to close.", 
//                                 "CMashMacDevice::CloseJoystick");
//            }
//            
//            result = (*(js->interface))->Release (js->interface);
//            
//            if (kIOReturnSuccess != result)
//            {
//                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
//                                 "IOHIDDevice failed to release.", 
//                                 "CMashMacDevice::CloseJoystick");
//            }
//            
//            js->interface = NULL;
//        }
//        
//        return result;
//    }
//    
//    static void addComponentInfo (CFTypeRef refElement, CMashMacDevice::sJoystick* js, int numActiveJoysticks)
//    {
//        long number;
//        CFTypeRef refType;
//        
//        refType = CFDictionaryGetValue ((CFDictionaryRef)refElement, CFSTR(kIOHIDElementCookieKey));
//        if (refType && CFNumberGetValue ((CFNumberRef)refType, kCFNumberLongType, &number))
//            js->cookie = (IOHIDElementCookie) number;
//        refType = CFDictionaryGetValue ((CFDictionaryRef)refElement, CFSTR(kIOHIDElementMinKey));
//        if (refType && CFNumberGetValue ((CFNumberRef)refType, kCFNumberLongType, &number))
//            js->minRead = pComponent->min = number;
//        refType = CFDictionaryGetValue ((CFDictionaryRef)refElement, CFSTR(kIOHIDElementMaxKey));
//        if (refType && CFNumberGetValue ((CFNumberRef)refType, kCFNumberLongType, &number))
//            js->maxRead = pComponent->max = number;
//    }
//    
//    static void getJoystickComponentArrayHandler (const void * value, void * parameter);
//    
//    static void addJoystickComponent (CFTypeRef refElement, JoystickInfo* joyInfo)
//    {
//        long elementType, usagePage, usage;
//        CFTypeRef refElementType = CFDictionaryGetValue ((CFDictionaryRef)refElement, CFSTR(kIOHIDElementTypeKey));
//        CFTypeRef refUsagePage = CFDictionaryGetValue ((CFDictionaryRef)refElement, CFSTR(kIOHIDElementUsagePageKey));
//        CFTypeRef refUsage = CFDictionaryGetValue ((CFDictionaryRef)refElement, CFSTR(kIOHIDElementUsageKey));
//        
//        if ((refElementType) && (CFNumberGetValue ((CFNumberRef)refElementType, kCFNumberLongType, &elementType)))
//        {
//            /* look at types of interest */
//            if ((elementType == kIOHIDElementTypeInput_Misc) || (elementType == kIOHIDElementTypeInput_Button) ||
//                (elementType == kIOHIDElementTypeInput_Axis))
//            {
//                if (refUsagePage && CFNumberGetValue ((CFNumberRef)refUsagePage, kCFNumberLongType, &usagePage) &&
//                    refUsage && CFNumberGetValue ((CFNumberRef)refUsage, kCFNumberLongType, &usage))
//                {
//                    switch (usagePage) /* only interested in kHIDPage_GenericDesktop and kHIDPage_Button */
//                    {
//                        case kHIDPage_GenericDesktop:
//                        {
//                            switch (usage) /* look at usage to determine function */
//                            {
//                                case kHIDUsage_GD_X:
//                                case kHIDUsage_GD_Y:
//                                case kHIDUsage_GD_Z:
//                                case kHIDUsage_GD_Rx:
//                                case kHIDUsage_GD_Ry:
//                                case kHIDUsage_GD_Rz:
//                                case kHIDUsage_GD_Slider:
//                                case kHIDUsage_GD_Dial:
//                                case kHIDUsage_GD_Wheel:
//                                {
//                                    joyInfo->axes++;
//                                    JoystickComponent newComponent;
//                                    addComponentInfo(refElement, &newComponent, joyInfo->numActiveJoysticks);
//                                    joyInfo->axisComp.push_back(newComponent);
//                                }
//                                    break;
//                                case kHIDUsage_GD_Hatswitch:
//                                {
//                                    joyInfo->hats++;
//                                    JoystickComponent newComponent;
//                                    addComponentInfo(refElement, &newComponent, joyInfo->numActiveJoysticks);
//                                    joyInfo->hatComp.push_back(newComponent);
//                                }
//                                    break;
//                            }
//                        }
//                            break;
//                        case kHIDPage_Button:
//                        {
//                            joyInfo->buttons++;
//                            JoystickComponent newComponent;
//                            addComponentInfo(refElement, &newComponent, joyInfo->numActiveJoysticks);
//                            joyInfo->buttonComp.push_back(newComponent);
//                        }
//                            break;
//                        default:
//                            break;
//                    }
//                }
//            }
//            else if (kIOHIDElementTypeCollection == elementType)
//            {
//                //get elements
//                CFTypeRef refElementTop = CFDictionaryGetValue ((CFMutableDictionaryRef) refElement, CFSTR(kIOHIDElementKey));
//                if (refElementTop)
//                {
//                    CFTypeID type = CFGetTypeID (refElementTop);
//                    if (type == CFArrayGetTypeID())
//                    {
//                        CFRange range = {0, CFArrayGetCount ((CFArrayRef)refElementTop)};
//                        CFArrayApplyFunction ((CFArrayRef)refElementTop, range, getJoystickComponentArrayHandler, joyInfo);
//                    }
//                }
//            }
//        }
//    }
//    
//    static void getJoystickComponentArrayHandler (const void * value, void * parameter)
//    {
//        if (CFGetTypeID (value) == CFDictionaryGetTypeID ())
//            addJoystickComponent ((CFTypeRef) value, (JoystickInfo *) parameter);
//    }
//    
//    static void joystickTopLevelElementHandler (const void * value, void * parameter)
//    {
//        CFTypeRef refCF = 0;
//        if (CFGetTypeID (value) != CFDictionaryGetTypeID ())
//            return;
//        refCF = CFDictionaryGetValue ((CFDictionaryRef)value, CFSTR(kIOHIDElementUsagePageKey));
//        if (!CFNumberGetValue ((CFNumberRef)refCF, kCFNumberLongType, &((JoystickInfo *) parameter)->usagePage))
//            irr::os::Printer::log("CFNumberGetValue error retrieving JoystickInfo->usagePage", irr::ELL_ERROR);
//        refCF = CFDictionaryGetValue ((CFDictionaryRef)value, CFSTR(kIOHIDElementUsageKey));
//        if (!CFNumberGetValue ((CFNumberRef)refCF, kCFNumberLongType, &((JoystickInfo *) parameter)->usage))
//            irr::os::Printer::log("CFNumberGetValue error retrieving JoystickInfo->usage", irr::ELL_ERROR);
//    }
//    
//    static void getJoystickDeviceInfo (io_object_t hidDevice, CFMutableDictionaryRef hidProperties, JoystickInfo *joyInfo)
//    {
//        CFMutableDictionaryRef usbProperties = 0;
//        io_registry_entry_t parent1, parent2;
//        
//        /* Mac OS X currently is not mirroring all USB properties to HID page so need to look at USB device page also
//         * get dictionary for usb properties: step up two levels and get CF dictionary for USB properties
//         */
//        if ((KERN_SUCCESS == IORegistryEntryGetParentEntry (hidDevice, kIOServicePlane, &parent1)) &&
//            (KERN_SUCCESS == IORegistryEntryGetParentEntry (parent1, kIOServicePlane, &parent2)) &&
//            (KERN_SUCCESS == IORegistryEntryCreateCFProperties (parent2, &usbProperties, kCFAllocatorDefault, kNilOptions)))
//        {
//            if (usbProperties)
//            {
//                CFTypeRef refCF = 0;
//                /* get device info
//                 * try hid dictionary first, if fail then go to usb dictionary
//                 */
//                
//                /* get joystickName name */
//                refCF = CFDictionaryGetValue (hidProperties, CFSTR(kIOHIDProductKey));
//                if (!refCF)
//                    refCF = CFDictionaryGetValue (usbProperties, CFSTR("USB Product Name"));
//                if (refCF)
//                {
//                    if (!CFStringGetCString ((CFStringRef)refCF, joyInfo->joystickName, 256, CFStringGetSystemEncoding ()))
//                        irr::os::Printer::log("CFStringGetCString error getting joyInfo->joystickName", irr::ELL_ERROR);
//                }
//                
//                /* get usage page and usage */
//                refCF = CFDictionaryGetValue (hidProperties, CFSTR(kIOHIDPrimaryUsagePageKey));
//                if (refCF)
//                {
//                    if (!CFNumberGetValue ((CFNumberRef)refCF, kCFNumberLongType, &joyInfo->usagePage))
//                        irr::os::Printer::log("CFNumberGetValue error getting joyInfo->usagePage", irr::ELL_ERROR);
//                    refCF = CFDictionaryGetValue (hidProperties, CFSTR(kIOHIDPrimaryUsageKey));
//                    if (refCF)
//                        if (!CFNumberGetValue ((CFNumberRef)refCF, kCFNumberLongType, &joyInfo->usage))
//                            irr::os::Printer::log("CFNumberGetValue error getting joyInfo->usage", irr::ELL_ERROR);
//                }
//                
//                if (NULL == refCF) /* get top level element HID usage page or usage */
//                {
//                    /* use top level element instead */
//                    CFTypeRef refCFTopElement = 0;
//                    refCFTopElement = CFDictionaryGetValue (hidProperties, CFSTR(kIOHIDElementKey));
//                    {
//                        /* refCFTopElement points to an array of element dictionaries */
//                        CFRange range = {0, CFArrayGetCount ((CFArrayRef)refCFTopElement)};
//                        CFArrayApplyFunction ((CFArrayRef)refCFTopElement, range, joystickTopLevelElementHandler, joyInfo);
//                    }
//                }
//                
//                CFRelease (usbProperties);
//            }
//            else
//                irr::os::Printer::log("IORegistryEntryCreateCFProperties failed to create usbProperties", irr::ELL_ERROR);
//            
//            if (kIOReturnSuccess != IOObjectRelease (parent2))
//                irr::os::Printer::log("IOObjectRelease failed to release parent2", irr::ELL_ERROR);
//            if (kIOReturnSuccess != IOObjectRelease (parent1))
//                irr::os::Printer::log("IOObjectRelease failed to release parent1", irr::ELL_ERROR);
//        }
//    }

    
	CMashMacDevice::CMashMacDevice(const MashStringc &debugFilePath):CMashDevice(debugFilePath),m_cglContext(nil), m_windowOriginX(0), m_windowOriginY(0), m_windowSizeX(0), m_windowSizeY(0),
        m_window(0), m_oglContext(0), m_keyboardMouseControllerID(-1), m_mouseCursorHidden(false),
        m_isMouseLockedToCenter(false), m_deviceDisplayWidth(0), m_deviceDisplayHeight(0), m_isControlDown(false),
        m_isShiftDown(false)
	{
        
	}
    
	CMashMacDevice::~CMashMacDevice()
	{
		
	}
    
	eMASH_STATUS CMashMacDevice::Initialise(const sMashDeviceSettings &settings)
	{
        [[MASHutoreleasePool alloc] init];
		[MASHpplication sharedApplication];
        [MASHpp setDelegate:[[[AppDelegate alloc] initWithDevice:this] autorelease]];
        [NSBundle loadNibNamed:@"MainMenu" owner:[MASHpp delegate]];
        [MASHpp finishLaunching];
        
        /*
            This makes sure the working directory is from the exe, not the xode directory
        */
        NSString *path;
        path = [[[NSBundle mainBundle] bundlePath] stringByDeletingLastPathComponent];
		chdir([path fileSystemRepresentation]);
		//[path release];
        
        NSRect mainDisplayRect;
        
        CGDirectDisplayID display = CGMainDisplayID();
        m_deviceDisplayWidth = (int) CGDisplayPixelsWide(display);
        m_deviceDisplayHeight = (int) CGDisplayPixelsHigh(display);
        
        display = CGMainDisplayID();

        
        eMASH_STATUS status = aMASH_FAILED;
        
        int32 backbufferFormat = 16;
        if (settings.backbufferFormat == aBACKBUFFER_FORMAT_32BIT)
            backbufferFormat = 32;
        
        int32 depthFormat = 24;
        if (settings.depthFormat == aDEPTH_FORMAT_32BIT)
            depthFormat = 32;
        
        uint32 multisampleLevels = 0;
        switch(settings.antiAliasType)
        {
            case aANTIALIAS_TYPE_X2:
                multisampleLevels = 2;
                break;
            case aANTIALIAS_TYPE_X4:
                multisampleLevels = 4;
                break;
            case aANTIALIAS_TYPE_X8:
                multisampleLevels = 8;
                break;
        }
        
        {
            if (settings.fullScreen)
            {
                mainDisplayRect = [[NSScreen mainScreen] frame];
             m_window = [[NSWindow alloc] initWithContentRect:mainDisplayRect styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:FALSE];
            }
            else
            {
                mainDisplayRect = NSMakeRect(0, 0, settings.screenWidth, settings.screenHeight);    
            m_window = [[NSWindow alloc] initWithContentRect:mainDisplayRect styleMask:NSTitledWindowMask+NSClosableWindowMask+NSResizableWindowMask backing:NSBackingStoreBuffered defer:FALSE];
            }
            
            if (m_window)
            {            				
                NSOpenGLPixelFormatAttribute attribs[] = 
                {
                    NSOpenGLPFANoRecovery,
                    NSOpenGLPFAAccelerated,
                    NSOpenGLPFADoubleBuffer,
                    NSOpenGLPFADepthSize,     (NSOpenGLPixelFormatAttribute)depthFormat,
                    NSOpenGLPFAColorSize,     (NSOpenGLPixelFormatAttribute)backbufferFormat,
                    NSOpenGLPFAAlphaSize,     (NSOpenGLPixelFormatAttribute)0,
                    NSOpenGLPFASampleBuffers, (NSOpenGLPixelFormatAttribute)((multisampleLevels > 1)?1:0),
                    NSOpenGLPFASamples,       (NSOpenGLPixelFormatAttribute)multisampleLevels,
                    NSOpenGLPFAStencilSize,   (NSOpenGLPixelFormatAttribute)0,
                    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
                    (NSOpenGLPixelFormatAttribute)nil
                };
                
                NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
                
                if (pixelFormat)
                {
                    m_oglContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:NULL];
                    [pixelFormat release];
                    
                    if (m_oglContext)
                    {
                        [m_window center];
                        [m_window setDelegate:[MASHpp delegate]];
                        [m_oglContext setView:[m_window contentView]];
                        [m_window setAcceptsMouseMovedEvents:TRUE];
                        [m_window setIsVisible:TRUE];
                        [m_window makeKeyAndOrderFront:nil];
                        [m_window setLevel:NSMainMenuWindowLevel+1];
                        
                        if (settings.fullScreen)
                        {
                            //make the window fullsceen
                            [m_window setFrame:[m_window frameRectForContentRect:[[m_window screen] frame]] display:YES animate:NO];
                            //hide the menu bar and dock
                            [NSMenu setMenuBarVisible:NO];
                        }
                        
                        UpdateWindowFrame();
                        
                        m_cglContext = (CGLContextObj) [m_oglContext CGLContextObj];
                        CGLSetCurrentContext(m_cglContext);
                        
                        status = aMASH_OK;
                    }
                }
            }
        }
        
        if (status == aMASH_OK)
        {
            //enable vsync if needed
            GLint vSync = (settings.enableVSync) ? 1 : 0;
            CGLSetParameter(CGLGetCurrentContext(),kCGLCPSwapInterval,&vSync);
            
            if (settings.fullScreen)
            {
                GLint dim[2] = {settings.screenWidth, settings.screenHeight};    
                CGLSetParameter(CGLGetCurrentContext(), kCGLCPSurfaceBackingSize, dim);
                CGLEnable (CGLGetCurrentContext(), kCGLCESurfaceBackingSize);
            }
            
            //need to do this otherwise the events dont work correctly
            ProcessSerialNumber psn;
            if (!GetCurrentProcess(&psn))
            {
                TransformProcessType(&psn, kProcessTransformToForegroundApplication);
                SetFrontProcess(&psn);
            }
        }
        
        m_keyMap[kVK_F1] = aKEYEVENT_F1;
        m_keyMap[kVK_F2] = aKEYEVENT_F2;
        m_keyMap[kVK_F3] = aKEYEVENT_F3;
        m_keyMap[kVK_F4] = aKEYEVENT_F4;
        m_keyMap[kVK_F5] = aKEYEVENT_F5;
        m_keyMap[kVK_F6] = aKEYEVENT_F6;
        m_keyMap[kVK_F7] = aKEYEVENT_F7;
        m_keyMap[kVK_F8] = aKEYEVENT_F8;
        m_keyMap[kVK_F9] = aKEYEVENT_F9;
        m_keyMap[kVK_F10] = aKEYEVENT_F10;
        m_keyMap[kVK_F11] = aKEYEVENT_F11;
        m_keyMap[kVK_F12] = aKEYEVENT_F12;
        m_keyMap[/*0x1B*/kVK_Escape] = aKEYEVENT_ESCAPE;
        m_keyMap[kVK_Tab] = aKEYEVENT_TAB;
        //m_keyMap[NSEnterCharacter] = aKEYEVENT_RETURN;
        m_keyMap[/*NSCarriageReturnCharacter*/kVK_Return] = aKEYEVENT_RETURN;

        m_keyMap[kVK_UpArrow] = aKEYEVENT_UP;
        m_keyMap[kVK_LeftArrow] = aKEYEVENT_LEFT;
        m_keyMap[kVK_RightArrow] = aKEYEVENT_RIGHT;
        m_keyMap[kVK_DownArrow] = aKEYEVENT_DOWN;
        
        m_keyMap[kVK_Delete] = aKEYEVENT_BACKSPACE;
        m_keyMap[kVK_ForwardDelete] = aKEYEVENT_DELETE;
        
        m_keyMap[kVK_Home] = aKEYEVENT_HOME;
        m_keyMap[kVK_End] = aKEYEVENT_END;
        m_keyMap[kVK_PageUp] = aKEYEVENT_PGUP;
        m_keyMap[kVK_PageDown] = aKEYEVENT_PGDOWN;
        
       /* m_keyMap[NSUpArrowFunctionKey] = aKEYEVENT_UP;
        m_keyMap[NSLeftArrowFunctionKey] = aKEYEVENT_LEFT;
        m_keyMap[NSRightArrowFunctionKey] = aKEYEVENT_RIGHT;
        m_keyMap[NSDownArrowFunctionKey] = aKEYEVENT_DOWN;
  
        m_keyMap[NSBackspaceCharacter] = aKEYEVENT_BACKSPACE;
        m_keyMap[NSDeleteFunctionKey] = aKEYEVENT_DELETE;
       // m_keyMap[NSDeleteFunctionKey] = aKEYEVENT_BACKSPACE;
        
        m_keyMap[NSHomeFunctionKey] = aKEYEVENT_HOME;
        m_keyMap[NSEndFunctionKey] = aKEYEVENT_END;
        m_keyMap[NSPageUpFunctionKey] = aKEYEVENT_PGUP;
        m_keyMap[NSPageDownFunctionKey] = aKEYEVENT_PGDOWN;*/

        if (status == aMASH_FAILED)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                              "Failed to create Mac device. Check the video settings used.", 
                              "CMashMacDevice::Initialise");
        }
        else
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
                              "Mac device created.", 
                              "CMashMacDevice::Initialise");
        }

        return status;
	}
    
    eMASH_STATUS CMashMacDevice::LoadComponents(const sMashDeviceSettings &settings)
	{
		if (CMashDevice::LoadComponents(settings) == aMASH_FAILED)
			return aMASH_FAILED;
        
		m_keyboardMouseControllerID = m_pInputManager->_CreateController(aINPUTCONTROLLER_KEYBOARD_MOUSE, true);
        
        return aMASH_OK;
    }
    
    void CMashMacDevice::GetWindowSize(uint32 &x, uint32 &y)const
    {
        x = m_windowSizeX;
        y = m_windowSizeY;
    }
    
    void CMashMacDevice::SetMouseCage(bool state)
    {
        //TODO
    }
    
    void CMashMacDevice::OnWindowWillMove()
    {
        MashRectangle2 rect;
        rect.left = mash::math::MinFloat();
        rect.right = mash::math::MaxFloat();
        rect.top = mash::math::MinFloat();
        rect.bottom = mash::math::MaxFloat();
        
        //m_pInputManager->SetMouseCage(rect);
    }
    
    void CMashMacDevice::OnWindowMoved()
    {
        [m_oglContext update];
        UpdateWindowFrameAndMouseCage();
    }
    
    void CMashMacDevice::OnResize()
    {
        [m_oglContext update];
        NSRect windowFrame = [m_window contentRectForFrameRect:[m_window frame]];
        //update the renderer
        m_pRenderer->OnResolutionChange(windowFrame.size.width, windowFrame.size.height);
        
        UpdateWindowFrameAndMouseCage();
    }
    
    void CMashMacDevice::UpdateWindowFrame()
    {

        NSRect windowFrame = [m_window contentRectForFrameRect:[m_window frame]];
        int32 screenHeight = [[[NSScreen screens] objectAtIndex:0] frame].size.height;
        m_windowSizeX = windowFrame.size.width;
        m_windowSizeY = windowFrame.size.height;
        m_windowOriginX = windowFrame.origin.x;
    
        /*
            NSWindow takes Y origin as the distance from the bottom of the screen
            to the bottom of the window. We want the opposite.
         */
        m_windowOriginY = screenHeight - m_windowSizeY - windowFrame.origin.y;

    }
    
    void CMashMacDevice::UpdateWindowFrameAndMouseCage()
    {
        UpdateWindowFrame();
        
        //update the mouse cage
        MashRectangle2 rect;
        rect.left = m_windowOriginX;
        rect.right = rect.left + m_windowSizeX;
        rect.top = m_windowOriginY;
        rect.bottom = rect.top + m_windowSizeY;
        
        //if (m_pInputManager)
        //    m_pInputManager->SetMouseCage(rect);
    }
    
    void CMashMacDevice::SyncInputDeviceWithCurrentState()
    {
        //TODO : Make sure the window is valid
        m_lastMousePos = [NSEvent mouseLocation];
        m_lastMousePos = [m_window convertScreenToBase:m_lastMousePos];
        m_lastMousePos.y = m_windowSizeY - m_lastMousePos.y;
        
        int32 x = m_lastMousePos.x;
        int32 y = m_lastMousePos.y;
        m_pInputManager->_SetCursorPosition(x, y);
        
        UpdateWindowFrameAndMouseCage();
    }
    
    bool CMashMacDevice::ProcessKeyEvent(NSEvent *event, int8 pressed)
    {
        sInputEvent newEvent;
        newEvent.character = 0;
        newEvent.action = aKEYEVENT_NONE;
        newEvent.controllerID = m_keyboardMouseControllerID;
        newEvent.eventType = sInputEvent::aEVENTTYPE_KEYBOARD;
        newEvent.value = (f32)pressed;
        newEvent.isPressed = pressed;
        
            NSString *keyStr = [event characters];
            if ((keyStr != nil) && ([keyStr length] > 0))
            {
                uint32 c = [keyStr characterAtIndex:0];
                //look for standard ascii characters first
                if (c >= 0x20  && c <= 0x7E)
                {
                    newEvent.character = c;
                    newEvent.action = (eINPUT_EVENT)toupper(c);
                }
                else
                {
                    //handle special case
                   /* if (c == NSEnterCharacter)
                    {
                        newEvent.inputEvent.action = aKEYEVENT_RETURN;
                        newEvent.inputEvent.character = 0;
                    }
                    else*/
                    {
                        //do slower special keys last
                        std::map<int32, eINPUT_EVENT>::iterator iter = m_keyMap.find([(NSEvent *)event keyCode]/*c*/);
                        if (iter != m_keyMap.end())
                        {
                            newEvent.action = iter->second;
                            newEvent.character = 0;
                        }
                    }
                }
            }
        
        ////TODO : Re impliment modifiers
        /*if ([event modifierFlags] & NSShiftKeyMask)
            newEvent.inputEvent.modifiers |= aKEYMOD_SHIFT;
        if ([event modifierFlags] & NSControlKeyMask)
            newEvent.inputEvent.modifiers |= aKEYMOD_CTRL;
        if ([event modifierFlags] & MASHlternateKeyMask)
            newEvent.inputEvent.modifiers |= aKEYMOD_ALT;*/
        
        //if ((newEvent.inputEvent.key != aKEY_NONE)/* || (newEvent.inputEvent.modifiers != 0)//*/)
        {
            m_pInputManager->ImmediateBroadcast(newEvent);
            return true;
        }
        
        return false;
    }
    
	bool CMashMacDevice::PollMessages()
	{
        NSEvent *event = nil;
		//do
       // {
            event = [MASHpp nextEventMatchingMask:MASHnyEventMask untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
            
            bool passOnMsg = true;
            if (event != nil)
            {
                switch([event type])
                {
                    case NSKeyDown:
                    {
                        if (ProcessKeyEvent(event, 1))
                            passOnMsg = false;
                        
                        break;
                    }
                    case NSKeyUp:
                    {
                        if (ProcessKeyEvent(event, 0))
                            passOnMsg = false;
                        
                        break;
                    }
                    case NSFlagsChanged:
                    {
                        bool isShiftDown = ([event modifierFlags] & NSShiftKeyMask) != 0;
                        bool isControlDown = ([event modifierFlags] & NSControlKeyMask) != 0;
                        
                        sInputEvent newEvent;
                        newEvent.character = 0;
                        newEvent.action = aKEYEVENT_NONE;
                        newEvent.controllerID = m_keyboardMouseControllerID;
                        newEvent.eventType = sInputEvent::aEVENTTYPE_KEYBOARD;
                        
                        if (isShiftDown != m_isShiftDown)
                        {
                            newEvent.action = aKEYEVENT_SHIFT;
                            newEvent.value = (f32)isShiftDown;
                            newEvent.isPressed = isShiftDown;
                            m_isShiftDown = isShiftDown;
                            m_pInputManager->ImmediateBroadcast(newEvent);
                        }
                        
                        if (isControlDown != m_isControlDown)
                        {
                            newEvent.action = aKEYEVENT_CTRL;
                            newEvent.value = (f32)isControlDown;
                            newEvent.isPressed = isControlDown;
                            m_isControlDown = isControlDown;
                            m_pInputManager->ImmediateBroadcast(newEvent);
                        }
                        
                        ////TODO
                        //ProcessKeyEvent(event, 1);
                        //always pass on flag changed msgs
                        
                        //m_isControlDown
                        
                        break;
                    }
                    case NSLeftMouseDown:
                    {
                        sInputEvent newEvent;
                        newEvent.eventType = sInputEvent::aEVENTTYPE_MOUSE;
                        newEvent.controllerID = m_keyboardMouseControllerID;
                        newEvent.action = aMOUSEEVENT_B1;
                        newEvent.value = 1.0f;
                        newEvent.isPressed = 1;
                        m_pInputManager->ImmediateBroadcast(newEvent);
                        //passOnMsg = false;
                        
                        break;
                    }
                    case NSLeftMouseUp:
                    {
                        sInputEvent newEvent;
                        newEvent.eventType = sInputEvent::aEVENTTYPE_MOUSE;
                        newEvent.controllerID = m_keyboardMouseControllerID;
                        newEvent.action = aMOUSEEVENT_B1;
                        newEvent.value = 0.0f;
                        newEvent.isPressed = 0;
                        m_pInputManager->ImmediateBroadcast(newEvent);
                       // passOnMsg = false;
                        
                        break;
                    }
                    case NSRightMouseDown:
                    {
                        sInputEvent newEvent;
                        newEvent.eventType = sInputEvent::aEVENTTYPE_MOUSE;
                        newEvent.controllerID = m_keyboardMouseControllerID;
                        newEvent.action = aMOUSEEVENT_B2;
                        newEvent.value = 1.0f;
                        newEvent.isPressed = 1;
                        m_pInputManager->ImmediateBroadcast(newEvent);
                       // passOnMsg = false;
                        
                        break;
                    }
                    case NSRightMouseUp:
                    {
                        sInputEvent newEvent;
                        newEvent.eventType = sInputEvent::aEVENTTYPE_MOUSE;
                        newEvent.controllerID = m_keyboardMouseControllerID;
                        newEvent.action = aMOUSEEVENT_B2;
                        newEvent.value = 0.0f;
                        newEvent.isPressed = 0;
                        m_pInputManager->ImmediateBroadcast(newEvent);
                        //passOnMsg = false;
                        
                        break;
                    }
                    case NSScrollWheel:
                    {
                       // CGFloat x = [event deltaX];
                        CGFloat y = [event deltaY];
                        
                        if (y != 0.0f)
                        {
                            sInputEvent newEvent;
                            newEvent.eventType = sInputEvent::aEVENTTYPE_MOUSE;
                            newEvent.controllerID = m_keyboardMouseControllerID;
                            newEvent.action = aMOUSEEVENT_AXISZ;
                            newEvent.isPressed = 0;
                            if (y < 0.0f)
                                newEvent.value = -1;
                            else
                                newEvent.value = 1;
                            
                            m_pInputManager->ImmediateBroadcast(newEvent);
                           // passOnMsg = false;
                        }

                        break;
                    }
                    case NSMouseMoved:
                    case NSLeftMouseDragged:
                    case NSRightMouseDragged:
                    {
                        //NSPoint absMouseLocation = [NSEvent mouseLocation];
                        
                        NSPoint mouseScreenPos;
                       // mouseScreenPos.x = (int32)[(NSEvent *)event locationInWindow].x;
                        //mouseScreenPos.y = /*m_windowSizeY - */(int32)[(NSEvent *)event locationInWindow].y;
                        mouseScreenPos = [NSEvent mouseLocation];
                        mouseScreenPos = [m_window convertScreenToBase:mouseScreenPos];
                        mouseScreenPos.y = m_windowSizeY - mouseScreenPos.y;

                        
                       /* CGPoint	screenPos;
                        
                        screenPos.x = (f32) m_windowSizeX / 2.0f;
                        screenPos.y = (f32) ((f32)m_windowSizeY) - (m_windowSizeY / 2.0f);
                        screenPos = [m_window convertBaseToScreen:screenPos];
                        screenPos.y = m_deviceDisplayHeight - screenPos.y;
                        
                        CGPoint sadsa;
                        sadsa.x = screenPos.x - m_windowOriginX;
                        sadsa.y = screenPos.y - m_windowOriginY;*/
                        
                       // mouseScreenPos = [NSEvent mouseLocation];
                       // mouseScreenPos = [m_window convertScreenToBase:mouseScreenPos];
                       // mouseScreenPos.y = m_windowSizeY - mouseScreenPos.y;
                        
                        bool skipMessage = false;
                        //if (mouseScreenPos.y < 0 || mouseScreenPos.x < 0)
                       //     skipMessage = true;
                        //if (mouseScreenPos.y > m_windowSizeY || mouseScreenPos.x > m_windowSizeX)
                         //   skipMessage = true;
                        
                        if (!skipMessage && (mouseScreenPos.x != m_lastMousePos.x))
                        {
                            sInputEvent newEvent;
                            newEvent.eventType = sInputEvent::aEVENTTYPE_MOUSE;
                            newEvent.controllerID = m_keyboardMouseControllerID;
                            newEvent.action = aMOUSEEVENT_AXISX;
                            newEvent.value = mouseScreenPos.x - m_lastMousePos.x;
                            m_pInputManager->ImmediateBroadcast(newEvent);
                            m_lastMousePos.x = mouseScreenPos.x;
                            //passOnMsg = false;
                        }
                        
                        if (!skipMessage && (mouseScreenPos.y != m_lastMousePos.y))
                        {
                            //float modifiedY = m_windowSizeY - mouseScreenPos.y;
                            sInputEvent newEvent;
                            newEvent.eventType = sInputEvent::aEVENTTYPE_MOUSE;
                            newEvent.controllerID = m_keyboardMouseControllerID;
                            newEvent.action = aMOUSEEVENT_AXISY;
                            newEvent.value = mouseScreenPos.y - m_lastMousePos.y;
                            m_pInputManager->ImmediateBroadcast(newEvent);
                            m_lastMousePos.y = mouseScreenPos.y;
                            //passOnMsg = false;
                        }
                        
                        if (!skipMessage && m_isMouseLockedToCenter)
                            SendMouseToScreenCenter();
                        
                        //if (ProcessMouseEvent(event, AeroEventDispatch:://aINPUT_ONMOUSE_MOVE, aMOUSE_KEY_COUNT))
                         //   passOnMsg = false;

                        break;
                    }
                    default:
                        //[MASHpp sendEvent:event];
                        break;
                };
                
                //pass on the event
                if (passOnMsg)
                    [MASHpp sendEvent:event];
            }
            
       // }while(event != nil);
        
		return ([[MASHpp delegate] quitApp]);
	}
    
    void CMashMacDevice::SendMouseToScreenCenter()
    {
        CGPoint	screenPos;


        screenPos.x = (f32) m_windowSizeX / 2.0f;
        screenPos.y = (f32) ((f32)m_windowSizeY) - (m_windowSizeY / 2.0f);
        screenPos = NSPointToCGPoint([m_window convertBaseToScreen:NSPointFromCGPoint(screenPos)]);
        screenPos.y = m_deviceDisplayHeight - screenPos.y;
            
            m_lastMousePos.x = screenPos.x - m_windowOriginX;
            m_lastMousePos.y = screenPos.y - m_windowOriginY;
            
            CGSetLocalEventsSuppressionInterval(0);
            CGWarpMouseCursorPosition(screenPos);
    }
    
	void CMashMacDevice::SetWindowCaption(const int8 *text)
	{
        if (!text)
            return;
        
        NSString *title = [[NSString alloc ] initWithCString:text encoding:NSUTF8StringEncoding];
        [m_window setTitle:title];
        [title release];
	}
    
    void CMashMacDevice::HideMouseCursor(bool state)
    {
        if (IsMouseCursorHidden() != state)
        {
            m_mouseCursorHidden = state;
            
            //CGDirectDisplayID display;
            
            //display = CGMainDisplayID();
            
            if (state)
                CGDisplayHideCursor(state);
            else
                CGDisplayShowCursor(state);
        }
    }
    
    void CMashMacDevice::LockMouseToScreenCenter(bool state)
    {
        if (m_isMouseLockedToCenter != state)
        {
            m_isMouseLockedToCenter = state;
            
            if (m_isMouseLockedToCenter)
                SendMouseToScreenCenter();
        }
    }
    
    void CMashMacDevice::Sleep(uint32 ms)const
    {
        struct timespec ts;
        if (ms > 0)
        {
            ts.tv_sec = (time_t)(ms / 1000);
            ts.tv_nsec = (long)(ms % 1000) * 1000000;
        }
        else
        {
            ts.tv_sec = 0;
            ts.tv_nsec = 1;
        }
        
        nanosleep(&ts, NULL);
    }
    
    /*
        The apple device is built using objective C, while
     the ogl renderer is built in c++. Some calls do not like
     to be mixe, hence why we have a final draw call here.
     */
    void CMashMacDevice::_Draw()
    {
        if (m_cglContext != nil)
        {
            CGLFlushDrawable(m_cglContext);
        }
    }
    
	_MASH_EXPORT MashDevice* CreateDevice (sMashDeviceSettings &settings)
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
        
		CMashMacDevice *device = MASH_NEW_COMMON CMashMacDevice(settings.debugFilePath);
        
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
