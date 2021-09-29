//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashDevice.h"
#include "MashVideo.h"
#include "CMashSceneManager.h"
#include "CMashInputManager.h"
#include "CMashTimer.h"
#include "CMashFileManager.h"
#include "MashScriptManager.h"
#include "CMashMemoryTracker.h"
#include "MashPhysics.h"
#include "MashGUIManager.h"
#include "MashMaterialManager.h"
#include "MashMaterialBuilder.h"
#include "Mash.h"
#include "MashMemoryManager.h"

namespace mash
{
	CMashDevice *g_pMash_Device = 0;
	extern "C"
	{
		typedef MashVideo* (*CREATERENDERDEVICE)(const sMashDeviceSettings &creationParameters);
		typedef mash::MashVideo* (*CREATERENDERDEVICEA)();

		typedef MashVideo* (*CREATEOPENGLRENDERDEVICE)(const void*);
	}

	CMashDevice::CMashDevice(const MashStringc &debugFilePath):m_pRenderer(0),
		m_pSceneManager(0),m_pPhysicsManager(0)/*, m_pGUIManager(0)*/, m_isResizable(false), m_isGameLoopInitialise(false),
		m_fps(0), m_debugFilePath(debugFilePath), m_pGUIManager(0),
		m_pInputManager(0), m_pTimer(0), m_pScriptManager(0), m_activeGameLoop(0), 
		m_activeGameState(aGAME_STATE_PAUSE), m_changeToGameState(aGAME_STATE_PLAY), m_currentGameStatePtr(0)
	{
#ifdef MASH_SHOW_LOGO
        m_screenLogoMaterial = 0;  
#endif
		/*
			The memory tracker is enabled here so that the device pointer and some
			of its internal objects (that are destroyed after the log is created)
			are not included in the debug output.

			It's also here so that any containers created before the device don't get 
			included in the output.
		*/
#ifdef MASH_MEMORY_TRACKING_ENABLED
		CMashMemoryTracker::Instance()->EnableTracking(true);
#endif

		SetGameState(aGAME_STATE_PLAY);
	}

	CMashDevice::~CMashDevice()
	{	
#ifdef MASH_SHOW_LOGO
        if (m_screenLogoMaterial)
        {
            m_screenLogoMaterial->Drop();
            m_screenLogoMaterial = 0;
        }
#endif
        
		if (m_activeGameLoop)
		{
			m_activeGameLoop->Drop();
			m_activeGameLoop = 0;
		}

		if (m_pInputManager)
		{
			m_pInputManager->Drop();
			m_pInputManager = 0;
		}

		if (m_pTimer)
		{
			m_pTimer->Drop();
			m_pTimer = 0;
		}

		if (m_pFileManager)
		{
			m_pFileManager->Drop();
			m_pFileManager = 0;
		}

		if (m_pPhysicsManager)
		{
			m_pPhysicsManager->Drop();
			m_pPhysicsManager = 0;
		}

		if (m_pScriptManager)
		{
			m_pScriptManager->Drop();
			m_pScriptManager = 0;
		}

		if (m_pGUIManager)
		{
			m_pGUIManager->Drop();
			m_pGUIManager = 0;
		}

		if (m_pSceneManager)
		{
			m_pSceneManager->Drop();
			m_pSceneManager = 0;
		}
		
		if (m_pRenderer)
		{
			m_pRenderer->Drop();
			m_pRenderer = 0;
		}

		

		CMashMemoryTracker::Instance()->OutputMemoryLog();
		MashMemoryManager::DestroyInstance();

		MashLog::DestroyInstance();
	}

	MashGUIManager* CMashDevice::GetGUIManager()
	{
		return m_pGUIManager;
	}

	MashScriptManager* CMashDevice::GetScriptManager()
	{
		return m_pScriptManager;
	}

	MashPhysics* CMashDevice::GetPhysicsManager()
	{
		return m_pPhysicsManager;
	}

	MashAIManager* CMashDevice::GetAIManager()
	{
		return m_pAIManager;
	}

	MashFileManager* CMashDevice::GetFileManager()
	{
		return m_pFileManager;
	}

	MashVideo* CMashDevice::GetRenderer()
	{
		return m_pRenderer;
	}

	MashSceneManager* CMashDevice::GetSceneManager()
	{
		return m_pSceneManager;
	}

	MashInputManager* CMashDevice::GetInputManager()
	{
		return m_pInputManager;
	}

	MashTimer* CMashDevice::GetTimer()
	{
		return m_pTimer;
	}

