//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashLinuxDevice.h"

#include "MashCompileSettings.h"
#ifdef MASH_LINUX

#include "MashVideo.h"
#include "MashGUIManager.h"
#include "CMashInputManager.h"
#include "MashLog.h"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <unistd.h>

namespace
{
    mash::f32 g_invShort = 1.0f / 32767.0f;
    const char* wmDeleteWindow = "WM_DELETE_WINDOW";
}

namespace mash
{
    static bool g_mash_ctxErrorOccurred = false;
    static int mash_ctxErrorHandler( Display *dpy, XErrorEvent *ev )
    {
        g_mash_ctxErrorOccurred = true;
        return 0;
    }

    bool CMashLinuxDevice::isExtensionSupported(const char *extList, const char *extension)
    {

      const char *start;
      const char *where, *terminator;

      /* Extension names should not have spaces. */
      where = strchr(extension, ' ');
      if ( where || *extension == '\0' )
        return false;

      /* It takes a bit of care to be fool-proof about parsing the
         OpenGL extensions string. Don't be fooled by sub-strings,
         etc. */
      for ( start = extList; ; ) {
        where = strstr( start, extension );

        if ( !where )
          break;

        terminator = where + strlen( extension );

        if ( where == start || *(where - 1) == ' ' )
          if ( *terminator == ' ' || *terminator == '\0' )
            return true;

        start = terminator;
      }

      return false;
    }

    CMashLinuxDevice::CMashLinuxDevice(const MashStringc &debugFilePath):
        CMashDevice(debugFilePath),
        m_keyboardMouseControllerID(0),
        m_windowSizeX(0), m_windowSizeY(0),
        m_lastMousePosX(0), m_lastMousePosY(0),
        m_mouseLeftWindow(false),
        m_mouseCursorHidden(false), m_lockMouse(false),
        m_invsCursor(0), m_defaultScreen(-1), m_grabMouseWarp(false),
        m_windowHasFocus(true)
    {
        //memset(m_joystickButtonMap, 0, sizeof(m_joystickButtonMap));
        //memset(m_joystickAxisMap, 0, sizeof(m_joystickAxisMap));
    }

    CMashLinuxDevice::~CMashLinuxDevice()
    {
        InitFullscreen(true);

        XFreeCursor(m_glXdisplay, m_invsCursor);

        if (m_glXdisplay)
            glXMakeCurrent(m_glXdisplay, 0, 0);

        if (m_glXdisplay && m_glXctx)
            glXDestroyContext(m_glXdisplay, m_glXctx);

        if (m_glXdisplay && m_glXwin)
            XDestroyWindow(m_glXdisplay, m_glXwin);

        if (m_glXdisplay && m_glXcmap)
            XFreeColormap(m_glXdisplay, m_glXcmap);

        if (m_glXdisplay)
            XCloseDisplay(m_glXdisplay);

        for (uint32 j = 0; j < m_joysticks.Size(); ++j)
        {
            if (m_joysticks[j].id >= 0)
            {
                close(m_joysticks[j].id);
            }
        }
    }

    void CMashLinuxDevice::InitFullscreen(bool reset)
    {
        if (!m_isFullscreen)
            return;

        if (reset)
        {
            if (m_isFullscreen && (m_defaultScreen != -1))
            {
                XF86VidModeSwitchToMode(m_glXdisplay, m_defaultScreen, &m_oldVideoMode);
                XF86VidModeSetViewPort(m_glXdisplay, m_defaultScreen, 0, 0);
            }

            return;
        }

        MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
                            "CMashLinuxDevice::Initialise",
                            "Attempting to start fullscreen mode using XF86 width '%i' height '%i'.",
                            m_windowSizeX, m_windowSizeY);

        int eventbase;
        int errorbase;
        int bestMode = -1;
        int defaultDepth = DefaultDepth(m_glXdisplay,m_defaultScreen);

        if (XF86VidModeQueryExtension(m_glXdisplay, &eventbase, &errorbase))
        {
            // enumerate video modes
            int modeCount;
            XF86VidModeModeInfo** modes;

            XF86VidModeGetAllModeLines(m_glXdisplay, m_defaultScreen, &modeCount, &modes);

            // save current video mode
            m_oldVideoMode = *modes[0];

            // find fitting mode
            for (int i = 0; i<modeCount; ++i)
            {
                if ((bestMode == -1) &&
                        (modes[i]->hdisplay >= m_windowSizeX) &&
                        (modes[i]->vdisplay >= m_windowSizeY))
                {
                    bestMode = i;
                }
                else if (bestMode!=-1 &&
                        modes[i]->hdisplay >= m_windowSizeX &&
                        modes[i]->vdisplay >= m_windowSizeY &&
                        modes[i]->hdisplay <= modes[bestMode]->hdisplay &&
                        modes[i]->vdisplay <= modes[bestMode]->vdisplay)
                {
                    bestMode = i;
                }
            }

            XFree(modes);

            if (bestMode != -1)
            {
                MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
                                    "CMashLinuxDevice::Initialise",
                                    "Fullscreen mode using XF86 width '%i' height '%i'.",
                                    modes[bestMode]->hdisplay, modes[bestMode]->vdisplay);

                XF86VidModeSwitchToMode(m_glXdisplay, m_defaultScreen, modes[bestMode]);
                XF86VidModeSetViewPort(m_glXdisplay, m_defaultScreen, 0, 0);
            }
            else
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                                "Failed to start fullscreen mode.",
                                 "CMashLinuxDevice::Initialise");

                m_isFullscreen = false;
            }
        }
    }

    eMASH_STATUS CMashLinuxDevice::Initialise(const mash::sMashDeviceSettings &settings)
    {
        #define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
        #define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
        typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

        m_glXdisplay = XOpenDisplay(0);

        if (!m_glXdisplay)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                            "Failed to get X display.",
                             "CMashLinuxDevice::Initialise");

            return aMASH_FAILED;
        }

        m_defaultScreen = DefaultScreen(m_glXdisplay);

        int colourBits = 8;
        int depthBits = 24;

        if (settings.backbufferFormat == aBACKBUFFER_FORMAT_16BIT)
            colourBits = 4;
        else
            colourBits = 8;

        if (settings.depthFormat == aDEPTH_FORMAT_32BIT)
            depthBits = 32;
        else
            depthBits = 24;

        int multisampleLevels = 0;
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
        default:
            multisampleLevels = 0;
        }

        m_windowSizeX = settings.screenWidth;
        m_windowSizeY = settings.screenHeight;
        m_isFullscreen = settings.fullScreen;

        InitFullscreen(false);

