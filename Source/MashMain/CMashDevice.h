//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _CMASH_DEVICE_H_
#define _CMASH_DEVICE_H_

#include "MashDataTypes.h"

#include "MashLog.h"
#include "MashLog.h"

#include "MashDevice.h"
#include "MashCreationParameters.h"
#ifdef 0
#define MASH_SHOW_LOGO
#endif
namespace mash
{
	class MashVideo;		
	class CMashSceneManager;

	class CMashInputManager;
	class CMashTimer;
	class CMashFileManager;
	class MashScriptManager;
	class MashAIManager;
	class MashGUIManager;
	class MashMaterialBuilder;
	class MashPhysics;
    class MashMaterial;
    
	class CMashDevice : public MashDevice
	{
	protected:
		//! Video Device
		MashVideo *m_pRenderer;

		//! Scene Manager
		CMashSceneManager *m_pSceneManager;
		
		//! Input manager
		CMashInputManager *m_pInputManager;

		//! Timer
		MashTimer *m_pTimer;

		//! File manager
		CMashFileManager *m_pFileManager;

		//! AI Manager
		MashAIManager *m_pAIManager;

		MashPhysics *m_pPhysicsManager;

		MashScriptManager *m_pScriptManager;

		MashGUIManager *m_pGUIManager;

		MashGameLoop *m_activeGameLoop;
        
        bool m_isResizable;

		bool m_isGameLoopInitialise;

		uint64 m_fps;

		MashStringc m_debugFilePath;
		MashStringc m_memoryAllocationFilePath;
        
#ifdef MASH_SHOW_LOGO
        MashMaterial *m_screenLogoMaterial;
#endif

		typedef bool(CMashDevice::*m_gameStateFunctPtr)();
		m_gameStateFunctPtr m_currentGameStatePtr;
		uint64 m_lastGameUpdateTime;
		uint64 m_currentGameTime;
		uint64 m_lastFpsTime;
		uint64 m_fpsFrameCounter;
		uint64 m_fixedGameUpdateTimeMS;
		f32 m_fixedGameUpdateTimeSeconds;

		eGAME_STATE m_activeGameState;
		eGAME_STATE m_changeToGameState;
		void _SwitchGameState();

		void UpdateFps();
		void RenderLogo();
		bool GameStateUpdate();
		bool GameStatePause();
	
		void EnterGameLoop();

		virtual void BeginUpdate(){}
		virtual void EndUpdate(){}

        void OnWindowFocusLost();
        void OnWindowFocusRegained();

		virtual bool PollMessages() = 0;
        virtual void SyncInputDeviceWithCurrentState() = 0;
	public:
		CMashDevice(const MashStringc &debugFilePath);
		virtual ~CMashDevice();

		/*!
			Gets the renderer.
			\return Renderer.
		*/
		virtual MashVideo* GetRenderer();

		/*!
			Gets the scene manager.
			\return Scene manager. 
		*/
		virtual MashSceneManager* GetSceneManager();

		/*!
			Gets the input manager.
			\return Input manager.
		*/
		virtual MashInputManager* GetInputManager();

		/*!
			Gets the timer.
			\return Timer.
		*/
		virtual MashTimer* GetTimer();

		/*!
			Gets the file manager.
			\return File manager.
		*/
		virtual MashFileManager* GetFileManager();

		/*!
			Gets the AI manager.
			\return AI manager.
		*/
		virtual MashAIManager* GetAIManager();

		/*!
			Gets the Physics manager.
			\return Physics manager.
		*/
		virtual MashPhysics* GetPhysicsManager();

		virtual MashScriptManager* GetScriptManager();

		virtual MashGUIManager* GetGUIManager();

		void SetGameState(eGAME_STATE gameState);

		void RestGameLoop();

		virtual void Destroy();

		//API specific
		virtual eMASH_STATUS Initialise(const mash::sMashDeviceSettings &settings) = 0;
		//common load
		virtual eMASH_STATUS LoadComponents(const mash::sMashDeviceSettings &settings);

		void SetGameLoop(MashGameLoop *gameLoop);
		eGAME_STATE GetGameState()const;
        
        void SetResizable(bool state);
        bool IsResizable()const;

		bool IsGameLoopInitialised()const;
		uint32 GetFPS()const;

		const MashStringc& GetDebugFilePath()const;
		const MashStringc& GetMemoryAllocationFilePath()const;

		//windows only
		virtual void* GetHwnd()const{return 0;}
	};

	inline MashDevice::eGAME_STATE CMashDevice::GetGameState()const
	{
		return m_activeGameState;
	}

	inline const MashStringc& CMashDevice::GetDebugFilePath()const
	{
		return m_debugFilePath;
	}

	inline const MashStringc& CMashDevice::GetMemoryAllocationFilePath()const
	{
		return m_memoryAllocationFilePath;
	}

	inline uint32 CMashDevice::GetFPS()const
	{
		return m_fps;
	}

	inline bool CMashDevice::IsGameLoopInitialised()const
	{
		return m_isGameLoopInitialise;
	}
    
    inline void CMashDevice::SetResizable(bool state)
    {
        m_isResizable = state;
    }
    
    inline bool CMashDevice::IsResizable()const
    {
        return m_isResizable;
    }
}

#endif