	eMASH_STATUS CMashDevice::LoadComponents(const mash::sMashDeviceSettings &settings)
	{
        m_pFileManager = MASH_NEW_COMMON CMashFileManager();
        
		m_pRenderer = settings.rendererFunctPtr();
        
		if (!m_pRenderer)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                              "Failed to create a renderer from the given function pointer.", 
                              "CMashDevice::CreateRenderer");
            
			return aMASH_FAILED;
		}
        
        m_pTimer = MASH_NEW_COMMON CMashTimer(settings.fixedTimeStep);
        
        m_pInputManager = MASH_NEW_COMMON CMashInputManager();

		m_pSceneManager = MASH_NEW_COMMON CMashSceneManager();
		
		const uint32 iRootPathCount = settings.rootPaths.Size();
		for(uint32 i = 0; i < iRootPathCount; ++i)
			m_pFileManager->AddRootPath(settings.rootPaths[i].GetCString());

        //syncs the mouse pos and cage with the current device
        if (m_pInputManager->Initialise() == aMASH_FAILED)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Failed to initialise the input manager.", 
                             "CMashDevice::LoadComponents");
            
            return aMASH_FAILED;
        }
        
        SyncInputDeviceWithCurrentState();
        
		if (m_pFileManager->Initialise() == aMASH_FAILED)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Failed to initialise the file manager.", 
                             "CMashDevice::LoadComponents");
            
            return aMASH_FAILED;
        }
        
        if (settings.virtualFileLoaderPtr)
        {
            settings.virtualFileLoaderPtr(m_pFileManager);
        }

#ifdef MASH_WINDOWS
		if (m_pRenderer->_Initialise(this, settings, GetHwnd()) == aMASH_FAILED)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Failed to initialise the renderer.", 
                             "CMashDevice::LoadComponents");
            
            return aMASH_FAILED;
        }
#else
		if (m_pRenderer->_Initialise(this, settings, 0) == aMASH_FAILED)
        {
            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Failed to initialise the renderer.", 
                             "CMashDevice::LoadComponents");
            
            return aMASH_FAILED;
        }
#endif

		m_pSceneManager->_Initialise(m_pRenderer, m_pInputManager, settings);

		if (settings.physicsManagerFunctPtr)
		{
			m_pPhysicsManager = settings.physicsManagerFunctPtr();
			
            if (m_pPhysicsManager)
            {
                m_pPhysicsManager->_Initialise(m_pRenderer, m_pSceneManager, settings);
            }
            else
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to create the physics manager from the given function pointer.", 
                                 "CMashDevice::LoadComponents");
                
                return aMASH_FAILED;
            }
		}
		

		/*
			TODO : Make the GUI manager a plug in.
		*/
		if (settings.guiManagerFunctPtr)
		{
			m_pGUIManager = settings.guiManagerFunctPtr();

            if (m_pGUIManager)
            {
                if (m_pGUIManager->_Initialise(settings, m_pRenderer, m_pInputManager) == aMASH_FAILED)
                {
                    MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                        "Failed to initialise the GUI manager.", 
                        "CMashDevice::LoadComponents");

                    return aMASH_FAILED;
                }
            }
            else
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to create the GUI manager from the given function pointer.", 
                                 "CMashDevice::LoadComponents");
                
                return aMASH_FAILED;
            }
		}

		if (settings.scriptManagerFunctPtr)
		{
            m_pScriptManager = settings.scriptManagerFunctPtr();
            
            if (m_pScriptManager)
            {
                if (m_pScriptManager->_Initialise(this) == aMASH_FAILED)
                {
                    MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                     "Failed to initialise the script manager.", 
                                     "CMashDevice::LoadComponents");
                    
                    return aMASH_FAILED;
                }
            }
            else
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to create the script manager from the given function pointer.", 
                                 "CMashDevice::LoadComponents");
                
                return aMASH_FAILED;
            }
		}
        
#ifdef MASH_SHOW_LOGO
        
#ifdef MASH_LOG_ENABLED
        //suppress logo messages
        MashLog::Instance()->SuppressMessages(true);      
		m_pRenderer->GetMaterialManager()->SetCompiledEffectOutputDirectory("");
		m_pRenderer->GetMaterialManager()->SetIntermediateEffectOutputDirectory("");