//        if (m_isFullscreen)
//        {
//            Window win = DefaultRootWindow(m_glXdisplay);
//            XWindowAttributes getWinAttr;
//            XGetWindowAttributes(m_glXdisplay, win, &getWinAttr);
//            m_windowSizeX = getWinAttr.width;
//            m_windowSizeY = getWinAttr.height;
//        }

        // Get a matching FB config GLX_WINDOW_BIT,
        static int visual_attribs[] =
        {
            GLX_X_RENDERABLE    , True,
            GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
            GLX_RENDER_TYPE     , GLX_RGBA_BIT,
            GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
            GLX_RED_SIZE        , colourBits,
            GLX_GREEN_SIZE      , colourBits,
            GLX_BLUE_SIZE       , colourBits,
            GLX_ALPHA_SIZE      , colourBits,
            GLX_DEPTH_SIZE      , depthBits,
            GLX_STENCIL_SIZE    , 8,
            GLX_DOUBLEBUFFER    , True,
            //GLX_SAMPLE_BUFFERS  , (multisampleLevels > 1)?1:0,
           // GLX_SAMPLES         , multisampleLevels,
            None
        };

        int glx_major = 0;
        int glx_minor = 0;

        if (!glXQueryVersion(m_glXdisplay, &glx_major, &glx_minor) ||
            ((glx_major == 1) && (glx_minor < 3) ) || (glx_major < 1))
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                            "GLX version must be 1.3 or greater.",
                             "CMashLinuxDevice::Initialise");

            return aMASH_FAILED;
        }

        int fbcount;
        GLXFBConfig *fbc = glXChooseFBConfig(m_glXdisplay, DefaultScreen( m_glXdisplay ),
                                visual_attribs, &fbcount);
        if (!fbc || (fbcount < 1))
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                            "Failed to choose framebuffer configuration.",
                             "CMashLinuxDevice::Initialise");

            return aMASH_FAILED;
        }

        // Pick the FB config/visual with the most samples per pixel
        int best_fbc = -1, worst_fbc = -1, best_num_samp = -1, worst_num_samp = 999;

        for (int i = 0; i < fbcount; i++)
        {
            XVisualInfo *vi = glXGetVisualFromFBConfig(m_glXdisplay, fbc[i]);
            if (vi)
            {
                int samp_buf, samples;
                glXGetFBConfigAttrib(m_glXdisplay, fbc[i], GLX_SAMPLE_BUFFERS, &samp_buf);
                glXGetFBConfigAttrib(m_glXdisplay, fbc[i], GLX_SAMPLES , &samples);

//                int pixw = 0;
//                int pixh = 0;
//                glXGetFBConfigAttrib(m_glXdisplay, fbc[i], GLX_MAX_PBUFFER_WIDTH, &pixw);
//                glXGetFBConfigAttrib(m_glXdisplay, fbc[i], GLX_MAX_PBUFFER_HEIGHT , &pixh);
//                std::cerr << "width:" << pixw << std::endl;
//                std::cerr << "height:" << pixh << std::endl;

//                printf( "  Matching fbconfig %d, visual ID 0x%2x: SAMPLE_BUFFERS = %d,"
//                        " SAMPLES = %d\n",
//                        i, vi -> visualid, samp_buf, samples );

                if ((best_fbc < 0) || (samp_buf) && (samples > best_num_samp))
                    best_fbc = i, best_num_samp = samples;
                if ((worst_fbc < 0) || (!samp_buf) || (samples < worst_num_samp))
                    worst_fbc = i, worst_num_samp = samples;
            }
            XFree( vi );
        }

        GLXFBConfig bestFbc = fbc[best_fbc];

        // Be sure to free the FBConfig list allocated by glXChooseFBConfig()
        XFree(fbc);

        // Get a visual
        XVisualInfo *vi = glXGetVisualFromFBConfig( m_glXdisplay, bestFbc );

        //printf( "Chosen visual ID = 0x%x\n", vi->visualid );

        //create colour map
        XSetWindowAttributes swa;
        swa.colormap = m_glXcmap = XCreateColormap(m_glXdisplay,
                                              RootWindow(m_glXdisplay, vi->screen),
                                              vi->visual, AllocNone);
        swa.background_pixmap = None;
        swa.border_pixel = 0;
        swa.event_mask = StructureNotifyMask | FocusChangeMask | ExposureMask;

        //for input
        swa.event_mask |= PointerMotionMask |
                ButtonPressMask | KeyPressMask |
                ButtonReleaseMask | KeyReleaseMask;

        swa.override_redirect = settings.fullScreen;




        //Creating window
       // if (!settings.fullScreen)
        //{
            m_glXwin = XCreateWindow(m_glXdisplay, RootWindow( m_glXdisplay, vi->screen),
                                       0, 0, m_windowSizeX, m_windowSizeY, 0, vi->depth, InputOutput,
                                       vi->visual,
                                       CWBorderPixel|CWColormap|CWEventMask|CWOverrideRedirect, &swa);


            if (!m_glXwin)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                                "Failed to create window.",
                                 "CMashLinuxDevice::Initialise");

                return aMASH_FAILED;
            }


            XMapRaised(m_glXdisplay, m_glXwin);



            XStoreName(m_glXdisplay, m_glXwin, "Mash Window");

            //Mapping window
            XMapWindow(m_glXdisplay, m_glXwin);

            Atom wmDelete = XInternAtom(m_glXdisplay, wmDeleteWindow, True);
            XSetWMProtocols(m_glXdisplay, m_glXwin, &wmDelete, 1);

            XFlush(m_glXdisplay);
        //}
//        else
//        {
//            int pbAttribs[] =
//            {
//                  GLX_PBUFFER_WIDTH, settings.screenWidth,
//                  GLX_PBUFFER_HEIGHT, settings.screenHeight,
//                  GLX_LARGEST_PBUFFER, False,
//                  GLX_PRESERVED_CONTENTS, False,
//                  None
//            };

//            //vi = glXGetVisualFromFBConfig(m_glXdisplay, bestFbc);

//            m_glXwin = glXCreatePbuffer(m_glXdisplay, bestFbc, pbAttribs);

