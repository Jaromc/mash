//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLRenderer.h"
#include "CMashOpenGLSkinManager.h"
#include "CMashOpenGLTexture.h"
#include "CMashOpenGLCubeTexture.h"
#include "CMashOpenGLVertexBuffer.h"
#include "CMashOpenGLIndexBuffer.h"
#include "CMashOpenGLVertex.h"
#include "CMashOpenGLTextureState.h"
#include "CMashOpenGLMeshBuffer.h"
#include "MashHelper.h"
#include "CMashOpenGLHelper.h"
#include "CMashOpenGLEffect.h"
#include "CMashOpenGLRenderSurface.h"
#include "CMashOpenGLCubicRenderSurface.h"
#include "MashRectangle2.h"
#include "MashFileStream.h"
#include "MashMaterial.h"
#include "MashMaterialDependentResource.h"
#include "MashLog.h"
#include "SOIL.h"

#ifdef MASH_WINDOWS
#include "MashDevice.h"
#elif defined (MASH_APPLE)
#include "MashDevice.h"
#elif defined (MASH_LINUX)
#include "MashDevice.h"
#endif

namespace mash
{
    
    MashVideo* CreateMashOpenGL3Device()
    {
        CMashOpenGLRenderer *pNewRenderer = MASH_NEW_COMMON CMashOpenGLRenderer();
        return pNewRenderer;
    }

#ifdef MASH_WINDOWS
	CMashOpenGLRenderer::CMashOpenGLRenderer():MashVideoIntermediate(), m_hWnd(0), m_backbufferSize(0.0f ,0.0f),
		m_defaultRenderTarget(0)
	{
	}
#elif defined (MASH_APPLE) || defined (MASH_LINUX)
    CMashOpenGLRenderer::CMashOpenGLRenderer():MashVideoIntermediate(), m_device(0), m_backbufferSize(0.0f ,0.0f),
        m_defaultRenderTarget(0)
	{
	}
#endif

	CMashOpenGLRenderer::~CMashOpenGLRenderer()
	{
		MashArray<MashTextureState*>::Iterator samplerIter = m_samplerStates.Begin();
		MashArray<MashTextureState*>::Iterator samplerIterEnd = m_samplerStates.End();
		for(; samplerIter != samplerIterEnd; ++samplerIter)
		{
			if (*samplerIter)
				(*samplerIter)->Drop();
		}
        
		m_samplerStates.Clear();
        
        if (m_defaultRenderTarget)
        {
            m_defaultRenderTarget->Drop();
            m_defaultRenderTarget = 0;
        }
	}

    bool CMashOpenGLRenderer::IsExtensionSupported(const char *extList, const char *extension)
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

	eMASH_STATUS CMashOpenGLRenderer::_Initialise(MashDevice *device, const sMashDeviceSettings &creationParameters, void *extraData)
	{
#ifdef MASH_WINDOWS

        if (!extraData)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                              "HWND missing.", 
                              "CMashOpenGLRenderer::_Initialise");

			return aMASH_FAILED;
		}

		m_hWnd = (HWND)extraData;
#elif defined (MASH_APPLE)
        if (device->GetDeviceType() != aDEVICE_TYPE_APPLE)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                              "A device other than apple was initialised in a apple environment.", 
                              "CMashOpenGLRenderer::_Initialise");
			return aMASH_FAILED;
        }
        
       m_device = device;
#elif defined (MASH_LINUX)
        if (device->GetDeviceType() != aDEVICE_TYPE_LINUX)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                              "A device other than linux was initialised in a linux environment.",
                              "CMashOpenGLRenderer::_Initialise");
            return aMASH_FAILED;
        }

       m_device = device;
#else
        MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                          "An invalid device was created not supported by this OpenGL renderer.",
                          "CMashOpenGLRenderer::_Initialise");
        return aMASH_FAILED;
#endif
      
        MashFileManager *pFileManager = device->GetFileManager();
        
		sMashViewPort viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = creationParameters.screenWidth;
		viewport.height = creationParameters.screenHeight;
		viewport.minZ = 0.000f;
		viewport.maxZ = 1.0f;

		m_defaultRenderTargetViewport = viewport;

		m_backbufferSize.x = creationParameters.screenWidth;
		m_backbufferSize.y = creationParameters.screenHeight;
        
		/*
			TODO : Output device capabilities. Max render targets etc...
		*/

#ifdef MASH_WINDOWS
		/*
			We need to create a dummy window to test for pixel formats
		*/
		RECT dummyRect;
		dummyRect.left = 0;
		dummyRect.top = 0;
		dummyRect.right = m_backbufferSize.x;
		dummyRect.bottom = m_backbufferSize.y;
		DWORD dwstyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		const DWORD dwexstyle = WS_EX_APPWINDOW;

		if (creationParameters.fullScreen)
			dwstyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		HINSTANCE m_hInstance = GetModuleHandle(0);
		const int8* sClassName = "MashWin32Device";//hack, name from CMashWinDevice

		AdjustWindowRectEx(&dummyRect, dwstyle, FALSE, dwexstyle);
		 HWND dummyHWnd = CreateWindowEx(dwexstyle,
			   sClassName,		// Name of the window class
			   "",				// Title
			 /*WS_CLIPSIBLINGS | WS_CLIPCHILDREN |  */dwstyle/* WS_OVERLAPPEDWINDOW*//*WS_POPUP*/, // Window style
			   0, // x pos
			   0, // y pos
			   dummyRect.right - dummyRect.left/* creationParameters.screenWidth*/, // width
			   dummyRect.bottom - dummyRect.top /*creationParameters.screenHeight*/, // height
			   NULL,				// Handle of parent window
			   NULL,				// Handle to menu
			   m_hInstance,			// Instance of app
			   NULL);

		   ShowWindow(dummyHWnd, SW_HIDE);
		   UpdateWindow(dummyHWnd);

		HDC dummyHdc = GetDC(dummyHWnd); // Get the device context for our window
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion   = 1;
		pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.iLayerType = PFD_MAIN_PLANE;

		if (creationParameters.backbufferFormat == aBACKBUFFER_FORMAT_16BIT)
			pfd.cColorBits = 16;
		else
			pfd.cColorBits = 32;

		if (creationParameters.depthFormat == aDEPTH_FORMAT_32BIT)
			pfd.cDepthBits = 32;
		else
			pfd.cDepthBits = 24;
		
		int32 nPixelFormat = ChoosePixelFormat(dummyHdc, &pfd);
		if (nPixelFormat == 0)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Failed to choose pixel format.", 
                             "CMashOpenGLRenderer::_Initialise");
            
            return aMASH_FAILED;
        }

		bool bResult = SetPixelFormat(dummyHdc, nPixelFormat, &pfd);
		if (!bResult)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Failed to set pixel format.", 
                             "CMashOpenGLRenderer::_Initialise");
            
			return aMASH_FAILED;
        }

		HGLRC tempContext = wglCreateContext(dummyHdc);
		if (!tempContext)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Failed to create wgl context.", 
                             "CMashOpenGLRenderer::_Initialise");
            
			return aMASH_FAILED;
		}

		if (!wglMakeCurrent(dummyHdc, tempContext))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Failed to make temporary wgl context current.", 
                             "CMashOpenGLRenderer::_Initialise");
            
			return aMASH_FAILED;
		}

		m_hdc = GetDC(m_hWnd); // Get the device context for our main window

		PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
		wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");
		if(wglCreateContextAttribsARB == NULL)
		{
			DWORD errorMsg = GetLastError();
            MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
                             "CMashOpenGLRenderer::_Initialise", 
                             "Failed to get wglCreateContextAttribsARB with error '%d'", errorMsg);
            
			return aMASH_FAILED;
		}

		PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB =
			(PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

		uint32 multisampleLevels = 0;
		switch(creationParameters.antiAliasType)
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
			glDisable(GL_MULTISAMPLE);
		}
	 
		bool multisamplingAvaliable = false;
		if (wglChoosePixelFormatARB && (multisampleLevels > 0))
		{
			int32 attributes[] = 
			{
				WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
				WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
				WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB, pfd.cColorBits,
				WGL_DEPTH_BITS_ARB, pfd.cDepthBits,
				WGL_STENCIL_BITS_ARB, 0,
				WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
				WGL_SAMPLES_ARB, multisampleLevels ,
				0
			};

			int32 newPixelFormat;
			bool valid;
			UINT numFormats;
			f32 fAttributes[] = {0,0};
			valid = wglChoosePixelFormatARB(m_hdc,attributes,fAttributes,1,&newPixelFormat,&numFormats);
			if (valid && numFormats >= 1)
			{
				bool bResult = SetPixelFormat(m_hdc, newPixelFormat, &pfd);
				if (bResult) // If it fails
				{
					multisamplingAvaliable = true;
					glEnable(GL_MULTISAMPLE);
				}
				else
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING, 
                                     "Failed to set pixel format for multisampling.", 
                                     "CMashOpenGLRenderer::_Initialise");
				}
			}
		}

		if (!multisamplingAvaliable)
		{
			/*
				No multisampling so use the default
			*/
			bool bResult = SetPixelFormat(m_hdc, nPixelFormat, &pfd);
			if (!bResult)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to set default pixel format.", 
                                 "CMashOpenGLRenderer::_Initialise");
                
				return aMASH_FAILED;
            }
		}

		int32 attributes[] = 
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3, // Set the MAJOR version of OpenGL to 3
			WGL_CONTEXT_MINOR_VERSION_ARB, 3, // Set the MINOR version of OpenGL to 3
			WGL_CONTEXT_FLAGS_ARB, 0, // Set our OpenGL context to be forward compatible
			//WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			//WGL_SAMPLES_ARB, 4,
			0
		};

		m_hrc = wglCreateContextAttribsARB(m_hdc, NULL, attributes);

		ReleaseDC(dummyHWnd, dummyHdc);
		wglMakeCurrent(NULL, NULL); // Remove the temporary context from being active
		wglDeleteContext(tempContext); // Delete the temporary OpenGL context
		if (!wglMakeCurrent(m_hdc, m_hrc)) // Make our OpenGL context current
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Failed to make wgl context current.", 
                             "CMashOpenGLRenderer::_Initialise");
            
			return aMASH_FAILED;
		}
		//enable vsync
		int32 vsyncFlag = (creationParameters.enableVSync)?1:0;
		PFNWGLSWAPINTERVALEXTPROC enableVSync = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

        if (enableVSync)
        {
            enableVSync(vsyncFlag);
        }
        else
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING,
                     "wglSwapIntervalEXT not defined. Vsync control not avaliable.",
                      "CMashOpenGLRenderer::_Initialise");
        }