#endif
        const char *logoMaterialString = "material _MashLogoMaterial\
        {\
            vertex\
            {\
                position rgb32float\
                texcoord rg32float\
            }\
            technique Standard\
            {\
                vertexprogram \"auto\" \"_MashLogo_Vertex.eff\" \"vsmain\"\
                pixelprogram \"auto\" \"_MashLogo_Pixel.eff\" \"psmain\"\
            }\
            sampler2D FontTexture\
            {\
                index 0\
                texture \"_MashScreenLogo.dds\"\
                minmagfilter linear\
                mipfilter linear\
                addressu clamp\
                addressv clamp\
            }\
            rasteriser\
            {\
                depthtestenabled false\
                depthwriteenabled false\
                depthcmp never\
            }\
            blendstate\
            {\
                blendingenabled true\
                srcblend srcalpha\
                destblend invsrcalpha\
                blendop add\
                srcblendalpha zero\
                destblendalpha zero\
                blendopalpha add\
                writemask all\
            }\
        }";
        
       const char *logoVertexString = "source\
        {\
        \n#ifdef _DEFINE_DIRECTX\n\
		float2 logoGetTexcoods(float2 t)\
		{\
			return t;\
		}\
        \n#endif\n\
        \n#ifdef _DEFINE_OPENGL\n\
		float2 logoGetTexcoods(float2 t)\
		{\
			return float2(t.x, 1 - t.y);\
		}\
        \n#endif\n\
            struct VS_INPUT\
            {\
                float3 Position : POSITION0;\
                float2 texcoord:TEXCOORD0;\
            };\
            struct VS_OUTPUT\
            {\
                float2 texcoord : TEXCOORD0;\
                float4 PositionH : SV_POSITION;\
            };\
            VS_OUTPUT vsmain( VS_INPUT Input)\
            {\
                VS_OUTPUT Output;\
                Output.PositionH = float4(Input.Position, 1.0f);\
                Output.texcoord = logoGetTexcoods(Input.texcoord);\
                \
                return Output;\
            }\
        }";
        
        const char *logoPixelString = "autos\
        {\
            sampler2D autoSampler0\
        }\
        source\
        {\
            struct VS_OUTPUT\
            {\
                float2 texcoord : TEXCOORD0;\
            };\
            float4 psmain(VS_OUTPUT input) : SV_TARGET0\
            {\
                return tex2D(autoSampler0, input.texcoord);\
            }\
        }";
        
        //insert logo here
        uint8 mashScreenLogo[] = {
            0x00};

        m_pFileManager->AddStringToVirtualFileSystem("_MashLogoMaterial.mtl", logoMaterialString);
        m_pFileManager->AddStringToVirtualFileSystem("_MashLogo_Vertex.eff", logoVertexString);
        m_pFileManager->AddStringToVirtualFileSystem("_MashLogo_Pixel.eff", logoPixelString);
        m_pFileManager->AddFileToVirtualFileSystem("_MashScreenLogo.dds", (const void*)mashScreenLogo, sizeof(mashScreenLogo));
        
        m_screenLogoMaterial = m_pRenderer->GetMaterialManager()->GetMaterial("_MashLogoMaterial", "_MashLogoMaterial.mtl", 0, 0);
        m_screenLogoMaterial->Grab();
        //remove the material from the manager so it cant be tampered with.
        m_pRenderer->GetMaterialManager()->RemoveMaterial(m_screenLogoMaterial);
        //remove texture for the same reason
        m_pRenderer->RemoveTextureFromCache(m_screenLogoMaterial->GetFirstTechnique()->GetTexture(0)->texture);
        
        
        m_screenLogoMaterial->CompileTechniques(m_pFileManager, m_pSceneManager, aMATERIAL_COMPILER_EVERYTHING);
        
        //remove loaded data
        m_pFileManager->AddStringToVirtualFileSystem("_MashLogoMaterial.mtl", 0);
        m_pFileManager->AddStringToVirtualFileSystem("_MashLogo_Vertex.eff", 0);
        m_pFileManager->AddStringToVirtualFileSystem("_MashLogo_Pixel.eff", 0);
        m_pFileManager->AddFileToVirtualFileSystem("_MashScreenLogo.dds", 0, 0);
#ifdef MASH_LOG_ENABLED
        MashLog::Instance()->SuppressMessages(false);  
		m_pRenderer->GetMaterialManager()->SetCompiledEffectOutputDirectory(settings.compiledShaderOutputDirectory);
		m_pRenderer->GetMaterialManager()->SetIntermediateEffectOutputDirectory(settings.intermediateShaderOutputDirectory);
#endif
        