//            if (m_glXwin==None)
//            {
//                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
//                                "Failed to create the default OpenGL Pbuffer.",
//                                 "CMashLinuxDevice::Initialise");

//              return aMASH_FAILED;
//            }

//            GrabInput();
//        }

        // Done with the visual info data
        XFree(vi);

        if (m_isFullscreen)
            GrabInput();

        // Get the default screen's GLX extension list
        const char *glxExts = glXQueryExtensionsString(m_glXdisplay,
                                                       DefaultScreen(m_glXdisplay));

        // NOTE: It is not necessary to create or make current to a context before
        // calling glXGetProcAddressARB
        glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
        glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
                glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );



        m_glXctx = 0;

        // Install an X error handler so the application won't exit if GL 3.0
        // context allocation fails.
        //
        // Note this error handler is global.  All display connections in all threads
        // of a process use the same error handler, so be sure to guard against other
        // threads issuing X commands while this code is running.
        g_mash_ctxErrorOccurred = false;
        int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&mash_ctxErrorHandler);

//        // Check for the GLX_ARB_create_context extension string and the function.
//        // If either is not present, use GLX 1.3 context creation method.
//        if (!isExtensionSupported(glxExts, "GLX_ARB_create_context") || !glXCreateContextAttribsARB)
//        {
//            //glXCreateContextAttribsARB() not found, using old-style GLX context
//            //ctx = glXCreateNewContext(display, bestFbc, GLX_RGBA_TYPE, 0, True);

//            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
//                            "glXCreateContextAttribsARB not found. Context could not be created.",
//                             "CMashLinuxDevice::Initialise");

//            return aMASH_FAILED;
//        }
//        // If it does, try to get a GL 3.1 context!
//        else
        {
            m_glXctx = 0;
            const int32 contextVersions[] = {3, 3, 3, 2, 3, 1, 3, 0, 2, 1};
            {
                for(int32 i = 0; i < sizeof(contextVersions) / sizeof(int32); i+=2)
                {
                    int context_attribs[] =
                    {
                        GLX_CONTEXT_MAJOR_VERSION_ARB, contextVersions[i],
                        GLX_CONTEXT_MINOR_VERSION_ARB, contextVersions[i+1],
                        //GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                        None
                    };

                    if (m_glXctx)
                    {
                        glXDestroyContext(m_glXdisplay, m_glXctx);
                    }

                    g_mash_ctxErrorOccurred = false;
                    m_glXctx = 0;

                    //Creating context
                    m_glXctx = glXCreateContextAttribsARB(m_glXdisplay, bestFbc, 0,
                                                     True, context_attribs);

                    // Sync to ensure any errors generated are processed.
                    XSync(m_glXdisplay, False);
                    if (!g_mash_ctxErrorOccurred && m_glXctx)
                    {
                        //context created
                        MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
                                            "CMashLinuxDevice::Initialise",
                                            "OpenGL %i.%i context created.",
                                            contextVersions[i], contextVersions[i+1]);

                        break;
                    }
                    else
                    {
                        MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
                                            "CMashLinuxDevice::Initialise",
                                            "The current system cannot create an OpenGL %i.%i context. A lower version will be attempted.",
                                            contextVersions[i], contextVersions[i+1]);
                    }
                }
            }

            // Sync to ensure any errors generated are processed.
            XSync(m_glXdisplay, False);
            if (!g_mash_ctxErrorOccurred && m_glXctx)
            {
                //context created
            }
            else
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                                "The current system cannot create an OpenGL context. OpenGL context creation failed.",
                                 "CMashLinuxDevice::Initialise");

                return aMASH_FAILED;
            }
        }

        // Sync to ensure any errors generated are processed.
        XSync(m_glXdisplay, False);

        // Restore the original error handler
        XSetErrorHandler(oldHandler);

        if (g_mash_ctxErrorOccurred || !m_glXctx)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                            "Failed to create an OpenGL context.",
                             "CMashLinuxDevice::Initialise");

            return aMASH_FAILED;
        }

        // Verifying that context is a direct context
        if (!glXIsDirect (m_glXdisplay, m_glXctx))
        {
            //printf( "Indirect GLX rendering context obtained\n" );
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING,
                            "Indirect GLX rendering is being used. This may cause problems. Direct rendering should be used.",
                             "CMashLinuxDevice::Initialise");
        }
        else
        {
            //printf( "Direct GLX rendering context obtained\n" );
        }

        //Making context current
        //glXMakeCurrent(m_glXdisplay, m_glXwin, m_glXctx);
        if (!glXMakeCurrent(m_glXdisplay, m_glXwin, m_glXctx))
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                            "Failed to make the rendering area current.",
                             "CMashLinuxDevice::Initialise");

          return aMASH_FAILED;
        }



//        if (!glXMakeCurrent(m_glXdisplay, pbuffer, m_glXctx))
//        {
//            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
//                            "Failed to set the default OpenGL Pbuffer.",
//                             "CMashLinuxDevice::Initialise");

