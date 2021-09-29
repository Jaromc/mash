//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_DEVICE_H_
#define _MASH_DEVICE_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"
#include "MashString.h"

namespace mash
{
	class MashTimer;
	class MashInputManager;
	class MashPhysics;
	class MashScriptManager;
	class MashGUIManager;
	class MashGameLoop;
	class MashVideo;
	class MashSceneManager;
	class MashFileManager;

	/*!
		This is the main hub for the engine. All of the main conponents are created and
		accessed from here.
	*/
	class MashDevice : public MashReferenceCounter
	{
	public:
		/*!
			Allows static access to the device.
			This pointer is only valid after init.
			This pointer must not be Dropped.
		*/
		static MashDevice *StaticDevice;

		enum eGAME_STATE
		{
			/*!
				This is the default state.
			*/
			aGAME_STATE_PLAY,
			/*!
				This state will continue to update the input manager
				and call MashGameLoop::Pause() each frame. The current scene will
				not be interpolated forward.

				A typical implimentation of Pause() could continue to render
				the scene as it was when paused plus render a GUI menu overlay
				for user options.

				The game state can be set back to aGAME_STATE_PLAY and the scene
				will continue to move/operate how it was left.
			*/
			aGAME_STATE_PAUSE
		};
	public:
		//! Constructor
		MashDevice():MashReferenceCounter(){StaticDevice = this;}
		virtual ~MashDevice(){}

		//! Returns the current device type.
		/*!
			\return Curretn device type.
		*/
        virtual eMASH_DEVICE_TYPE GetDeviceType()const = 0;

		//! Returns the script manager.
		/*!
			\return Script manager.
		*/
		virtual MashScriptManager* GetScriptManager() = 0;

		//! Returns the renderer.
		/*!
			\return Render manager.
		*/
		virtual MashVideo* GetRenderer() = 0;

		//! Returns the scene manager.
		/*!
			\return Scene manager.
		*/
		virtual MashSceneManager* GetSceneManager() = 0;
		
		//! Returns the timer.
		/*!
			\return Timer.
		*/
		virtual MashTimer* GetTimer() = 0;

		//! Returns the input manager.
		/*!
			\return Input manager.
		*/
		virtual MashInputManager* GetInputManager() = 0;

		//! Returns the file manager.
		/*!
			\return File manager.
		*/
		virtual MashFileManager* GetFileManager() = 0;

		//! Returns the physics manager.
		/*!
			\return physics manager.
		*/
		virtual MashPhysics* GetPhysicsManager() = 0;

		//! Returns the gui manager.
		/*!
			\return gui manager.
		*/
		virtual MashGUIManager* GetGUIManager() = 0;

		//! Sets the active game loop.
		/*!
			This game loop is what the engine will process. This function grabs
			a copy of the loop, then when the loop quits the loop will be dropped. 
			If a game loop was previously set then it will be dropped.
			See MashGameLoop for more info.

			\param gameLoop New active game loop.
		*/
		virtual void SetGameLoop(MashGameLoop *gameLoop) = 0;

		//! Locks a mouse to the center of the screen.
		/*!
			Mouse movement will still be registered.
			This function is normally used with HideMouseCursor().

			After this yout can render a custom cursor for simply put a crosshair in the middle
			of the screen for fps games.

			\param state True to lock the mouse, false to free the mouse.
		*/
		virtual void LockMouseToScreenCenter(bool state) = 0;

		//! Gets the mouse lock state.
		/*!
			\return True if the mouse is locked to the center of the screen. False if its free.
		*/
		virtual bool IsMouseLockedToScreenCenter()const = 0;

		//! Hides to default cursor.
		/*!
			After this you could render a custom cursor.

			\param state True to hide the default mouse cursor. False to show it.
		*/
		virtual void HideMouseCursor(bool state) = 0;

		//! Gets the mouse hidden state.
		/*!
			\return True if the default cursor is hidden. False otherwise.
		*/
		virtual bool IsMouseCursorHidden()const = 0;

		//! Returns true if the initialise function has been finalized.
		/*!
			\return True if the Initialise function has been called in the current game loop.
		*/
		virtual bool IsGameLoopInitialised()const = 0;

		//! Returns the current FPS.
		/*!
			\return Frames per second.
		*/
		virtual uint32 GetFPS()const = 0;

		//! Returns the file path of the debug output file.
		/*!
			\return Debug file path.
		*/
		virtual const MashStringc& GetDebugFilePath()const = 0;

		//! Sets the window caption.
		/*!
			This is will only show up when run in windowed mode.

			\param text Window text.
		*/
		virtual void SetWindowCaption(const int8 *text) = 0;

		//! Sets the current engine game state.
		/*!
			See eGAME_STATE for game state information.

			\param New game state.
		*/
		virtual void SetGameState(eGAME_STATE gameState) = 0;

		//! Gets the active game state.
		virtual eGAME_STATE GetGameState()const = 0;

		//! Sends the active game loop back to MashGameLoop::Initialise().
		/*!
			Note this simply sends the active game loop back to MashGameLoop::Initialise(),
			and does not destroy anything internally. This must be done by the user.

			Any new objects loaded (such as materials) in MashGameLoop::Initialise() will
			be compiled automatically if needed. And any meta data loaded (such as extra 
			internal mesh data) will be destroyed (if no longer needed).
		*/
		virtual void RestGameLoop() = 0;

        //! Causes this application to sleep for milliseconds.
        /*!
            This can allow other threads and/or applications to use
            some CPU.

            \param ms Milliseconds to sleep. A value of 0 will yeild for the shortest possible time.
        */
        virtual void Sleep(uint32 ms)const = 0;

		//! Internal use only.
		/*!
			Impliments drawing for each API.
		*/
		virtual void _Draw() = 0;
	};
}

#endif