#elif defined (MASH_APPLE)
//empty
#elif defined (MASH_LINUX)
//empty
#endif

        MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION,
                            "CMashOpenGLRenderer::_Initialise",
                            "Current OpenGL version: %s", glGetString(GL_VERSION));

#if defined (MASH_WINDOWS) || defined  (MASH_LINUX)

#ifdef MASH_WINDOWS
#define OPENGL_GET_PROC wglGetProcAddress
#define OPENGL_PROC_CAST const char*
#elif defined (MASH_LINUX)
#define OPENGL_GET_PROC glXGetProcAddress
#define OPENGL_PROC_CAST const GLubyte*
#endif

#define LOAD_FUNCT_FALLBACK(mainString, fallbackString, functionPtrType, functionPtr)\
    functionPtr = (functionPtrType)OPENGL_GET_PROC((OPENGL_PROC_CAST)mainString);\
    if (!functionPtr)\
    {\
        if (fallbackString)\
        {\
            functionPtr = (functionPtrType)OPENGL_GET_PROC((OPENGL_PROC_CAST)fallbackString);\
        }\
    }\

		glGetStringiPtr = (PFNGLGETSTRINGIPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGetStringi");

		struct sGLExt
        {
            //common ext
            const GLubyte *first;
            //ext if first isn't on this system
            const GLubyte *second;
        };

        /*
         * we only check the extentions that are most likely not to be here.
         */
        const sGLExt mustHaveExt[] = {{(const GLubyte*)"GL_ARB_vertex_array_object", 0},
                                {(const GLubyte*)"GL_ARB_instanced_arrays", 0},
                                {(const GLubyte*)"GL_ARB_map_buffer_range", 0},
                                {(const GLubyte*)"GL_ARB_copy_buffer", 0},
                                {(const GLubyte*)"GL_ARB_draw_instanced", (const GLubyte*)"GL_EXT_draw_instanced"},
                                {(const GLubyte*)"GL_ARB_framebuffer_object", (const GLubyte*)"GL_EXT_framebuffer_object"},
                                {(const GLubyte*)"GL_ARB_uniform_buffer_object", (const GLubyte*)"GL_EXT_bindable_uniform"},
                                     {0, 0}};

    GLint extentionCount = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extentionCount);
    for(GLint mh = 0; mustHaveExt[mh].first != 0; ++mh)
    {
        bool found = false;
        for (GLint ext = 0; ext < extentionCount; ++ext)
        {
            const GLubyte *glExts =  glGetStringiPtr(GL_EXTENSIONS, ext);
            if (glExts && ((strcmp((const char*)glExts, (const char*)mustHaveExt[mh].first) == 0) ||
                           (mustHaveExt[mh].second && (strcmp((const char*)glExts, (const char*)mustHaveExt[mh].second) == 0))))
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, \
                            "CMashOpenGLRenderer::_Initialise",\
                             "OpenGL extention missing on the current system '%s'.", mustHaveExt[mh]);\

             return aMASH_FAILED;\
        }
    }


        //GL_EXT_draw_instanced
        LOAD_FUNCT_FALLBACK("glDrawElementsInstanced", "glDrawElementsInstancedEXT", PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstancedPtr);
        LOAD_FUNCT_FALLBACK("glDrawArraysInstanced", "glDrawArraysInstancedEXT", PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstancedPtr);

        //GL_EXT_framebuffer_object
        LOAD_FUNCT_FALLBACK("glGenFramebuffers", "glGenFramebuffersEXT", PFNGLGENRENDERBUFFERSPROC, glGenFramebuffersPtr);
        LOAD_FUNCT_FALLBACK("glDeleteFramebuffers", "glDeleteFramebuffersEXT", PFNGLDELETERENDERBUFFERSPROC, glDeleteFramebuffersPtr);
        LOAD_FUNCT_FALLBACK("glFramebufferTexture1D", "glFramebufferTexture1DEXT", PFNGLFRAMEBUFFERTEXTURE1DPROC, glFramebufferTexture1DPtr);
        LOAD_FUNCT_FALLBACK("glFramebufferTexture2D", "glFramebufferTexture2DEXT", PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2DPtr);
        LOAD_FUNCT_FALLBACK("glFramebufferTexture3D", "glFramebufferTexture3DEXT", PFNGLFRAMEBUFFERTEXTURE3DPROC, glFramebufferTexture3DPtr);
        LOAD_FUNCT_FALLBACK("glBindFramebuffer", "glBindFramebufferEXT", PFNGLBINDFRAMEBUFFERPROC, glBindFramebufferPtr);
        LOAD_FUNCT_FALLBACK("glFramebufferRenderbuffer", "glFramebufferRenderbufferEXT", PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbufferPtr);
        LOAD_FUNCT_FALLBACK("glCheckFramebufferStatus", "glCheckFramebufferStatusEXT", PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatusPtr);
        LOAD_FUNCT_FALLBACK("glGenerateMipmap", "glGenerateMipmapEXT", PFNGLGENERATEMIPMAPPROC, glGenerateMipmapPtr);
        LOAD_FUNCT_FALLBACK("glGenRenderbuffers", "glGenRenderbuffersEXT", PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffersPtr);
        LOAD_FUNCT_FALLBACK("glDeleteRenderbuffers", "glDeleteRenderbuffersEXT", PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffersPtr);
        LOAD_FUNCT_FALLBACK("glBindRenderbuffer", "glBindRenderbufferEXT", PFNGLBINDRENDERBUFFERPROC, glBindRenderbufferPtr);
        LOAD_FUNCT_FALLBACK("glRenderbufferStorage", "glRenderbufferStorageEXT", PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStoragePtr);
        LOAD_FUNCT_FALLBACK("glGetRenderbufferParameteriv", "glGetRenderbufferParameterivEXT", PFNGLGETRENDERBUFFERPARAMETERIVPROC, glGetRenderbufferParameterivPtr);

        //GL_ARB_copy_buffer
        LOAD_FUNCT_FALLBACK("glBindBuffer", "glBindBufferARB", PFNGLBINDBUFFERPROC, glBindBufferPtr);
        LOAD_FUNCT_FALLBACK("glBufferData", "glBufferDataARB", PFNGLBUFFERDATAPROC, glBufferDataPtr);
        LOAD_FUNCT_FALLBACK("glUnmapBuffer", "glUnmapBufferARB", PFNGLUNMAPBUFFERPROC, glUnmapBufferPtr);

        //GL_ARB_map_buffer_range
        LOAD_FUNCT_FALLBACK("glMapBufferRange", "glMapBufferRangeARB", PFNGLMAPBUFFERRANGEPROC, glMapBufferRangePtr);
        LOAD_FUNCT_FALLBACK("glFlushMappedBufferRange", "glFlushMappedBufferRangeARB", PFNGLFLUSHMAPPEDBUFFERRANGEPROC, glFlushMappedBufferRangePtr);

        //GL_ARB_instanced_arrays
        LOAD_FUNCT_FALLBACK("glVertexAttribDivisor", "glVertexAttribDivisorARB", PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisorPtr);

        //GL_ARB_vertex_array_object
        LOAD_FUNCT_FALLBACK("glBindVertexArray", "glBindVertexArrayARB", PFNGLBINDVERTEXARRAYPROC, glBindVertexArrayPtr);
        LOAD_FUNCT_FALLBACK("glGenVertexArrays", "glGenVertexArraysARB", PFNGLGENVERTEXARRAYSPROC, glGenVertexArraysPtr);
        LOAD_FUNCT_FALLBACK("glDeleteVertexArrays", "glDeleteVertexArraysARB", PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArraysPtr);

        glGetActiveUniformBlockNamePtr = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGetActiveUniformBlockName");
        glGetProgramInfoLogPtr = (PFNGLGETPROGRAMINFOLOGPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGetProgramInfoLog");
        glBindBufferRangePtr = (PFNGLBINDBUFFERRANGEPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glBindBufferRange");
        glBlitFramebufferPtr = (PFNGLBLITFRAMEBUFFERPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glBlitFramebuffer");
        glDrawBuffersPtr = (PFNGLDRAWBUFFERSPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glDrawBuffers");
        glGenBuffersPtr = (PFNGLGENBUFFERSPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGenBuffers");
        glBufferSubDataPtr = (PFNGLBUFFERSUBDATAPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glBufferSubData");
        glVertexAttribPointerPtr = (PFNGLVERTEXATTRIBPOINTERPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glVertexAttribPointer");
        glEnableVertexAttribArrayPtr = (PFNGLENABLEVERTEXATTRIBARRAYPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glEnableVertexAttribArray");
        glDisableVertexAttribArrayPtr = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glDisableVertexAttribArray");
        glBlendEquationSeparatePtr = (PFNGLBLENDEQUATIONSEPARATEPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glBlendEquationSeparate");
        glBlendFuncSeparatePtr = (PFNGLBLENDFUNCSEPARATEPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glBlendFuncSeparate");

        glMapBufferPtr = (PFNGLMAPBUFFERPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glMapBuffer");
        glGetBufferParameterivPtr = (PFNGLGETBUFFERPARAMETERIVPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGetBufferParameteriv");
        glDeleteBuffersPtr = (PFNGLDELETEBUFFERSPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glDeleteBuffers");

        glBindFragDataLocationPtr = (PFNGLBINDFRAGDATALOCATIONPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glBindFragDataLocation");
        glCreateProgramPtr = (PFNGLCREATEPROGRAMPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glCreateProgram");
        glGetActiveUniformPtr = (PFNGLGETACTIVEUNIFORMPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGetActiveUniform");
        glGetProgramivPtr = (PFNGLGETPROGRAMIVPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGetProgramiv");
        glGetUniformLocationPtr = (PFNGLGETUNIFORMLOCATIONPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGetUniformLocation");
        glLinkProgramPtr = (PFNGLLINKPROGRAMPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glLinkProgram");
        glUseProgramPtr = (PFNGLUSEPROGRAMPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUseProgram");
        glAttachShaderPtr = (PFNGLATTACHSHADERPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glAttachShader");
        glGetUniformBlockIndexPtr = (PFNGLGETUNIFORMBLOCKINDEXPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGetUniformBlockIndex");

        glCompileShaderPtr = (PFNGLCOMPILESHADERPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glCompileShader");
        glCreateShaderPtr = (PFNGLCREATESHADERPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glCreateShader");
        glGetShaderInfoLogPtr = (PFNGLGETSHADERINFOLOGPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGetShaderInfoLog");
        glGetShaderivPtr = (PFNGLGETSHADERIVPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGetShaderiv");
        glShaderSourcePtr = (PFNGLSHADERSOURCEPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glShaderSource");
        glDetachShaderPtr = (PFNGLDETACHSHADERPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glDetachShader");
        glDeleteShaderPtr = (PFNGLDELETESHADERPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glDeleteShader");
        glDeleteProgramPtr = (PFNGLDELETEPROGRAMPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glDeleteProgram");

        glBindAttribLocationPtr = (PFNGLBINDATTRIBLOCATIONPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glBindAttribLocation");
        glGetActiveAttribPtr = (PFNGLGETACTIVEATTRIBPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGetActiveAttrib");
        glGetAttribLocationPtr = (PFNGLGETATTRIBLOCATIONPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGetAttribLocation");
        glActiveTexturePtr = (PFNGLACTIVETEXTUREPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glActiveTexture");

        glUniformBlockBindingPtr = (PFNGLUNIFORMBLOCKBINDINGPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniformBlockBinding");
        glBindBufferBasePtr = (PFNGLBINDBUFFERBASEPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glBindBufferBase");
        glGetActiveUniformBlockivPtr = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glGetActiveUniformBlockiv");
        glUniformMatrix4fvPtr = (PFNGLUNIFORMMATRIX4FVPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniformMatrix4fv");
        glUniform1iPtr = (PFNGLUNIFORM1IPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform1i");
        glUniform1ivPtr = (PFNGLUNIFORM1IVPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform1iv");
        glUniform1fPtr = (PFNGLUNIFORM1FPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform1f");
        glUniform1fvPtr = (PFNGLUNIFORM1FVPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform1fv");
        glUniform2ivPtr = (PFNGLUNIFORM2IVPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform2iv");
        glUniform2iPtr = (PFNGLUNIFORM2IPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform2i");
        glUniform2fvPtr = (PFNGLUNIFORM2FVPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform2fv");
        glUniform2fPtr = (PFNGLUNIFORM2FPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform2f");
        glUniform3ivPtr = (PFNGLUNIFORM3IVPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform3iv");
        glUniform3iPtr = (PFNGLUNIFORM3IPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform3i");
        glUniform3fvPtr = (PFNGLUNIFORM3FVPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform3fv");
        glUniform3fPtr = (PFNGLUNIFORM3FPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform3f");
        glUniform4ivPtr = (PFNGLUNIFORM4IVPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform4iv");
        glUniform4iPtr = (PFNGLUNIFORM4IPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform4i");
        glUniform4fvPtr = (PFNGLUNIFORM4FVPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform4fv");
        glUniform4fPtr = (PFNGLUNIFORM4FPROC)OPENGL_GET_PROC((OPENGL_PROC_CAST)"glUniform4f");

#elif defined (MASH_APPLE)
        
        glGetActiveUniformBlockNamePtr = glGetActiveUniformBlockName;
        glGetProgramInfoLogPtr = glGetProgramInfoLog;
        glFlushMappedBufferRangePtr = glFlushMappedBufferRange;
        glMapBufferRangePtr = glMapBufferRange;
        glBindBufferRangePtr = glBindBufferRange;
        glGenFramebuffersPtr = glGenFramebuffers;
        glDeleteFramebuffersPtr = glDeleteFramebuffers;
        glFramebufferTexture1DPtr = glFramebufferTexture1D;
        glFramebufferTexture2DPtr = glFramebufferTexture2D;
        glFramebufferTexture3DPtr = glFramebufferTexture3D;
        glBindFramebufferPtr = glBindFramebuffer;
        glFramebufferRenderbufferPtr = glFramebufferRenderbuffer;
        glCheckFramebufferStatusPtr = glCheckFramebufferStatus;
        glDrawElementsInstancedPtr = glDrawElementsInstanced;
        glDrawArraysInstancedPtr = glDrawArraysInstanced;
        glVertexAttribDivisorPtr = glVertexAttribDivisorARB;//ARB

        glBlitFramebufferPtr = glBlitFramebuffer;
        glDrawBuffersPtr = glDrawBuffers;
        glGenRenderbuffersPtr = glGenRenderbuffers;
        glDeleteRenderbuffersPtr = glDeleteRenderbuffers;
        glBindRenderbufferPtr = glBindRenderbuffer;
        glRenderbufferStoragePtr = glRenderbufferStorage;
        glGetRenderbufferParameterivPtr = glGetRenderbufferParameteriv;

        glGenBuffersPtr = glGenBuffers;
        glBindBufferPtr = glBindBuffer;
        glBufferDataPtr = glBufferData;
        glBufferSubDataPtr = glBufferSubData;
        glVertexAttribPointerPtr = glVertexAttribPointer;

        glEnableVertexAttribArrayPtr = glEnableVertexAttribArray;
        glDisableVertexAttribArrayPtr = glDisableVertexAttribArray;
        glBlendEquationSeparatePtr = glBlendEquationSeparate;
        glBlendFuncSeparatePtr = glBlendFuncSeparate;
        glGenerateMipmapPtr = glGenerateMipmap;
        glUnmapBufferPtr = glUnmapBuffer;
        glMapBufferPtr = glMapBuffer;
        glGetBufferParameterivPtr = glGetBufferParameteriv;
        glDeleteBuffersPtr = glDeleteBuffers;

        glBindFragDataLocationPtr = glBindFragDataLocation;
        glCreateProgramPtr = glCreateProgram;
        glGetActiveUniformPtr = glGetActiveUniform;
        glGetProgramivPtr = glGetProgramiv;
        glGetUniformLocationPtr = glGetUniformLocation;
        glLinkProgramPtr = glLinkProgram;
        glUseProgramPtr = glUseProgram;
        glAttachShaderPtr = glAttachShader;
        glGetUniformBlockIndexPtr = glGetUniformBlockIndex;

        glCompileShaderPtr = glCompileShader;
        glCreateShaderPtr = glCreateShader;
        glGetShaderInfoLogPtr = glGetShaderInfoLog;
        glGetShaderivPtr = glGetShaderiv;
        glShaderSourcePtr = glShaderSource;
        glDetachShaderPtr = glDetachShader;
        glDeleteShaderPtr = glDeleteShader;
        glDeleteProgramPtr = glDeleteProgram;

        glBindVertexArrayPtr = glBindVertexArray;
        glGenVertexArraysPtr = glGenVertexArrays;
        glDeleteVertexArraysPtr = glDeleteVertexArrays;
        glBindAttribLocationPtr = glBindAttribLocation;
        glGetActiveAttribPtr = glGetActiveAttrib;
        glGetAttribLocationPtr = glGetAttribLocation;
        glActiveTexturePtr = glActiveTexture;

        glUniformBlockBindingPtr = glUniformBlockBinding;
        glBindBufferBasePtr = glBindBufferBase;
        glGetActiveUniformBlockivPtr = glGetActiveUniformBlockiv;
        glUniformMatrix4fvPtr = glUniformMatrix4fv;
        glUniform1iPtr = glUniform1i;
        glUniform1ivPtr = glUniform1iv;
        glUniform1fPtr = glUniform1f;
        glUniform1fvPtr = glUniform1fv;
        glUniform2ivPtr = glUniform2iv;
        glUniform2iPtr = glUniform2i;
        glUniform2fvPtr = glUniform2fv;
        glUniform2fPtr = glUniform2f;
        glUniform3ivPtr = glUniform3iv;
        glUniform3iPtr = glUniform3i;
        glUniform3fvPtr = glUniform3fv;
        glUniform3fPtr = glUniform3f;
        glUniform4ivPtr = glUniform4iv;
        glUniform4iPtr = glUniform4i;
        glUniform4fvPtr = glUniform4fv;
        glUniform4fPtr = glUniform4f;
#endif

#define CHECK_OPENGL_FUNCTION(function, functionString)\
    if(!function)\
    {\
    MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,\
                    "CMashOpenGLRenderer::_Initialise",\
                     "Missing OpenGL function '%s'.", functionString);\
        return aMASH_FAILED;\
    }\

        //
        CHECK_OPENGL_FUNCTION(glVertexAttribDivisorPtr, "glVertexAttribDivisor")
        CHECK_OPENGL_FUNCTION(glGetActiveUniformBlockNamePtr, "glGetActiveUniformBlockName");
        CHECK_OPENGL_FUNCTION(glGetProgramInfoLogPtr, "glGetProgramInfoLog");
        CHECK_OPENGL_FUNCTION(glFlushMappedBufferRangePtr, "glFlushMappedBufferRange");
        CHECK_OPENGL_FUNCTION(glMapBufferRangePtr, "glMapBufferRange");
        CHECK_OPENGL_FUNCTION(glBindBufferRangePtr, "glBindBufferRange");
        CHECK_OPENGL_FUNCTION(glGenFramebuffersPtr, "glGenFramebuffers");
        CHECK_OPENGL_FUNCTION(glDeleteFramebuffersPtr, "glDeleteFramebuffers");
        CHECK_OPENGL_FUNCTION(glFramebufferTexture1DPtr, "glFramebufferTexture1D");
        CHECK_OPENGL_FUNCTION(glFramebufferTexture2DPtr, "glFramebufferTexture2D");
        CHECK_OPENGL_FUNCTION(glFramebufferTexture3DPtr, "glFramebufferTexture3D");
        CHECK_OPENGL_FUNCTION(glBindFramebufferPtr, "glBindFramebuffer");
        CHECK_OPENGL_FUNCTION(glFramebufferRenderbufferPtr, "glFramebufferRenderbuffer");
        CHECK_OPENGL_FUNCTION(glCheckFramebufferStatusPtr, "glCheckFramebufferStatus");
        CHECK_OPENGL_FUNCTION(glDrawElementsInstancedPtr, "glDrawElementsInstanced");
        CHECK_OPENGL_FUNCTION(glDrawArraysInstancedPtr, "glDrawArraysInstanced");
        CHECK_OPENGL_FUNCTION(glBlitFramebufferPtr, "glBlitFramebuffer");
        CHECK_OPENGL_FUNCTION(glDrawBuffersPtr, "glDrawBuffers");
        CHECK_OPENGL_FUNCTION(glGenRenderbuffersPtr, "glGenRenderbuffers");
        CHECK_OPENGL_FUNCTION(glDeleteRenderbuffersPtr, "glDeleteRenderbuffers");
        CHECK_OPENGL_FUNCTION(glBindRenderbufferPtr, "glBindRenderbuffer");
        CHECK_OPENGL_FUNCTION(glRenderbufferStoragePtr, "glRenderbufferStorage");
        CHECK_OPENGL_FUNCTION(glGetRenderbufferParameterivPtr, "glGetRenderbufferParameteriv");
        CHECK_OPENGL_FUNCTION(glGenBuffersPtr, "glGenBuffers");
        CHECK_OPENGL_FUNCTION(glBindBufferPtr, "glBindBuffer");
        CHECK_OPENGL_FUNCTION(glBufferDataPtr, "glBufferData");
        CHECK_OPENGL_FUNCTION(glBufferSubDataPtr, "glBufferSubData");
        CHECK_OPENGL_FUNCTION(glEnableVertexAttribArrayPtr, "glEnableVertexAttribArray");
        CHECK_OPENGL_FUNCTION(glDisableVertexAttribArrayPtr, "glDisableVertexAttribArray");
        CHECK_OPENGL_FUNCTION(glBlendEquationSeparatePtr, "glBlendEquationSeparate");
        CHECK_OPENGL_FUNCTION(glBlendFuncSeparatePtr, "glBlendFuncSeparate");
        CHECK_OPENGL_FUNCTION(glGenerateMipmapPtr, "glGenerateMipmap");
        CHECK_OPENGL_FUNCTION(glUnmapBufferPtr, "glUnmapBuffer");
        CHECK_OPENGL_FUNCTION(glMapBufferPtr, "glMapBuffer");
        CHECK_OPENGL_FUNCTION(glGetBufferParameterivPtr, "glGetBufferParameteriv");
        CHECK_OPENGL_FUNCTION(glDeleteBuffersPtr, "glDeleteBuffers");
        CHECK_OPENGL_FUNCTION(glBindFragDataLocationPtr, "glBindFragDataLocation");
        CHECK_OPENGL_FUNCTION(glCreateProgramPtr, "glCreateProgram");

        CHECK_OPENGL_FUNCTION(glGetActiveUniformPtr, "glGetActiveUniform");
        CHECK_OPENGL_FUNCTION(glGetProgramivPtr, "glGetProgramiv");
        CHECK_OPENGL_FUNCTION(glGetUniformLocationPtr, "glGetUniformLocation");
        CHECK_OPENGL_FUNCTION(glLinkProgramPtr, "glLinkProgram");
        CHECK_OPENGL_FUNCTION(glUseProgramPtr, "glUseProgram");
        CHECK_OPENGL_FUNCTION(glAttachShaderPtr, "glAttachShader");
        CHECK_OPENGL_FUNCTION(glGetUniformBlockIndexPtr, "glGetUniformBlockIndex");
        CHECK_OPENGL_FUNCTION(glCompileShaderPtr, "glCompileShader");
        CHECK_OPENGL_FUNCTION(glCreateShaderPtr, "glCreateShader");
        CHECK_OPENGL_FUNCTION(glGetShaderInfoLogPtr, "glGetShaderInfoLog");
        CHECK_OPENGL_FUNCTION(glGetShaderivPtr, "glGetShaderiv");
        CHECK_OPENGL_FUNCTION(glShaderSourcePtr, "glShaderSource");
        CHECK_OPENGL_FUNCTION(glDetachShaderPtr, "glDetachShader");
        CHECK_OPENGL_FUNCTION(glDeleteShaderPtr, "glDeleteShader");
        CHECK_OPENGL_FUNCTION(glDeleteProgramPtr, "glDeleteProgram");
        CHECK_OPENGL_FUNCTION(glBindVertexArrayPtr, "glBindVertexArray");
        CHECK_OPENGL_FUNCTION(glGenVertexArraysPtr, "glGenVertexArrays");
        CHECK_OPENGL_FUNCTION(glDeleteVertexArraysPtr, "glDeleteVertexArrays");
        CHECK_OPENGL_FUNCTION(glBindAttribLocationPtr, "glBindAttribLocation");
        CHECK_OPENGL_FUNCTION(glGetActiveAttribPtr, "glGetActiveAttrib");
        CHECK_OPENGL_FUNCTION(glGetAttribLocationPtr, "glGetAttribLocation");
        CHECK_OPENGL_FUNCTION(glActiveTexturePtr, "glActiveTexture");
        CHECK_OPENGL_FUNCTION(glUniformBlockBindingPtr, "glUniformBlockBinding");
        CHECK_OPENGL_FUNCTION(glBindBufferBasePtr, "glBindBufferBase");
        CHECK_OPENGL_FUNCTION(glGetActiveUniformBlockivPtr, "glGetActiveUniformBlockiv");
        CHECK_OPENGL_FUNCTION(glUniformMatrix4fvPtr, "glUniformMatrix4fv");
        CHECK_OPENGL_FUNCTION(glUniform1iPtr, "glUniform1i");
        CHECK_OPENGL_FUNCTION(glUniform1ivPtr, "glUniform1iv");

        CHECK_OPENGL_FUNCTION(glUniform1fPtr, "glUniform1f");
        CHECK_OPENGL_FUNCTION(glUniform1fvPtr, "glUniform1fv");
        CHECK_OPENGL_FUNCTION(glUniform2ivPtr, "glUniform2iv");
        CHECK_OPENGL_FUNCTION(glUniform2iPtr, "glUniform2i");
        CHECK_OPENGL_FUNCTION(glUniform2fvPtr, "glUniform2fv");
        CHECK_OPENGL_FUNCTION(glUniform2fPtr, "glUniform2f");
        CHECK_OPENGL_FUNCTION(glUniform3ivPtr, "glUniform3iv");
        CHECK_OPENGL_FUNCTION(glUniform3iPtr, "glUniform3i");
        CHECK_OPENGL_FUNCTION(glUniform3fvPtr, "glUniform3fv");
        CHECK_OPENGL_FUNCTION(glUniform3fPtr, "glUniform3f");
        CHECK_OPENGL_FUNCTION(glUniform4ivPtr, "glUniform4iv");
        CHECK_OPENGL_FUNCTION(glUniform4iPtr, "glUniform4i");
        CHECK_OPENGL_FUNCTION(glUniform4fvPtr, "glUniform4fv");
        CHECK_OPENGL_FUNCTION(glUniform4fPtr, "glUniform4f");
        CHECK_OPENGL_FUNCTION(glUniform1ivPtr, "glUniform1iv");
        CHECK_OPENGL_FUNCTION(glUniform1ivPtr, "glUniform1iv");
        CHECK_OPENGL_FUNCTION(glUniform1ivPtr, "glUniform1iv");

        int32 glVersion[2] = {-1, -1};
        glGetIntegerv(GL_MAJOR_VERSION, &glVersion[0]);
        glGetIntegerv(GL_MINOR_VERSION, &glVersion[1]);

        if (glVersion[0] == 2 && glVersion[1] == 1)
            m_oglVersion = aOGLVERSION_2_1;
        else if (glVersion[0] == 3 && glVersion[1] == 0)
            m_oglVersion = aOGLVERSION_3_0;
        else if (glVersion[0] == 3 && glVersion[1] == 1)
            m_oglVersion = aOGLVERSION_3_1;
        else if (glVersion[0] == 3 && glVersion[1] == 2)
            m_oglVersion = aOGLVERSION_3_2;
        else if (glVersion[0] == 3 && glVersion[1] == 3)
            m_oglVersion = aOGLVERSION_3_3;
        else
        {
            //shoudn't happen
            m_oglVersion = aOGLVERSION_3_3;

            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING,
                "OpenGL context does not match engine supported versions. Defaulting to gl3.3.",
                "CMashOpenGLRenderer::_Initialise");
        }

		// clockwise order is front facing
		glFrontFace(GL_CW);

		//glEnable(GL_MULTISAMPLE);
		//glEnable(GL_LINE_SMOOTH);
		//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		//glEnable(GL_POLYGON_SMOOTH);

		SetViewport(viewport);
        
        m_skinManager = MASH_NEW_COMMON CMashOpenGLSkinManager(this);
		if (m_skinManager->_Initialise(creationParameters) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Skin manager failed to initialise.", "CMashOpenGLRenderer::CreateRenderer");
			return aMASH_FAILED;
		}
        
    

		eMASH_STATUS status = MashVideoIntermediate::_InitialiseCommon(pFileManager);
        return status;
	}

	uint32 CMashOpenGLRenderer::GetOGLDefaultDepthBuffer()
	{
		//generate buffer if it has net yet been created
		//if (m_defaultDepthBuffer == mash::math::MaxUInt32())
        if (!m_defaultRenderTarget)
		{//m_backbufferFormat
            mash::eFORMAT eRTFormats[1] = {aFORMAT_RGBA16_FLOAT};
            m_defaultRenderTarget = CreateRenderSurface(-1, -1, eRTFormats, 1, false, aDEPTH_OPTION_OWN_DEPTH);
            
            if (!m_defaultRenderTarget)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to create default render target.", 
                                 "CMashOpenGLRenderer::GetOGLDefaultDepthBuffer");
                return 0;
            }
            
            SetRenderTargetDefault();
		}

        if (m_defaultRenderTarget)
            return ((CMashOpenGLRenderSurface*)m_defaultRenderTarget)->GetOGLDepthRenderBuffer();
        
        return 0;
	}

	eMASH_STATUS CMashOpenGLRenderer::SetRasteriserState(int32 state)
	{
		if (!m_lockRasterizerState)
		{
			if (state < 0 || state >= m_rasterizerStates.Size())
				state = m_defaultRasterizerState;

			if (m_currentRasterizerState != state)
			{
				sRasterizerStateData *stateData = &m_rasterizerStates[state];

				//special case for cull modes
				if (m_rasterizerStates[state].stateData.cullMode != aCULL_NONE)
					 glEnable(GL_CULL_FACE);
				else
					glDisable(GL_CULL_FACE);

				f32 ep = 0.00001f;
				if (math::FloatEqualTo(stateData->depthBias.paramA, 0.0f, ep) ||
					math::FloatEqualTo(stateData->depthBias.paramB, 0.0f, ep))
				{
					glEnable(GL_POLYGON_OFFSET_FILL);
					glEnable(GL_POLYGON_OFFSET_LINE);
				}
				else
				{
					glDisable(GL_POLYGON_OFFSET_FILL);
					glDisable(GL_POLYGON_OFFSET_LINE);
				}

				stateData->fillMode.func(stateData->fillMode.paramA, stateData->fillMode.paramB);
				stateData->cullMode.func(stateData->cullMode.param);
				stateData->depthTestingEnabled.func(stateData->depthTestingEnabled.param);
				stateData->depthWritingEnabled.func(stateData->depthWritingEnabled.param);
				stateData->depthComparison.func(stateData->depthComparison.param);
				stateData->depthBias.func(stateData->depthBias.paramA, stateData->depthBias.paramB);
				
				m_currentRasterizerState = state;
			}
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLRenderer::SetBlendState(int32 state)
	{
		if (!m_lockBlendState)
		{
			if (state < 0 && state >= m_blendStates.Size())
				state = m_defaultBlendState;

			if (m_currentBlendState != state)
			{
				sBlendStateData *stateData = &m_blendStates[state];
				stateData->blendEnable.func(stateData->blendEnable.param);
                glBlendEquationSeparatePtr(stateData->blendOp, stateData->blendOpAlpha);
                glBlendFuncSeparatePtr(stateData->srcBlend,
					stateData->destBlend,
					stateData->srcBlendAlpha,
					stateData->destBlendAlpha);

				glColorMask(stateData->mr, stateData->mg, stateData->mb, stateData->ma);
			}

			m_currentBlendState = state;
		}

		return aMASH_OK;
	}

	int32 CMashOpenGLRenderer::AddRasteriserState(const sRasteriserStates &state)
	{
		//make sure the same state has not been created
		const uint32 iStateCount = m_rasterizerStates.Size();
		for(uint32 i = 0; i < iStateCount; ++i)
		{
			if (m_rasterizerStates[i].stateData == state)
				return i;
		}

		sRasterizerStateData newState;
		newState.stateData = state;

		newState.fillMode.func = &glPolygonMode;
		newState.fillMode.paramA = GL_FRONT_AND_BACK;
		newState.fillMode.paramB = MashToOpenGLFillMode(state.fillMode);

		newState.depthTestingEnabled.param = GL_DEPTH_TEST;
		newState.depthTestingEnabled.func = (state.depthTestingEnable)?&glEnable:&glDisable;

		newState.depthWritingEnabled.func = &glDepthMask;
		newState.depthWritingEnabled.param = (state.depthWritingEnabled)?GL_TRUE:GL_FALSE;

		newState.depthComparison.func = &glDepthFunc;
		newState.depthComparison.param = MashToOpenGLDepthFunc(state.depthComparison);

		newState.depthBias.func = &glPolygonOffset;
		newState.depthBias.paramA = state.slopeScaledDepthBias;
		newState.depthBias.paramB = state.depthBias;

		if (state.cullMode == aCULL_NONE)
		{
			newState.cullMode.func = &glDisable;
			newState.cullMode.param = GL_CULL_FACE;
		}
		else
		{
			newState.cullMode.func = &glCullFace;
			newState.cullMode.param = MashToOpenGLCullMode(state.cullMode);
		}

		m_rasterizerStates.PushBack(newState);

		return (m_rasterizerStates.Size() - 1);
	}

	int32 CMashOpenGLRenderer::AddBlendState(const sBlendStates &state)
	{
		//make sure the same state has not been created
		const uint32 iStateCount = m_blendStates.Size();
		for(uint32 i = 0; i < iStateCount; ++i)
		{
			if (m_blendStates[i].stateData == state)
				return i;
		}

		sBlendStateData newState;
		newState.blendEnable.func = (state.blendingEnabled)?&glEnable:&glDisable;
		newState.blendEnable.param = GL_BLEND;

		newState.stateData = state;
		newState.srcBlend = MashToOpenGLBlendState(state.srcBlend);
		newState.destBlend = MashToOpenGLBlendState(state.destBlend);
		newState.srcBlendAlpha = MashToOpenGLBlendState(state.srcBlendAlpha);
		newState.destBlendAlpha = MashToOpenGLBlendState(state.destBlendAlpha);
		newState.blendOp = MashToOpenGLBlendOp(state.blendOp);
		newState.blendOpAlpha = MashToOpenGLBlendOp(state.blendOpAlpha);
		MashToOpenGLColourWriteMask(state.colourWriteMask, 
			newState.mr,
			newState.mg,
			newState.mb,
			newState.ma);

		m_blendStates.PushBack(newState);

		return (m_blendStates.Size() - 1);
	}

	MashTextureState* CMashOpenGLRenderer::AddSamplerState(const sSamplerState &state)
	{
		//make sure the same state has not been created
		const uint32 iStateCount = m_samplerStates.Size();
		for(uint32 i = 0; i < iStateCount; ++i)
		{
			if (*(m_samplerStates[i]->GetSamplerState()) == state)
				return m_samplerStates[i];
		}

		sMashOpenGLTextureStates openGLStateData;

		MashToOpenGLSamplerFilter(state.filter, openGLStateData.minFilter, openGLStateData.magFilter);
		openGLStateData.texWrapS = MashToOpenGLSamplerAddress(state.uMode);
		openGLStateData.texWrapT = MashToOpenGLSamplerAddress(state.vMode);
		openGLStateData.maxAnistropy = (float)math::Clamp<int32>(1, 16, state.maxAnistropy);

		CMashOpenGLTextureState *newState = MASH_NEW_COMMON CMashOpenGLTextureState(state, openGLStateData);

		m_samplerStates.PushBack(newState);

		return newState;
	}

	const sBlendStates* CMashOpenGLRenderer::GetBlendState(int32 iIndex)const
	{
		if (iIndex < 0 || iIndex >= m_blendStates.Size())
			iIndex = m_defaultBlendState;

		return &m_blendStates[iIndex].stateData;
	}

	const sRasteriserStates* CMashOpenGLRenderer::GetRasterizerState(int32 iIndex)const
	{
		if (iIndex < 0 || iIndex >= m_rasterizerStates.Size())
			iIndex = m_defaultRasterizerState;

		return &m_rasterizerStates[iIndex].stateData;
	}

	eMASH_STATUS CMashOpenGLRenderer::SetVertexFormat(MashVertex *pVertex)
	{
		if (!pVertex)
		{
			m_renderInfo->SetVertex(0);
		}
		else if (m_renderInfo->GetVertex() != pVertex)
		{
			const MashArray<sMashOpenGLVertexData> &elements = ((CMashOpenGLVertex*)pVertex)->GetOpenGLVertexElements();
			const uint32 elementCount = elements.Size();
			for(uint32 i = 0; i < elementCount; ++i)
			{
                glEnableVertexAttribArrayPtr(elements[i].attribute);
                glVertexAttribPointerPtr(elements[i].attribute,
					elements[i].size,
					elements[i].type,
					elements[i].normalized,
					elements[i].stride,
					elements[i].pointer);
			}
		}

		return aMASH_OK;
	}

	const mash::MashVector2 CMashOpenGLRenderer::GetBackBufferSize(bool returnActiveRenderSurfaceSize)const
	{
		if (!returnActiveRenderSurfaceSize || !m_currentRenderSurface)
			return m_backbufferSize;
		else
		{
			return m_currentRenderSurface->GetDimentions();
		}
	}

	void CMashOpenGLRenderer::SetFillColour(const sMashColour4 &fillColour)
	{
		MashVideoIntermediate::SetFillColour(fillColour);
		glClearColor(m_FillColour.r, m_FillColour.g, m_FillColour.b, m_FillColour.a);
	}

	eMASH_STATUS CMashOpenGLRenderer::BeginRender()
	{
		MashVideoIntermediate::BeginRender();

		ClearTarget(aCLEAR_TARGET | aCLEAR_DEPTH | aCLEAR_STENCIL, m_FillColour, 1.0f);
		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLRenderer::EndRender()
	{
        //flushes geometry buffers
        MashVideoIntermediate::EndRender();
        
        /*
            If the default target is created then all default rendering has
            been sent to that buffer. So we need to flush its contents
            to the back buffer.
        */
        if (m_defaultRenderTarget)
        {
            MashRectangle2 rect(m_defaultRenderTargetViewport.x, m_defaultRenderTargetViewport.y,
                               m_defaultRenderTargetViewport.x + m_defaultRenderTargetViewport.width,
                               m_defaultRenderTargetViewport.y + m_defaultRenderTargetViewport.height);
            
            //draw fbo to back buffer
            glBindFramebufferPtr(GL_FRAMEBUFFER, 0);
            DrawTexture(m_defaultRenderTarget->GetTexture(0), rect);
        }

#ifdef MASH_WINDOWS
		if (!SwapBuffers(m_hdc))
			return aMASH_FAILED;
#elif defined (MASH_APPLE) || defined (MASH_LINUX)
        m_device->_Draw();
#endif
        
        /*
            if the default fbo is created then the current fbo was unmounted above. So we need
            to set it back. Note, the current target will be the default target if the default
            target is set.
        */
        if (m_defaultRenderTarget && m_currentRenderSurface)
            glBindFramebufferPtr(GL_FRAMEBUFFER, ((CMashOpenGLRenderSurface*)m_currentRenderSurface)->GetOGLFrameBuffer());
       // int32 u = glGetError();

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLRenderer::OnResolutionChange(uint32 width, uint32 height)
	{
		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
					"CMashOpenGLRenderer::OnResolutionChange",
                    "Resolution changing to width '%u height '%u'.", width, height);

		m_backbufferSize.x = width;
		m_backbufferSize.y = height;

		//clean up any other references
		if (OnPreResolutionChangeIntermediate() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to change resolution.", 
					"CMashOpenGLRenderer::OnResolutionChange");

			return aMASH_FAILED;
		}

		sMashViewPort viewport;
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = m_backbufferSize.x;
		viewport.height = m_backbufferSize.y;
		viewport.minZ = 0.0f;
		viewport.maxZ = 1.0f;

		m_defaultRenderTargetViewport = viewport;

		SetViewport(viewport);

		if (OnPostResolutionChangeIntermediate(m_backbufferSize.x, m_backbufferSize.y) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to change resolution.", 
					"CMashOpenGLRenderer::OnResolutionChange");

			return aMASH_FAILED;
		}

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
					"Resolution change succeeded.", 
					"CMashOpenGLRenderer::OnResolutionChange");

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLRenderer::SetScreenResolution(bool fullscreen, uint32 width, uint32 height)
	{

#ifdef MASH_WINDOWS

		if (fullscreen)
		{
			SetWindowPos(m_hWnd, /*HWND_TOP*/HWND_TOPMOST, 0, 0,
				width, 
				height,
				SWP_DRAWFRAME|SWP_FRAMECHANGED);

			SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_POPUP);

			ShowWindow(m_hWnd,SW_MAXIMIZE);
		}
		else
		{
			RECT rect = {0 , 0, width, height};
			const DWORD dwstyle = WS_OVERLAPPEDWINDOW;
			const DWORD dwexstyle = WS_EX_APPWINDOW;
			AdjustWindowRectEx(&rect, dwstyle, false, dwexstyle);

			SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 
				rect.right - rect.left, 
				rect.bottom - rect.top,
				SWP_NOZORDER | SWP_SHOWWINDOW | SWP_FRAMECHANGED);

			//change the window style
			SetWindowLongPtr(m_hWnd, GWL_STYLE, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwstyle);

			ShowWindow(m_hWnd,SW_SHOW);
		}

				DEVMODE dmode;
        bool foundMode = false;
        memset(&dmode, 0, sizeof(DEVMODE));
        dmode.dmSize = sizeof(DEVMODE);
        for(int32 i=0 ; EnumDisplaySettings(0, i, &dmode) && !foundMode ; ++i)
        {
            foundMode = (dmode.dmPelsWidth==(DWORD)width) &&
                        (dmode.dmPelsHeight==(DWORD)height) &&
                        (dmode.dmBitsPerPel==(DWORD)32);
        }
        if(!foundMode) 
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to find valid screen mode for screen resolution change.", 
					"CMashOpenGLRenderer::SetScreenResolution");

			return aMASH_FAILED;
		}

        dmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		uint32 fullScreenFlag = 0;
		if (fullscreen)
		{
			fullScreenFlag = CDS_FULLSCREEN;
		}

		if (ChangeDisplaySettings(0, 0) != DISP_CHANGE_SUCCESSFUL)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to change display settings.", 
					"CMashOpenGLRenderer::SetScreenResolution");

			return aMASH_FAILED;
		}
#else
		m_backbufferSize.x = width;
		m_backbufferSize.y = height;
        //Mac shouldnt have to do anything because the backbuffer resizes to the window automatically
#endif

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLRenderer::SetViewport(const sMashViewPort &viewport)
	{
		if ((viewport.height == m_viewPort.height) &&
			(viewport.width == m_viewPort.width) &&
			(viewport.maxZ == m_viewPort.maxZ) &&
			(viewport.minZ == m_viewPort.minZ) &&
			(viewport.x == m_viewPort.x) &&
			(viewport.y == m_viewPort.y))
		{
			return aMASH_OK;
		}

		f32 currentRenderTargetHeight = m_backbufferSize.y;
		if (m_currentRenderSurface)
			currentRenderTargetHeight = m_currentRenderSurface->GetDimentions().y;

		m_viewPort = viewport;
		glViewport(m_viewPort.x, currentRenderTargetHeight - m_viewPort.y - m_viewPort.height, m_viewPort.width, m_viewPort.height);
		glDepthRange(m_viewPort.minZ, m_viewPort.maxZ);

		_OnViewportChange();

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLRenderer::SaveScreenShotToFile(eSAVE_TEXTURE_FORMAT outputFormat, const MashStringc &file)const
	{
		int32 imageType = SOIL_SAVE_TYPE_BMP;
		switch(outputFormat)
		{
		case aSAVE_TEX_FORMAT_BMP:
			imageType = SOIL_SAVE_TYPE_BMP;
			break;
		case aSAVE_TEX_FORMAT_DDS:
			imageType = SOIL_SAVE_TYPE_DDS;
			break;
		}

		if (SOIL_save_screenshot(file.GetCString(), imageType, 0, 0, (int32)m_backbufferSize.x, (int32)m_backbufferSize.y) == 0)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to save texture to file.", 
					"CMashD3D10Renderer::SaveScreenShotToFile");

			return aMASH_FAILED;
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLRenderer::SaveTextureToFile(const MashTexture *texture, 
		eSAVE_TEXTURE_FORMAT outputFormat, const MashStringc &file)const
	{
		if (!texture)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"No texture pointer given.", 
					"CMashOpenGLRenderer::SaveTextureToFile");

			return aMASH_FAILED;
		}

		uint32 oglIndex = -1;

		if (texture->GetType() == aRESOURCE_TEXTURE)
			oglIndex = ((CMashOpenGLTexture*)texture)->GetOpenGLIndex();
		else
			oglIndex = ((CMashOpenGLCubeTexture*)texture)->GetOpenGLIndex();

		if (oglIndex == -1)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"The given texture does not contain a valid OGL index.",
					"CMashOpenGLRenderer::SaveTextureToFile");

			return aMASH_FAILED;
		}

		uint32 width, height;
		texture->GetSize(width, height);

		glBindTexture(GL_TEXTURE_2D, oglIndex);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		uint8 *rawData = MASH_ALLOC_T_COMMON(uint8, width * height * 3);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, rawData);
		glBindTexture(GL_TEXTURE_2D, 0);

		uint8 *flippedRawData = MASH_ALLOC_T_COMMON(uint8, width * height * 3);
		for(uint32 y = 0; y < height; ++y)
		{
			for(uint32 x = 0; x < width; ++x)
			{
				flippedRawData[((y * width) + x) * 3] = rawData[(((height - y - 1) * width) + x) * 3];
				flippedRawData[((y * width) + x) * 3 + 1] = rawData[(((height - y - 1) * width) + x) * 3 + 1];
				flippedRawData[((y * width) + x) * 3 + 2] = rawData[(((height - y - 1) * width) + x) * 3 + 2];
			}
		}

		int32 imageType = SOIL_SAVE_TYPE_BMP;
		switch(outputFormat)
		{
		case aSAVE_TEX_FORMAT_BMP:
			imageType = SOIL_SAVE_TYPE_BMP;
			break;
		case aSAVE_TEX_FORMAT_DDS:
			imageType = SOIL_SAVE_TYPE_DDS;
			break;
		}

		if (SOIL_save_image(file.GetCString(), imageType, width, height, 3, flippedRawData) == 0)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to save texture to file.", 
					"CMashOpenGLRenderer::SaveTextureToFile");

			return aMASH_FAILED;
		}

		MASH_FREE(rawData);
		MASH_FREE(flippedRawData);

		return aMASH_OK;
	}

	MashVertex* CMashOpenGLRenderer::_CreateVertexType(MashMaterial *material,
			const sMashVertexElement *vertexDecl,
			uint32 declCount)
	{
		MashVertex *vertex = 0;
		//look for prev loaded vertex decl
		const int32 iVertexCount = m_vertexTypes.Size();
		for(int32 i = 0; i < iVertexCount; ++i)
		{
			if (m_vertexTypes[i]->IsEqual(vertexDecl, declCount))
			{
				vertex = m_vertexTypes[i];
				break;
			}
		}

		if (!vertex)
		{
			uint32 vertexSizeInBytes = 0;
			for(uint32 i = 0; i < declCount; ++i)
				vertexSizeInBytes += mash::helpers::GetVertexDeclTypeSize(vertexDecl[i].type);

			vertex = MASH_NEW_COMMON CMashOpenGLVertex(vertexDecl, declCount, vertexSizeInBytes);
			m_vertexTypes.PushBack(vertex);
			_AddCompileDependency(material, (CMashOpenGLVertex*)vertex);
		}
		return vertex;
	}

	eMASH_STATUS CMashOpenGLRenderer::SetRenderTargetDefault()
	{

        if (m_defaultRenderTarget)
        {
            if (m_currentRenderSurface != m_defaultRenderTarget)
            {
                SetRenderTarget(m_defaultRenderTarget, 0);
            }
        }
        else
        {
            if (m_currentRenderSurface)
            {
                SetRenderTarget(0, 0);
                SetViewport(m_defaultRenderTargetViewport);
            }
        }
		return aMASH_OK;
	}

	MashTexture* CMashOpenGLRenderer::LoadTextureFromFile(const MashStringc &fileName)
	{        

		GLenum e1 = glGetError();
		MashFileStream *pFileStream = m_pFileManager->CreateFileStream();
		if (pFileStream->LoadFile(fileName.GetCString(), aFILE_IO_BINARY) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
				"CMashOpenGLRenderer::LoadTextureFromFile",
				"Failed to load texture '%s'.", fileName.GetCString());

			pFileStream->Destroy();
			return 0;
		}

		uint32 openGLTexID = SOIL_load_OGL_texture_from_memory((const uint8*)pFileStream->GetData(),
			pFileStream->GetDataSizeInBytes(),
			0,
			SOIL_CREATE_NEW_ID,
			SOIL_FLAG_POWER_OF_TWO | SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);

		pFileStream->Destroy();

		if (openGLTexID == 0)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
				"CMashOpenGLRenderer::LoadTextureFromFile",
				"Failed to load texture '%s'.", fileName.GetCString());

			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				SOIL_last_result(), 
				"CMashOpenGLRenderer::LoadTextureFromFile");

			return 0;
		}

		int32 width, height;//, depth, depthSize;
		glBindTexture(GL_TEXTURE_2D, openGLTexID);

		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

		int32/* form, */redSize, greenSize, blueSize, alphaSize;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &redSize);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &greenSize);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &blueSize);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &alphaSize);
		glBindTexture(GL_TEXTURE_2D, 0);

		uint32 pixelSizeInBytes = (redSize + greenSize + blueSize + alphaSize) / 8;

		//this lib can produce some errors so we just clear the error buffer here
		GLenum e2 = glGetError();
		/*
			Note, GL_RGBA and GL_FLOAT will probably not be right for this loaded texture.
			Therefore locks are not supported for these textures. However, loaded textures
			are static and should not be locked anyway.
		*/
		MashTexture *texture = MASH_NEW_COMMON CMashOpenGLTexture(this, openGLTexID, m_textureIDCounter++,
			fileName, true, aUSAGE_STATIC, width, height, GL_RGBA, GL_FLOAT, pixelSizeInBytes);

		m_textures.insert(std::make_pair(fileName, texture));

		return texture;
	}

	MashTexture* CMashOpenGLRenderer::AddTexture(const MashStringc &sOldName,
										uint32 iWidth, 
										uint32 iHeight, 
										bool useMipmaps, 
										eUSAGE usage, 
										eFORMAT format)
	{
		MashStringc sName;
		if (ValidateTextureName(sOldName, sName) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Texture creataion failed. The name given was not unique.",
				"CMashOpenGLRenderer::AddTexture");

			return 0;
		}

		GLuint openGLTexID = 0;
		glGenTextures(1, &openGLTexID);

		GLenum glInternalFormatParam, glFormatParam, glTypeParam;
		MashToOpenGLTexFormat(format, glInternalFormatParam, glFormatParam, glTypeParam);

		glBindTexture(GL_TEXTURE_2D, openGLTexID);
		glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormatParam, iWidth, iHeight, 0, glFormatParam, glTypeParam, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (useMipmaps)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glGenerateMipmapPtr(GL_TEXTURE_2D);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		int32 width, height;//, depth, depthSize;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

		int32 redSize, greenSize, blueSize, alphaSize;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &redSize);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &greenSize);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &blueSize);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &alphaSize);
		glBindTexture(GL_TEXTURE_2D, 0);

		uint32 pixelSizeInBytes = (redSize + greenSize + blueSize + alphaSize) / 8;

		glBindTexture(GL_TEXTURE_2D, 0);

		MashTexture *texture = MASH_NEW_COMMON CMashOpenGLTexture(this, openGLTexID, m_textureIDCounter++,
			sName.GetCString(), useMipmaps, usage, width, height, glFormatParam, glTypeParam, pixelSizeInBytes);

		m_textures.insert(std::make_pair(sName, texture));

		return texture;
	}

	MashTexture* CMashOpenGLRenderer::AddCubeTexture(const MashStringc &sOldName,
										uint32 iSize,
										bool useMipmaps, 
										eUSAGE usage, 
										eFORMAT format)
	{
		MashStringc sName;
		if (ValidateTextureName(sOldName, sName) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Texture creataion failed. The name given was not unique.",
				"CMashOpenGLRenderer::AddTexture");

			return 0;
		}

		GLuint openGLTexID = 0;
		glGenTextures(1, &openGLTexID);

		GLenum glInternalFormatParam, glFormatParam, glTypeParam;
		MashToOpenGLTexFormat(format, glInternalFormatParam, glFormatParam, glTypeParam);

		glBindTexture(GL_TEXTURE_CUBE_MAP, openGLTexID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, glInternalFormatParam, iSize, iSize, 0, glFormatParam, glTypeParam, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, glInternalFormatParam, iSize, iSize, 0, glFormatParam, glTypeParam, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, glInternalFormatParam, iSize, iSize, 0, glFormatParam, glTypeParam, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, glInternalFormatParam, iSize, iSize, 0, glFormatParam, glTypeParam, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, glInternalFormatParam, iSize, iSize, 0, glFormatParam, glTypeParam, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, glInternalFormatParam, iSize, iSize, 0, glFormatParam, glTypeParam, 0);

		if (useMipmaps)
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glGenerateMipmapPtr(GL_TEXTURE_CUBE_MAP);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}

		int32 width, height;//, depth, depthSize;
		//values are the same for all faces
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_WIDTH, &width);
		glGetTexLevelParameteriv(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_TEXTURE_HEIGHT, &height);

		int32 redSize, greenSize, blueSize, alphaSize;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &redSize);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &greenSize);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &blueSize);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE, &alphaSize);

		uint32 pixelSizeInBytes = (redSize + greenSize + blueSize + alphaSize) / 8;

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		MashTexture *texture = MASH_NEW_COMMON CMashOpenGLCubeTexture(this, openGLTexID, m_textureIDCounter++,
			sName.GetCString(), useMipmaps, usage, width, height, glFormatParam, glTypeParam, pixelSizeInBytes);

		m_textures.insert(std::make_pair(sName, texture));

		return texture;
	}
    
    MashMeshBuffer* CMashOpenGLRenderer::_CreateMeshBuffer()
    {
        return MASH_NEW_COMMON CMashOpenGLMeshBuffer(this);
    }

	MashMeshBuffer* CMashOpenGLRenderer::CreateMeshBuffer(const sVertexStreamInit *initVertexStreamData,
			uint32 initVertexStreamCount,
			const MashVertex *vertexDecl, 
			const void *indexData, 
			uint32 indexCount, 
			eFORMAT indexFormat,
			eUSAGE indexUsage)
	{
		if (!initVertexStreamCount || !initVertexStreamData)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Vertex initialiser data was not given.", 
                             "CMashOpenGLRenderer::CreateMeshBuffer");
            
			return 0;
        }

		if (initVertexStreamCount > vertexDecl->GetStreamCount())
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "There are more vertex streams given then there are elements in the vertex declaration.", 
                             "CMashOpenGLRenderer::CreateMeshBuffer");
            
			return 0;
        }

		MashArray<MashVertexBuffer*> vertexStreams;
		bool error = false;

		for(uint32 i = 0; i < vertexDecl->GetStreamCount(); ++i)
		{
			if ((initVertexStreamCount > i) && (initVertexStreamData[i].dataSizeInBytes > 0))
			{
				MashVertexBuffer *vertexBuffer = CreateVertexBuffer(initVertexStreamData[i].data, initVertexStreamData[i].dataSizeInBytes, initVertexStreamData[i].usage);
				if (!vertexBuffer)
					error = true;
				else
					vertexStreams.PushBack(vertexBuffer);
			}
		}

		if (error)
		{
			for(uint32 i = 0; i < vertexStreams.Size(); ++i)
			{
				MASH_DELETE vertexStreams[i];
			}
            
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "An error occured while creating the vertex buffers.", 
                             "CMashOpenGLRenderer::CreateMeshBuffer");

			return 0;
		}

		MashIndexBuffer *indexBuffer = 0;
		if (indexData && (indexCount > 0))
		{
			indexBuffer = CreateIndexBuffer(indexData, indexCount, indexUsage, indexFormat);
			if (!indexBuffer)
			{
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "An error occured while creating the index buffer.", 
                                 "CMashOpenGLRenderer::CreateMeshBuffer");
                
				return 0;
			}
		}

		CMashOpenGLMeshBuffer *newBuffer = MASH_NEW_COMMON CMashOpenGLMeshBuffer(this, vertexStreams, indexBuffer, (MashVertex*)vertexDecl);
		_AddCompileDependency((CMashOpenGLVertex*)vertexDecl, newBuffer);
 
		return newBuffer;
	}

	MashVertexBuffer* CMashOpenGLRenderer::CreateVertexBuffer(const void *pData, uint32 iSizeInBytes, eUSAGE usage)
	{

		uint32 openGLBufferID = 0;
        glGenBuffersPtr(1, &openGLBufferID);
        glBindBufferPtr(GL_ARRAY_BUFFER, openGLBufferID);

		uint32 flag = GL_STATIC_DRAW;
		if (usage == aUSAGE_DYNAMIC)
			flag = GL_DYNAMIC_DRAW;

        glBufferDataPtr(GL_ARRAY_BUFFER, iSizeInBytes, pData, flag);

        glBindBufferPtr(GL_ARRAY_BUFFER, 0);

		CMashOpenGLVertexBuffer *newBuffer = MASH_NEW_COMMON CMashOpenGLVertexBuffer(this, openGLBufferID, usage, iSizeInBytes);
		return newBuffer;
	}

	MashIndexBuffer* CMashOpenGLRenderer::CreateIndexBuffer(const void *pData, uint32 iIndexCount, eUSAGE usage, eFORMAT format)
	{
		if (!iIndexCount)
			return 0;

		int32 iElementSizeInBytes = 0;
		if (format == aFORMAT_R16_UINT)
			iElementSizeInBytes = sizeof(int16);
		else
		{
			iElementSizeInBytes = sizeof(int32);
			//make sure the format is valid
			format = aFORMAT_R32_UINT;
		}

		const uint32 totalBufferSize = iElementSizeInBytes * iIndexCount;

		uint32 openGLBufferID = 0;
        glGenBuffersPtr(1, &openGLBufferID);
        glBindBufferPtr(GL_ELEMENT_ARRAY_BUFFER, openGLBufferID);

		uint32 flag = GL_STATIC_DRAW;
		if (usage == aUSAGE_DYNAMIC)
			flag = GL_DYNAMIC_DRAW;

        glBufferDataPtr(GL_ELEMENT_ARRAY_BUFFER, totalBufferSize, pData, flag);
        glBindBufferPtr(GL_ELEMENT_ARRAY_BUFFER, 0);

		CMashOpenGLIndexBuffer *newBuffer = MASH_NEW_COMMON CMashOpenGLIndexBuffer(this, openGLBufferID, usage, format, totalBufferSize);
		return newBuffer;
	}

	MashIndexBuffer* CMashOpenGLRenderer::CreateIndexBufferBySize(const void *pData, uint32 iSizeInBytes, eUSAGE usage, eFORMAT format)
	{
		//TODO : Remove this function from renderers
		return 0;
	}

	MashRenderSurface* CMashOpenGLRenderer::CreateRenderSurface(int32 iWidth, int32 iHeight, const eFORMAT *pFormats,
		uint32 iTargetCount, bool useMipmaps, eDEPTH_BUFFER_OPTIONS depthOptions, eFORMAT eDepthFormat)
	{        
		bool fitToScreen = false;
		uint32 newWidth = 0;
		uint32 newHeight = 0;
		if (iWidth <= 0 || iHeight <= 0)
		{
			newWidth = (uint32)m_defaultRenderTargetViewport.width;
			newHeight = (uint32)m_defaultRenderTargetViewport.height;

			fitToScreen = true;
		}
		else
		{
			newWidth = (uint32)iWidth;
			newHeight = (uint32)iHeight;
		}
        
        if ((depthOptions == aDEPTH_OPTION_SHARE_MAIN_DEPTH) && !fitToScreen)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                             "Failed to create render surface. Shared depth targets must use -1 for width and height.",
                             "CMashOpenGLRenderer::CreateRenderSurface");
        }

		MashArray<eFORMAT> formats(pFormats, iTargetCount);
		CMashOpenGLRenderSurface *newRenderSurface = MASH_NEW_COMMON CMashOpenGLRenderSurface(this, formats, useMipmaps, newWidth, newHeight, depthOptions, fitToScreen);
		if (newRenderSurface->Initialise() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Failed to create render surface.",
					"CMashOpenGLRenderer::CreateRenderSurface");

			MASH_DELETE newRenderSurface;
			return 0;
		}

		m_renderSurfaces.PushBack(newRenderSurface);

		return newRenderSurface;
	}

	MashRenderSurface* CMashOpenGLRenderer::CreateCubicRenderSurface(uint32 iSize, bool useMipmaps,
		eFORMAT eTextureFormat, bool bUseDepth, eFORMAT eDepthFormat)
	{
		CMashOpenGLCubicRenderSurface *newRenderSurface = MASH_NEW_COMMON CMashOpenGLCubicRenderSurface(this, eTextureFormat, useMipmaps, iSize, iSize, bUseDepth, eDepthFormat);
		if (newRenderSurface->Initialise() == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
					"Failed to create render surface.",
					"CMashOpenGLRenderer::CreateCubicRenderSurface");

			MASH_DELETE newRenderSurface;
			return 0;
		}

		return newRenderSurface;
	}

	eMASH_STATUS CMashOpenGLRenderer::ClearTarget(uint32 iClearFlags, const sMashColour4 &colour, f32 fZDepth)
	{
		bool disableDepthMask = false;
		if ((iClearFlags & aCLEAR_DEPTH) ||
			(iClearFlags & aCLEAR_STENCIL))
		{
			GLboolean depthMask = false;
			glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);

			/*
				Enable depth mask otherwise depth wont be cleared!
			*/
			if (!depthMask)
			{
				disableDepthMask = true;
				glDepthMask(true);
			}
		}
        
        glClearColor(colour.r, colour.g, colour.b, colour.a);
        glClearDepth(fZDepth);
        glClear(MashToOpenGLClearFlags(iClearFlags));

        //clear the default ogl buffer aswell if the engine default buffer is created and currently set
        if (m_defaultRenderTarget && (m_currentRenderSurface == m_defaultRenderTarget))
        {
			//set ogl default
            glBindFramebufferPtr(GL_FRAMEBUFFER, 0);
            
            glClearColor(colour.r, colour.g, colour.b, colour.a);
            glClearDepth(fZDepth);
            glClear(MashToOpenGLClearFlags(iClearFlags));
            
			//set back engine default
            glBindFramebufferPtr(GL_FRAMEBUFFER, ((CMashOpenGLRenderSurface*)m_defaultRenderTarget)->GetOGLFrameBuffer());
        }

		//disable it again if needed
		if (disableDepthMask)
			glDepthMask(false);
        
		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLRenderer::DrawIndexedList(const MashMeshBuffer *buffer, uint32 iVertexCount, uint32 iIndexCount, uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType/*, 
					MashTechniqueInstance *pTechnique*/)
	{
        glBindVertexArrayPtr(((CMashOpenGLMeshBuffer*)buffer)->GetOpenGLIndex());

		glDrawElements(MashToOpenGLPrimitiveType(ePrimType), 
			iIndexCount, 
			MashToOpenGLIndexFormat(buffer->GetIndexBuffer()->GetFormat()),
			0);
		++m_currentDrawCount;

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLRenderer::DrawVertexList(const MashMeshBuffer *buffer, uint32 iVertexCount,
			uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType/*, MashTechniqueInstance *pTechnique*/)
	{
        glBindVertexArrayPtr(((CMashOpenGLMeshBuffer*)buffer)->GetOpenGLIndex());

		glDrawArrays(MashToOpenGLPrimitiveType(ePrimType), 0, iVertexCount);
        glBindVertexArrayPtr(0);
		++m_currentDrawCount;
		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLRenderer::DrawVertexInstancedList(const MashMeshBuffer *buffer, uint32 iVertexCount,
				uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType, uint32 instanceCount)
	{
        glBindVertexArrayPtr(((CMashOpenGLMeshBuffer*)buffer)->GetOpenGLIndex());

        glDrawArraysInstancedPtr(MashToOpenGLPrimitiveType(ePrimType), 0, iVertexCount, instanceCount);

        glBindVertexArrayPtr(0);
		++m_currentDrawCount;

		return aMASH_OK;
	}

	eMASH_STATUS CMashOpenGLRenderer::DrawIndexedInstancedList(const MashMeshBuffer *buffer, uint32 iVertexCount, uint32 indexCount,
				uint32 iPrimitiveCount, ePRIMITIVE_TYPE ePrimType, uint32 instanceCount)
	{
        glBindVertexArrayPtr(((CMashOpenGLMeshBuffer*)buffer)->GetOpenGLIndex());

        glDrawElementsInstancedPtr(MashToOpenGLPrimitiveType(ePrimType),
			indexCount, 
			MashToOpenGLIndexFormat(buffer->GetIndexBuffer()->GetFormat()),
			0,
			instanceCount);

        glBindVertexArrayPtr(0);
		++m_currentDrawCount;

		return aMASH_OK;
	}
}