//          return aMASH_FAILED;
//        }

        //XFree(vi);

        // glXSwapIntervalEXTProc glXSwapIntervalEXT = 0;
         typedef void (*swapIntervalProc)(int);
        typedef void (*glXSwapIntervalEXTProc)(Display *dpy,
                                                GLXDrawable drawable,
                                                int interval);

        swapIntervalProc swapInterval = 0;
        glXSwapIntervalEXTProc glXSwapIntervalEXT = 0;

         if (isExtensionSupported(glxExts, "GLX_MESA_swap_control"))
           swapInterval = (swapIntervalProc) glXGetProcAddress((const GLubyte*) "glXSwapIntervalMESA");
         else if (isExtensionSupported(glxExts, "GLX_SGI_swap_control"))
           swapInterval = (swapIntervalProc) glXGetProcAddress((const GLubyte*) "glXSwapIntervalSGI");

         if (!swapInterval)
         {
            if (isExtensionSupported(glxExts, "GLX_EXT_swap_control"))
            {
                typedef void (*glXSwapIntervalEXTProc)(Display *dpy,
                                                        GLXDrawable drawable,
                                                        int interval);

                glXSwapIntervalEXT = (glXSwapIntervalEXTProc)glXGetProcAddressARB((const GLubyte *) "glXSwapIntervalEXT");
            }
         }

         if (glXSwapIntervalEXT || swapInterval)
         {
             if (settings.enableVSync)
             {
                 if (swapInterval)
                    swapInterval(1);
                 else
                     glXSwapIntervalEXT(m_glXdisplay, m_glXwin, 1);
             }
             else
             {
                 if (swapInterval)
                    swapInterval(0);
                 else
                     glXSwapIntervalEXT(m_glXdisplay, m_glXwin, 0);
             }
         }
         else
         {
             MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING,
                      "glXSwapInterval not defined. Vsync control not avaliable.",
                       "CMashLinuxDevice::Initialise");
         }

        MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION,
                          "Linux device created.",
                          "CMashLinuxDevice::Initialise");

        return aMASH_OK;
    }

    eMASH_STATUS CMashLinuxDevice::LoadComponents(const sMashDeviceSettings &settings)
    {
        if (CMashDevice::LoadComponents(settings) == aMASH_FAILED)
            return aMASH_FAILED;

        m_keyboardMouseControllerID = m_pInputManager->_CreateController(aINPUTCONTROLLER_KEYBOARD_MOUSE, true);

        /////////////////////////////////////////////////////////
        /// init input

//         /* XInput Extension available? */
//         int opcode, event, error;
//         if (!XQueryExtension(dpy, "XInputExtension", &opcode, &event, &error))
//         {
//            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
//                              "XInput extention not found. Make sure you have the XInput packages installed.",
//                              "CMashLinuxDevice::Initialise");

//            return aMASH_FAILED;
//         }

//         /* Which version of XI2? We support 2.0 */
//         int major = 2, minor = 0;
//         if (XIQueryVersion(dpy, &major, &minor) == BadRequest)
//         {
//           printf("XI2 not available. Server supports %d.%d\n", major, minor);
//           MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
//                               "CMashLinuxDevice::Initialise",
//                             "XInput2 not available. Version found: %d.%d.",
//                             major, minor);

//           return aMASH_FAILED;
//         }

//         XIEventMask eventmask;
//         unsigned char mask[1] = { 0 }; /* the actual mask */

//         eventmask.deviceid = 2;
//         eventmask.mask_len = sizeof(mask); /* always in bytes */
//         eventmask.mask = mask;
//         /* now set the mask */
//         XISetMask(mask, XI_ButtonPress);
//         XISetMask(mask, XI_Motion);
//         XISetMask(mask, XI_KeyPress);

//         /* select on the window */
//         XISelectEvents(m_glXdisplay, m_glXwin, &eventmask, 1);
        m_keyMap[XK_F1] = aKEYEVENT_F1;
        m_keyMap[XK_F2] = aKEYEVENT_F2;
        m_keyMap[XK_F3] = aKEYEVENT_F3;
        m_keyMap[XK_F4] = aKEYEVENT_F4;
        m_keyMap[XK_F5] = aKEYEVENT_F5;
        m_keyMap[XK_F6] = aKEYEVENT_F6;
        m_keyMap[XK_F7] = aKEYEVENT_F7;
        m_keyMap[XK_F8] = aKEYEVENT_F8;
        m_keyMap[XK_F9] = aKEYEVENT_F9;
        m_keyMap[XK_F10] = aKEYEVENT_F10;
        m_keyMap[XK_F11] = aKEYEVENT_F11;
        m_keyMap[XK_F12] = aKEYEVENT_F12;
        m_keyMap[XK_Escape] = aKEYEVENT_ESCAPE;
        m_keyMap[XK_Tab] = aKEYEVENT_TAB;
        m_keyMap[XK_Return] = aKEYEVENT_RETURN;
        m_keyMap[XK_Left] = aKEYEVENT_LEFT;
        m_keyMap[XK_Right] = aKEYEVENT_RIGHT;
        m_keyMap[XK_Up] = aKEYEVENT_UP;
        m_keyMap[XK_Down] = aKEYEVENT_DOWN;
        m_keyMap[XK_Page_Up] = aKEYEVENT_PGUP;
        m_keyMap[XK_Page_Down] = aKEYEVENT_PGDOWN;
        m_keyMap[XK_Delete] = aKEYEVENT_DELETE;
        m_keyMap[XK_BackSpace] = aKEYEVENT_BACKSPACE;
        m_keyMap[XK_Home] = aKEYEVENT_HOME;
        m_keyMap[XK_End] = aKEYEVENT_END;
        m_keyMap[XK_Shift_L] = aKEYEVENT_SHIFT;
        m_keyMap[XK_Shift_R] = aKEYEVENT_SHIFT;
        m_keyMap[XK_Control_L] = aKEYEVENT_CTRL;
        m_keyMap[XK_Control_R] = aKEYEVENT_CTRL;
        m_keyMap[XK_Menu] = aKEYEVENT_MENU;

        //the following keys are mapped to their ASCII values
        m_keyMap[XK_space] = aKEYEVENT_SPACE;
        m_keyMap[XK_exclam] = aKEYEVENT_EXCLAM;
        m_keyMap[XK_quotedbl] = aKEYEVENT_QUOTE;
        m_keyMap[XK_numbersign] = aKEYEVENT_HASH;
        m_keyMap[XK_ampersand] = aKEYEVENT_AMPERSAND;
        m_keyMap[XK_percent] = aKEYEVENT_PERCENT;
        m_keyMap[XK_dollar] = aKEYEVENT_DOLLAR;
        m_keyMap[XK_apostrophe] = aKEYEVENT_APOSTROPHE;
        m_keyMap[XK_parenleft] = aKEYEVENT_LEFTPAREN;
        m_keyMap[XK_parenright] = aKEYEVENT_RIGHTPARAN;
        m_keyMap[XK_asterisk] = aKEYEVENT_ASTERISK;
        m_keyMap[XK_plus] = aKEYEVENT_PLUS;
        m_keyMap[XK_comma] = aKEYEVENT_COMMA;
        m_keyMap[XK_minus] = aKEYEVENT_MINUS;
        m_keyMap[XK_period] = aKEYEVENT_PERIOD;
        m_keyMap[XK_slash] = aKEYEVENT_SLASH;
        m_keyMap[XK_0] = aKEYEVENT_0;
        m_keyMap[XK_1] = aKEYEVENT_1;
        m_keyMap[XK_2] = aKEYEVENT_2;
        m_keyMap[XK_3] = aKEYEVENT_3;
        m_keyMap[XK_4] = aKEYEVENT_4;
        m_keyMap[XK_5] = aKEYEVENT_5;
        m_keyMap[XK_6] = aKEYEVENT_6;
        m_keyMap[XK_7] = aKEYEVENT_7;
        m_keyMap[XK_8] = aKEYEVENT_8;
        m_keyMap[XK_9] = aKEYEVENT_9;
        m_keyMap[XK_colon] = aKEYEVENT_COLON;
        m_keyMap[XK_semicolon] = aKEYEVENT_SEMICOLON;
        m_keyMap[XK_less] = aKEYEVENT_LESS;
        m_keyMap[XK_equal] = aKEYEVENT_EQUALS;
        m_keyMap[XK_greater] = aKEYEVENT_GREATER;
        m_keyMap[XK_question] = aKEYEVENT_QUESTION;
        m_keyMap[XK_at] = aKEYEVENT_AT;
        m_keyMap[XK_A] = aKEYEVENT_A;
        m_keyMap[XK_B] = aKEYEVENT_B;
        m_keyMap[XK_C] = aKEYEVENT_C;
        m_keyMap[XK_D] = aKEYEVENT_D;
        m_keyMap[XK_E] = aKEYEVENT_E;
        m_keyMap[XK_F] = aKEYEVENT_F;
        m_keyMap[XK_G] = aKEYEVENT_G;
        m_keyMap[XK_H] = aKEYEVENT_H;
        m_keyMap[XK_I] = aKEYEVENT_I;
        m_keyMap[XK_J] = aKEYEVENT_J;
        m_keyMap[XK_K] = aKEYEVENT_K;
        m_keyMap[XK_L] = aKEYEVENT_L;
        m_keyMap[XK_M] = aKEYEVENT_M;
        m_keyMap[XK_N] = aKEYEVENT_N;
        m_keyMap[XK_O] = aKEYEVENT_O;
        m_keyMap[XK_P] = aKEYEVENT_P;
        m_keyMap[XK_Q] = aKEYEVENT_Q;
        m_keyMap[XK_R] = aKEYEVENT_R;
        m_keyMap[XK_S] = aKEYEVENT_S;
        m_keyMap[XK_T] = aKEYEVENT_T;
        m_keyMap[XK_U] = aKEYEVENT_U;
        m_keyMap[XK_V] = aKEYEVENT_V;
        m_keyMap[XK_W] = aKEYEVENT_W;
        m_keyMap[XK_X] = aKEYEVENT_X;
        m_keyMap[XK_Y] = aKEYEVENT_Y;
        m_keyMap[XK_Z] = aKEYEVENT_Z;
        m_keyMap[XK_a] = aKEYEVENT_A;
        m_keyMap[XK_b] = aKEYEVENT_B;
        m_keyMap[XK_c] = aKEYEVENT_C;
        m_keyMap[XK_d] = aKEYEVENT_D;
        m_keyMap[XK_e] = aKEYEVENT_E;
        m_keyMap[XK_f] = aKEYEVENT_F;
        m_keyMap[XK_g] = aKEYEVENT_G;
        m_keyMap[XK_h] = aKEYEVENT_H;
        m_keyMap[XK_i] = aKEYEVENT_I;
        m_keyMap[XK_j] = aKEYEVENT_J;
        m_keyMap[XK_k] = aKEYEVENT_K;
        m_keyMap[XK_l] = aKEYEVENT_L;
        m_keyMap[XK_m] = aKEYEVENT_M;
        m_keyMap[XK_n] = aKEYEVENT_N;
        m_keyMap[XK_o] = aKEYEVENT_O;
        m_keyMap[XK_p] = aKEYEVENT_P;
        m_keyMap[XK_q] = aKEYEVENT_Q;
        m_keyMap[XK_r] = aKEYEVENT_R;
        m_keyMap[XK_s] = aKEYEVENT_S;
        m_keyMap[XK_t] = aKEYEVENT_T;
        m_keyMap[XK_u] = aKEYEVENT_U;
        m_keyMap[XK_v] = aKEYEVENT_V;
        m_keyMap[XK_w] = aKEYEVENT_W;
        m_keyMap[XK_x] = aKEYEVENT_X;
        m_keyMap[XK_y] = aKEYEVENT_Y;
        m_keyMap[XK_z] = aKEYEVENT_Z;
        m_keyMap[XK_bracketleft] = aKEYEVENT_LEFTBRACKET;
        m_keyMap[XK_backslash] = aKEYEVENT_BACKSLASH;
        m_keyMap[XK_bracketright] = aKEYEVENT_RIGHTBRACKET;
        m_keyMap[XK_asciicircum] = aKEYEVENT_CARET;
        m_keyMap[XK_underscore] = aKEYEVENT_UNDERSCORE;
        m_keyMap[XK_grave] = aKEYEVENT_GRAVE ;
        m_keyMap[XK_braceleft] = aKEYEVENT_LBRACE;
        m_keyMap[XK_brokenbar] = aKEYEVENT_PIPE;
        m_keyMap[XK_braceright] = aKEYEVENT_RBRACE;
        m_keyMap[XK_asciitilde] = aKEYEVENT_TILDE ;

        for(uint32 js = 0; js < 20; ++js)
        {
            MashStringc devName = "/dev/js";
            devName += MashStringc::CreateFrom(js);

            sJoystick newJoystick;

            newJoystick.id = open(devName.GetCString(), O_RDONLY);
            if (newJoystick.id == -1)
            {
                devName = "/dev/input/js";
                devName += MashStringc::CreateFrom(js);
                newJoystick.id = open(devName.GetCString(), O_RDONLY);
                if (newJoystick.id == -1)
                {
                    devName = "/dev/joy";
                    devName += MashStringc::CreateFrom(js);
                    newJoystick.id = open(devName.GetCString(), O_RDONLY);
                }
            }

            if (newJoystick.id == -1)
               continue;


            ioctl(newJoystick.id, JSIOCGAXES, &(newJoystick.axis));
            ioctl(newJoystick.id, JSIOCGBUTTONS, &(newJoystick.buttons));
            fcntl(newJoystick.id, F_SETFL, O_NONBLOCK);

//            newJoystick.maxAxis1 = 1.0f / 32767.0f;
//            newJoystick.maxAxis2 = 1.0f / 32767.0f;
//            newJoystick.maxAxisT1 = 1.0f / 32767.0f;
//            newJoystick.maxAxisT2 = 1.0f / 32767.0f;

            newJoystick.controllerId = GetInputManager()->_CreateController(aINPUTCONTROLLER_JOYSTICK, true);
            m_joysticks.PushBack(newJoystick);
        }

        for(uint32 js = 0; js < m_joysticks.Size(); ++js)
        {
            MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
                               "CMashLinuxDevice::Initialise",
                              "Joystick '%d' found with '%d' buttons and '%d' axes.",
                                js, m_joysticks[js].buttons, m_joysticks[js].axis);
        }

        //init inv cursor
        {
            XGCValues values;
            unsigned long valuemask = 0;

            XColor fg;

            Pixmap invisBitmap = XCreatePixmap(m_glXdisplay, m_glXwin, 32, 32, 1);
            Colormap screen_colormap = DefaultColormap(m_glXdisplay, DefaultScreen(m_glXdisplay));
            XAllocNamedColor(m_glXdisplay, screen_colormap, "black", &fg, &fg);

            GC gc = XCreateGC(m_glXdisplay, invisBitmap, valuemask, &values);

            XSetForeground(m_glXdisplay, gc, BlackPixel(m_glXdisplay, DefaultScreen(m_glXdisplay)));
            XFillRectangle(m_glXdisplay, invisBitmap, gc, 0, 0, 32, 32 );

            m_invsCursor = XCreatePixmapCursor(m_glXdisplay, invisBitmap, invisBitmap, &fg, &fg, 1, 1);
            XFreeGC(m_glXdisplay, gc);
            XFreePixmap(m_glXdisplay, invisBitmap);
        }


        return aMASH_OK;
    }

    void CMashLinuxDevice::_Draw()
    {
        glXSwapBuffers (m_glXdisplay, m_glXwin);
    }

    void CMashLinuxDevice::SetWindowCaption(const int8 *text)
    {
        if (text && (strlen(text) > 0))
            XStoreName(m_glXdisplay, m_glXwin, text);
    }

    void CMashLinuxDevice::GetWindowSize(uint32 &x, uint32 &y)const
    {
        x = m_windowSizeX;
        y = m_windowSizeY;
    }

    void CMashLinuxDevice::GrabInput()
    {
        XSetInputFocus(m_glXdisplay, m_glXwin, RevertToParent, CurrentTime);
        XGrabKeyboard(m_glXdisplay, m_glXwin, True, GrabModeAsync, GrabModeAsync, CurrentTime);
        XGrabPointer(m_glXdisplay, m_glXwin, True, ButtonPressMask, GrabModeAsync, GrabModeAsync, m_glXwin, None, CurrentTime);
    }

    void CMashLinuxDevice::UngrabDevices()
    {
        XSetInputFocus(m_glXdisplay, m_glXwin, RevertToParent, CurrentTime);
        XUngrabKeyboard(m_glXdisplay, CurrentTime);
        XUngrabPointer(m_glXdisplay, CurrentTime);
    }

    void CMashLinuxDevice::LockMouseToScreenCenter(bool state)
    {
        if (m_lockMouse != state)
        {
            m_lockMouse = state;
            if (m_lockMouse)
            {
                GrabInput();
            }
            else
            {
                if (!m_isFullscreen)
                    UngrabDevices();
            }
        }
    }

    void CMashLinuxDevice::HideMouseCursor(bool state)
    {
        if (!m_mouseCursorHidden)
            XDefineCursor(m_glXdisplay, m_glXwin, m_invsCursor);
        else
            XUndefineCursor(m_glXdisplay, m_glXwin);
    }

    void CMashLinuxDevice::WarpMouse()
    {
        if (m_lockMouse)
        {
            if (m_lastMousePosX != m_lastMouseWarpPosX || m_lastMousePosY != m_lastMouseWarpPosY)
            {
                int mx = m_windowSizeX * 0.5f;
                int my = m_windowSizeY * 0.5f;

                m_grabMouseWarp = true;
                XWarpPointer(m_glXdisplay, None, m_glXwin, 0, 0, 0, 0, mx, my);
            }
        }
    }

    void CMashLinuxDevice::BeginUpdate()
    {
        WarpMouse();
    }

    bool CMashLinuxDevice::PollMessages()
    {
        bool close = false;

        while (XPending(m_glXdisplay) > 0)
        {
            XEvent ev;
            XNextEvent(m_glXdisplay, &ev);

            switch(ev.type)
            {
            case FocusIn:
            {
                bool oldState = m_windowHasFocus;
                m_windowHasFocus = true;

                if (!oldState)
                    OnWindowFocusRegained();

                break;
            }
            case FocusOut:
            {
                bool oldState = m_windowHasFocus;
                m_windowHasFocus = false;

                if (oldState)
                    OnWindowFocusLost();

                break;
            }
            case ConfigureNotify:
            {
                if ((ev.xconfigure.width != m_windowSizeX) ||
                    (ev.xconfigure.height != m_windowSizeY))
                {
                    m_windowSizeX = ev.xconfigure.width;
                    m_windowSizeY = ev.xconfigure.height;

                    m_lastMouseWarpPosX = -1;
                    m_lastMouseWarpPosY = -1;
                    WarpMouse();

                    if (GetRenderer())
                    {
                        GetRenderer()->OnResolutionChange(m_windowSizeX, m_windowSizeY);
                    }

                    if (GetGUIManager())
                    {
                        GetGUIManager()->OnResize();
                    }
                }
                break;
            }
            case MappingNotify:
                XRefreshKeyboardMapping (&ev.xmapping) ;
                break;
                case MotionNotify:
                {
                    if (m_grabMouseWarp)
                    {
                        m_grabMouseWarp = false;
                        m_lastMousePosX = ev.xbutton.x;
                        m_lastMousePosY = ev.xbutton.y;
                        m_lastMouseWarpPosX = m_lastMousePosX;
                        m_lastMouseWarpPosY = m_lastMousePosY;
                        GetInputManager()->_SetCursorPosition(m_lastMousePosX, m_lastMousePosY);
                        break;
                    }

                    if (ev.xbutton.x == 0 && ev.xbutton.y == 0)
                    {
                        m_mouseLeftWindow = true;
                    }
                    else if (m_mouseLeftWindow)
                    {
                        m_lastMousePosX = ev.xbutton.x;
                        m_lastMousePosY = ev.xbutton.y;
                        m_mouseLeftWindow = false;
                        GetInputManager()->_SetCursorPosition(m_lastMousePosX, m_lastMousePosY);
                    }
                    else
                    {
                        if (ev.xbutton.x != m_lastMousePosX)
                        {
                            sInputEvent newEvent;
                            newEvent.eventType = mash::sInputEvent::aEVENTTYPE_MOUSE;
                            newEvent.controllerID = m_keyboardMouseControllerID;
                            newEvent.action = aMOUSEEVENT_AXISX;
                            newEvent.value = ev.xbutton.x - m_lastMousePosX;
                            GetInputManager()->ImmediateBroadcast(newEvent);
                            m_lastMousePosX = ev.xbutton.x;
                        }
                        if (ev.xbutton.y != m_lastMousePosY)
                        {
                            sInputEvent newEvent;
                            newEvent.eventType = mash::sInputEvent::aEVENTTYPE_MOUSE;
                            newEvent.controllerID = m_keyboardMouseControllerID;
                            newEvent.action = aMOUSEEVENT_AXISY;
                            newEvent.value = ev.xbutton.y - m_lastMousePosY;
                            GetInputManager()->ImmediateBroadcast(newEvent);
                            m_lastMousePosY = ev.xbutton.y;
                        }
                    }
                    break;
                }
                case ButtonPress:
                case ButtonRelease:
                {
                    sInputEvent newEvent;
                    newEvent.eventType = mash::sInputEvent::aEVENTTYPE_MOUSE;
                    newEvent.controllerID = m_keyboardMouseControllerID;

                    switch(ev.xbutton.button)
                    {
                    case Button1:
                        newEvent.action = aMOUSEEVENT_B1;
                        break;
                    case Button2:
                        newEvent.action = aMOUSEEVENT_B3;
                        break;
                    case Button3:
                        newEvent.action = aMOUSEEVENT_B2;
                        break;
                    case Button4:
                        newEvent.action = aMOUSEEVENT_AXISZ;
                        break;
                    case Button5:
                        newEvent.action = aMOUSEEVENT_AXISZ;
                        break;
                    }

                    switch(ev.xbutton.button)
                    {
                    case Button1:
                    case Button2:
                    case Button3:
                        newEvent.isPressed = (ev.type == ButtonPress)?1:0;
                        newEvent.value = (ev.type == ButtonPress)?1.0f:0.0f;
                        break;
                    case Button4:
                        newEvent.isPressed = (ev.type == ButtonPress)?1:0;
                        newEvent.value = (ev.type == ButtonPress)?1.0f:0.0f;
                        break;
                    case Button5:
                        newEvent.isPressed = (ev.type == ButtonPress)?1:0;
                        newEvent.value = (ev.type == ButtonPress)?-1.0f:0.0f;
                        break;
                    }

                    GetInputManager()->ImmediateBroadcast(newEvent);

                    break;
                }
                case KeyRelease:
                {
                    if (XPending(m_glXdisplay) > 0)
                    {
                        XEvent nev;
                        XPeekEvent (ev.xkey.display, &nev);
                        if ((nev.type == KeyPress) &&
                            (nev.xkey.keycode == ev.xkey.keycode) &&
                            (nev.xkey.time - ev.xkey.time) < 2)
                        {
                            //eat the next KeyPress message too
                            XNextEvent(m_glXdisplay, &nev);
                            //stop repeat
                            break;
                        }
                    }
                }
                case KeyPress:
                {
                    char buf[8] = {0};
                    KeySym keysym_return;
                    XLookupString(&ev.xkey, buf, sizeof(buf), &keysym_return, NULL);
                   // {
                        sInputEvent newEvent;
                        newEvent.action = aKEYEVENT_NONE;
                        newEvent.controllerID = m_keyboardMouseControllerID;
                        newEvent.eventType = mash::sInputEvent::aEVENTTYPE_KEYBOARD;

                        if (ev.type == KeyRelease)
                        {
                            newEvent.value = 0.0f;
                            newEvent.isPressed = false;
                        }
                        else
                        {
                            newEvent.value = 1.0f;
                            newEvent.isPressed = true;
                        }

                        //std::cerr << keysym_return << std::endl;
                        std::map<int32, eINPUT_EVENT>::const_iterator keyIter = m_keyMap.find(keysym_return);
                        if (keyIter != m_keyMap.end())
                        {
                            newEvent.character = buf[0];
                            newEvent.action = keyIter->second;
                            GetInputManager()->ImmediateBroadcast(newEvent);
                        }
                   // }
                        break;
                }
                case ClientMessage:
                {
                    char *atom = XGetAtomName(m_glXdisplay, ev.xclient.message_type);
                    if (*atom == *wmDeleteWindow)
                    {
                        close = true;
                    }
                    XFree(atom);
                }
            }
        }

        //        5 == rt
        //        2 == lt
        //        4 == rightthumb_y
        //        3 == rightthum_x
        //        1 == leftthumb_y
        //        0 == leftthumb_x

        //        0 = a
        //        1 = b
        //        2 = x
        //        3 = y
        //        4 = lb
        //        5 = rb
        //        6 = back
        //        7 = start
        //        8 = xbox
        //        9 = lthumb
        //         10 = rthumb


        //poll joysticks
        const uint32 joystickCount = m_joysticks.Size();
        if (joystickCount > 0)
        {
            static int32 joystickButtonMap[] = {aJOYEVENT_B6, aJOYEVENT_B7, aJOYEVENT_B8, aJOYEVENT_B9, aJOYEVENT_B4,
                                               aJOYEVENT_B5, aJOYEVENT_B1, aJOYEVENT_B0, aJOYEVENT_UNKNOWN, aJOYEVENT_B2, aJOYEVENT_B3};
            static const int32 joystichButtonMapCount = sizeof(joystickButtonMap) / sizeof(int32);

            static int32 joystickAxisMap[] = {aJOYEVENT_AXIS_1_X, aJOYEVENT_AXIS_1_Y, aJOYEVENT_THROTTLE_1, aJOYEVENT_AXIS_2_X,
                                             aJOYEVENT_AXIS_2_Y, aJOYEVENT_THROTTLE_2, aJOYEVENT_POVRIGHT, aJOYEVENT_POVUP};
            static const int32 joystickAxisMapCount = sizeof(joystickAxisMap) / sizeof(int32);

            for(uint32 js = 0; js < joystickCount; ++js)
            {
                sJoystick *currentJoystick = &m_joysticks[js];
                const sJoystickThreshold *controllerThresholds = GetInputManager()->GetControllerThresholds(currentJoystick->controllerId);

                struct js_event ev;
                while(sizeof(ev) == read(currentJoystick->id, &ev, sizeof(ev)))
                {
                    //aJOYEVENT_AXIS_2_Y aJOYEVENT_B0
                    sInputEvent newEvent;
                    newEvent.eventType = sInputEvent::aEVENTTYPE_JOYSTICK;
                    newEvent.controllerID = currentJoystick->controllerId;

                    switch(ev.type & ~JS_EVENT_INIT)
                    {
                    case JS_EVENT_BUTTON:
                        {
                            int32 buttonNum = (int32)ev.number;
                            if (buttonNum < joystichButtonMapCount)
                            {
                                newEvent.action = (eINPUT_EVENT)joystickButtonMap[buttonNum];
                            }
                            else
                            {
                                newEvent.action = aJOYEVENT_UNKNOWN;
                            }

                            //std::cerr << "button : " << buttonNum << " : "<< ev.value << std::endl;
                            newEvent.value = (ev.value)?1.0f:0.0f;
                            newEvent.isPressed = (ev.value)?true:false;
                            newEvent.character = ev.number;
                            GetInputManager()->ImmediateBroadcast(newEvent);

                            break;
                        }
                    case JS_EVENT_AXIS:
                        {
                            int32 axisNumber = (int32)ev.number;
                            int16 filteredValue = 0;
                            if (axisNumber < joystickAxisMapCount)
                            {
                                switch(joystickAxisMap[axisNumber])//flip Y axis
                                {
                                case aJOYEVENT_AXIS_1_X:
                                    filteredValue = math::FilterIntValue<int16>(controllerThresholds->axis1 * 32767, ev.value);
                                    newEvent.action = aJOYEVENT_AXIS_1_X;
                                    break;
                                case aJOYEVENT_AXIS_1_Y:
                                    filteredValue = math::FilterIntValue<int16>(controllerThresholds->axis1 * 32767, -ev.value);
                                    newEvent.action = aJOYEVENT_AXIS_1_Y;
                                    break;
                                case aJOYEVENT_AXIS_2_X:
                                    filteredValue = math::FilterIntValue<int16>(controllerThresholds->axis2 * 32767, ev.value);
                                    newEvent.action = aJOYEVENT_AXIS_2_X;
                                    break;
                                case aJOYEVENT_AXIS_2_Y:
                                    filteredValue = math::FilterIntValue<int16>(controllerThresholds->axis2 * 32767, -ev.value);
                                    newEvent.action = aJOYEVENT_AXIS_2_Y;
                                    break;
                                case aJOYEVENT_THROTTLE_1:
                                    filteredValue = math::FilterIntValue<int16>(controllerThresholds->throttle1 * 32767, ev.value);
                                    newEvent.action = aJOYEVENT_THROTTLE_1;
                                    break;
                                case aJOYEVENT_THROTTLE_2:
                                    filteredValue = math::FilterIntValue<int16>(controllerThresholds->throttle2 * 32767, ev.value);
                                    newEvent.action = aJOYEVENT_THROTTLE_2;
                                    break;
                                case aJOYEVENT_POVRIGHT:
                                case aJOYEVENT_POVLEFT:
                                {
                                    //use axis1 values : todo : make this better
                                    filteredValue = math::FilterIntValue<int16>(controllerThresholds->axis1 * 32767, ev.value);

                                    if (filteredValue > 0)
                                        newEvent.action = aJOYEVENT_POVRIGHT;
                                    else if (filteredValue < 0)
                                    {
                                        newEvent.action = aJOYEVENT_POVLEFT;
                                        filteredValue = abs(filteredValue);
                                    }
                                    else
                                        newEvent.action = currentJoystick->lastHorizontalPOV;

                                    currentJoystick->lastHorizontalPOV = newEvent.action;

                                    break;
                                }
                                case aJOYEVENT_POVUP:
                                case aJOYEVENT_POVDOWN:
                                {
                                    filteredValue = math::FilterIntValue<int16>(controllerThresholds->axis1 * 32767, -ev.value);

                                    if (filteredValue > 0)
                                        newEvent.action = aJOYEVENT_POVUP;
                                    else if (filteredValue < 0)
                                    {
                                        newEvent.action = aJOYEVENT_POVDOWN;
                                        filteredValue = abs(filteredValue);
                                    }
                                    else
                                        newEvent.action = currentJoystick->lastVerticalPOV;

                                    currentJoystick->lastVerticalPOV = newEvent.action;

                                    break;
                                }
                                 }
                            }
                            else
                            {
                                //use axis1 values : todo : make this better
                                filteredValue = math::FilterIntValue<int16>(controllerThresholds->axis1 * 32767, ev.value);
                                newEvent.action = aJOYEVENT_UNKNOWN;
                            }

                            if (filteredValue != currentJoystick->lastAxisValues[axisNumber])
                            {
                                //std::cerr << "axis: " << axisNumber << "mashVal: " << (int)newEvent.action << " : "<< filteredValue << std::endl;
                                newEvent.value = filteredValue * g_invShort;
                                newEvent.isPressed = (filteredValue != 0)?true:false;
                                newEvent.character = ev.number;
                                currentJoystick->lastAxisValues[axisNumber] = filteredValue;
                                GetInputManager()->ImmediateBroadcast(newEvent);
                            }
                            break;
                        }
                    }
                }
            }
        }

        return close;
    }

    void CMashLinuxDevice::SyncInputDeviceWithCurrentState()
    {
        m_lastMousePosX = 0;
        m_lastMousePosY = 0;
        XWarpPointer(m_glXdisplay, None, m_glXwin, 0, 0, 0, 0, m_lastMousePosX, m_lastMousePosY);
        GetInputManager()->_SetCursorPosition(m_lastMousePosX, m_lastMousePosY);
    }

    void CMashLinuxDevice::Sleep(uint32 ms)const
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

        CMashLinuxDevice *device = MASH_NEW_COMMON CMashLinuxDevice(settings.debugFilePath);
        if (device->Initialise(settings) == aMASH_FAILED)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                            "Failed to initialise device.",
                             "CMashLinuxDevice::CreateDevice");

            MASH_DELETE device;
            return 0;
        }

        if (device->LoadComponents(settings) == aMASH_FAILED)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                            "Failed to load components.",
                             "CMashLinuxDevice::CreateDevice");

            MASH_DELETE device;
            return 0;
        }

        return device;
    }
}
#endif