#endif
		return aMASH_OK;
	}

	void CMashDevice::RestGameLoop()
	{
		m_isGameLoopInitialise = false;
		EnterGameLoop();
	}

	void CMashDevice::SetGameLoop(MashGameLoop *gameLoop)
	{
		m_isGameLoopInitialise = false;

		if (gameLoop)
			gameLoop->Grab();

		if (m_activeGameLoop)
			m_activeGameLoop->Drop();

		m_activeGameLoop = gameLoop;

		if (m_activeGameLoop)
		{
			RestGameLoop();
		}

		if (m_activeGameLoop)
		{
			m_activeGameLoop->Drop();
			m_activeGameLoop = 0;
		}
	}

	void CMashDevice::UpdateFps()
	{        
		++m_fpsFrameCounter;

		uint64 fpsDifference = m_currentGameTime - m_lastFpsTime;
		if (fpsDifference > 1000)
		{
			m_fps = (m_fpsFrameCounter * fpsDifference) / 1000;
			m_lastFpsTime = m_currentGameTime;
			m_fpsFrameCounter = 0;
		}
	}

	void CMashDevice::RenderLogo()
	{
#ifdef MASH_SHOW_LOGO
		if (m_screenLogoMaterial)
		{
			m_pRenderer->SetRenderTargetDefault();
		    
			const sTexture *tex = m_screenLogoMaterial->GetActiveTechnique()->GetTexture(0);
			uint32 texWidth, texHeight;
			tex->texture->GetSize(texWidth, texHeight);
			sMashViewPort activeViewport = m_pRenderer->GetViewport();
		    
			sMashViewPort viewport;
			viewport.x = activeViewport.width - texWidth;
			viewport.y = activeViewport.height - texHeight;
			viewport.width = texWidth;
			viewport.height = texHeight;
			viewport.minZ = 0.0f;
			viewport.maxZ = 1.0f;
		    
			m_pRenderer->SetViewport(viewport);
		    
			if (m_screenLogoMaterial->OnSet())
				m_pRenderer->DrawFullScreenQuad();
		    
			m_pRenderer->SetViewport(activeViewport);
		}
#endif
	}

	bool CMashDevice::GameStateUpdate()
	{
		while(((m_currentGameTime - m_lastGameUpdateTime) > m_fixedGameUpdateTimeMS))
		{
			if (PollMessages())
				return true; //quit

			f32 updateDTSeconds = m_fixedGameUpdateTimeSeconds;

			/*
				This takes care of the rare oocasion when the app looses and regains focus.
				timeElapsed will be very large (greater than 1sec) and the scene will jump forward
				by whatever that value is instead of by the fixed time.
			*/
			uint64 timeDiff = m_currentGameTime - m_lastGameUpdateTime;
			if (timeDiff > 1000)
			{
				updateDTSeconds = (f32)timeDiff * 0.001f;//convert ms to seconds
				m_lastGameUpdateTime += timeDiff;
			}
			else
				m_lastGameUpdateTime += m_fixedGameUpdateTimeMS;

			BeginUpdate();
			
			if (m_pSceneManager)
				m_pSceneManager->_Update(updateDTSeconds);

			if (m_activeGameLoop->Update(updateDTSeconds))
				return true;//quit

			if (m_pSceneManager)
				m_pSceneManager->_LateUpdate();

			if (m_pInputManager)
			{
				//needs to be done last for helper input functions
				m_pInputManager->Update();
				m_pInputManager->_EndScene();
			}

			//physics overrides all
			 if (m_pPhysicsManager)
				 m_pPhysicsManager->_Simulate(updateDTSeconds);

			EndUpdate();

			m_pTimer->_IncrementUpdateCount();
		}

		m_activeGameLoop->LateUpdate();

		if (m_pRenderer->BeginRender() == mash::aMASH_OK)
		{
			/*
				The scene nodes store the last interpolated state. So we need to keep
				track of the last interpolated time so the nodes know how far to lerp relative
				to the last frame. This time is then reset when the scene is updated.
			*/
			f32 interpDiff = (f32)(m_currentGameTime - m_lastGameUpdateTime);
			m_pTimer->_SetFrameInterpolatorTime(interpDiff / (f32)m_fixedGameUpdateTimeMS);
			m_activeGameLoop->Render();
#ifdef MASH_SHOW_LOGO

			/*
				This is called MashVideo::EndRender. However when rendering the logo
				we want to do this prior.
			*/
			m_pSceneManager->FlushGeometryBuffers();

          RenderLogo();
#endif
			if (m_pRenderer->EndRender() == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to end rendering.", 
                                 "CMashDevice::GameStateUpdate");
			}
		}
		else
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Failed to begin rendering.", 
                             "CMashDevice::GameStateUpdate");
		}

		m_pTimer->_IncrementFrameCount();
       UpdateFps();

	   return false;
	}

    void CMashDevice::OnWindowFocusLost()
    {
        if (m_activeGameLoop)
            m_activeGameLoop->WindowFocusLost();
    }

    void CMashDevice::OnWindowFocusRegained()
    {
        if (m_activeGameLoop)
            m_activeGameLoop->WindowFocusRegained();
    }

	bool CMashDevice::GameStatePause()
	{
		if (PollMessages())
			return true;//quit

		if (m_pInputManager)
		{
			//needs to be done last for helper input functions
			m_pInputManager->Update();
			m_pInputManager->_EndScene();
		}

		if (m_pRenderer->BeginRender() == mash::aMASH_OK)
		{
			if (m_activeGameLoop->Pause())
				return true;//quit

#ifdef MASH_SHOW_LOGO

			/*
				This is called MashVideo::EndRender. However when rendering the logo
				we want to do this prior.
			*/
			m_pSceneManager->FlushGeometryBuffers();

          RenderLogo();
#endif
			if (m_pRenderer->EndRender() == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Failed to end rendering.", 
                                 "CMashDevice::GameStatePause");
			}
		}
		else
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                             "Failed to begin rendering.", 
                             "CMashDevice::GameStatePause");
		}

		m_pTimer->_IncrementFrameCount();
		UpdateFps();

		return false;
	}

	void CMashDevice::SetGameState(eGAME_STATE gameState)
	{
		m_changeToGameState = gameState;

		/*
			Unfortunatly we need to check for a game state change each update because changing
			the pointers within the function pointers has some bad effects. eg, Switching with
			GameUpdate::PollMessage() will come back out of the stack at GamePause::PollMessage().
		*/

		//if the pointer is null we can safley change at this point
		if (m_currentGameStatePtr == 0)
			_SwitchGameState();
	}

	void CMashDevice::_SwitchGameState()
	{
		switch(m_changeToGameState)
		{
		case aGAME_STATE_PLAY:
			{
				if (m_pTimer)
				{
					m_currentGameTime = m_pTimer->GetTimeSinceProgramStart();

					/*
						We force an update on the first loop. This just makes sure
						we have enough space to subtract. It should almost never
						be needed.
					*/
					while (m_currentGameTime < m_fixedGameUpdateTimeMS)
						m_currentGameTime = m_pTimer->GetTimeSinceProgramStart();

					m_lastGameUpdateTime = m_currentGameTime - (m_fixedGameUpdateTimeMS + 1);
				}

				m_currentGameStatePtr = &CMashDevice::GameStateUpdate;
				break;
			}
		case aGAME_STATE_PAUSE:
			{
				m_currentGameStatePtr = &CMashDevice::GameStatePause;
				break;
			}
		default:
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Invalid game state given.", 
					"CMashDevice::_SwitchGameState");

				return;
			}
		}

		m_activeGameState = m_changeToGameState;
	}

	void CMashDevice::EnterGameLoop()
	{
		m_pSceneManager->_OnBeginUserInitialise();

		if (m_activeGameLoop->Initialise())
			return;

		m_pSceneManager->_OnEndUserInitialise();

		m_isGameLoopInitialise = true;

		m_fixedGameUpdateTimeSeconds = m_pTimer->GetFixedTimeInSeconds();
		m_fixedGameUpdateTimeMS = m_fixedGameUpdateTimeSeconds * 1000.0f;

		m_currentGameTime = m_pTimer->GetTimeSinceProgramStart();

		/*
			We force an update on the first loop. This just makes sure
			we have enough space to subtract. It should almost never
			be needed.
		*/
		while (m_currentGameTime < m_fixedGameUpdateTimeMS)
			m_currentGameTime = m_pTimer->GetTimeSinceProgramStart();

		m_lastGameUpdateTime = m_currentGameTime - (m_fixedGameUpdateTimeMS + 1);

		m_lastFpsTime = m_lastGameUpdateTime;
		m_fpsFrameCounter = 0;

		while(!(*this.*m_currentGameStatePtr)())
		{
			m_currentGameTime = m_pTimer->GetTimeSinceProgramStart();

			if (m_activeGameState != m_changeToGameState)
			{
				_SwitchGameState();
			}
		}
	}

	void CMashDevice::Destroy()
	{
		MASH_DELETE this;
	}
}
