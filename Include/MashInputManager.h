//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_INPUT_MANAGER_H_
#define _MASH_INPUT_MANAGER_H_

#include "MashEventTypes.h"
#include "MashReferenceCounter.h"
#include "MashArray.h"
#include "MashString.h"

namespace mash
{
	class MashVector2;
	class MashRectangle2;
    struct sControllerSensitivity;
    struct sJoystickThreshold;

    /*!
        The input manager handles all key mappings and input from external devices.
    */
	class MashInputManager : public MashEventDispatch<const sInputEvent>, public MashReferenceCounter
	{
	public:
		struct sKeyClashInfo
		{
			eINPUT_EVENT newKey;
			uint32 newKeyWasAssignedToAction;
			uint32 actionItsConflictingWith;

			sKeyClashInfo():
			newKey(aKEYEVENT_NONE), newKeyWasAssignedToAction(0), actionItsConflictingWith(0){}

			sKeyClashInfo(eINPUT_EVENT n, uint32 a1, uint32 a2):
			newKey(n), newKeyWasAssignedToAction(a1), actionItsConflictingWith(a2){}
		};
	public:
		MashInputManager():MashEventDispatch<const sInputEvent>(), MashReferenceCounter(){}
		virtual ~MashInputManager(){}

		//! Enables input helper functions.
		/*!
			This enables some helper functions that may be useful for the user.
			Also, some internal classes may access these functions in which case
			they will be activated automatically. Those classes that do activate this
			state so in their documentation.

			These function were made optional to save on memory and computation
			if not needed.

			\param value Enable/disable the helpers.
		*/
		virtual void EnabledInputHelpers(bool value) = 0;

		//! Is the key being held down.
		/*!
			This function must be activated by calling EnabledInputHelpers().
			The result of this function is dependent on the current context.
			\param player PlayerID.
			\param action Action to check.
			\return True if held, false otherwise.
		*/
		virtual bool HelperIsKeyHeld(uint32 player, uint32 action)const = 0;

		//! Was the key just pressed (not held).
		/*!
			This function must be activated by calling EnabledInputHelpers().
			The result of this function is dependent on the current context.
			\param player PlayerID.
			\param action Action to check.
			\return True if just pressed, false otherwise.
		*/
		virtual bool HelperIsKeyPressed(uint32 player, uint32 action)const = 0;

		//! Was the key just released.
		/*!
			This function must be activated by calling EnabledInputHelpers().
			The result of this function is dependent on the current context.
			\param player PlayerID.
			\param action Action to check.
			\return True if just released, false otherwise.
		*/
		virtual bool HelperIsKeyReleased(uint32 player, uint32 action)const = 0;

		//! Axis or button value.
		/*!
			This function must be activated by calling EnabledInputHelpers().
			The result of this function is dependent on the current context.
			\param player PlayerID.
			\param action Action to check.
			\return Axis or button value.
		*/
		virtual f32 HelperGetKeyValue(uint32 player, uint32 action)const = 0;

        //! Emulates key presses as input from a joystick
		/*!
			This function is used to convert events (mainly made for keyboard events)
			into an axis. For example, keyboard events return 1 if the key is pressed.
			We could set the 'a' key to return -1 and 'd' to return 1 so that it
			emmulates the x axis on a joystick.

			Note, this does not affect event receivers that get there
			input data directly, ie, not mapped.
         
            \param ev Key event.
            \param isNegative Should the key return -1 or 1.
		*/
		virtual void SetEventValueSign(eINPUT_EVENT ev, bool isNegative) = 0;
        
		//! Resets all key signs.
		virtual void ResetEventValueSign() = 0;

        //! Creates an action map
        /*!
            This allows multiple inputs from multiple devices to register
            as the same thing and greatly simplifies handling different input
            devices. For example, 'A', 'D', 'Arrow left', 'Arrow right', 'Joystick X axis', 
            could all be registered as the 'Move horizontal' action under a particular context.
         
            A context is a different state in your application, for example, "menu", "play",
            "pause". This allows the same key eg, 'A', 'D'... to register as different actions
            depending on a particular context.
         
            The action ID is some value set by the user and identifies this new action. This ID
            must be greater than or equal to aINPUTMAP_USER_REGION and less than aINPUTMAP_MAX.

			If this function return 2, then 1 or more new eINPUT_EVENTs clash with previous settings.
			This will result in the new keys not taking effect. You can view the clash list by calling
			GetPreviousKeyClashes().
         
            \param playerID Players ID created with CreatePlayer().
            \param context This actions context.
            \param actionID User defined ID for this action >= aINPUTMAP_USER_REGION and < aINPUTMAP_MAX.
            \param keysForAction Keys to assign to this action.
            \param callback User defined callback for when this action is triggered.
            \return Function status. 0 == Failed, 1 == Succeeded, 2 == Key clashes exist with previous settings.
        */
		virtual int32 SetPlayerActionMap(uint32 playerID,
			const MashStringc &context, 
			uint32 actionID, 
			const MashArray<eINPUT_EVENT> &keysForAction = MashArray<eINPUT_EVENT>(),
			MashEventFunctor callback = MashEventFunctor()) = 0;

		//! Gets the key clash list.
		/*!
			This array contains any key clashes that may have occured from the last
			call to SetPlayerActionMap(). See that function for more information.

			\return Key clash list.
		*/
		virtual const MashArray<sKeyClashInfo>& GetPreviousKeyClashes()const = 0;

        //! Assigns a player to a particular controler.
        /*!
            Players and controllers are independent of each other until bound using
            this function. CreatePlayer() must first be called to create a player then
            that player can be bound to a controller less than GetControllerCount(). Note
            the controller ID is based on the number of devices currently detected and
            is therefore runtime dependent.
         
            \param playerID Player ID created with CreatePlayer().
            \param controlerID Controller ID less than GetControllerCount(). If -1 is used then the player will be assigned to all controllers.
            \return Function status.
        */
		virtual eMASH_STATUS AssignPlayerToController(uint32 playerID, int32 controllerID) = 0;
        
        //! Create a new player.
        /*!
            This function just creates an ID to which input can be bound.
            \return New player ID.
        */
		virtual int32 CreatePlayer() = 0;
        
        //! Gets the controller type.
        /*!
            \param controllerID Controller ID less than GetControllerCount().
        */ 
		virtual eINPUT_CONTROLLER_TYPE GetControllerType(uint32 controller)const = 0;
        
        //! Number of devices currently connected.
        /*!
            \return Number of devices currently conected.
        */
		virtual uint32 GetControllerCount()const = 0;
        
        //! Create defaut action map.
        /*!
            Helper function. Quickly sets up input handling. See eINPUT_KEY_MAP action IDs.

			w,a,s,d, arrow keys, and joy axis 1 are mapped to aINPUTMAP_MOVE_HORIZONTAL, aINPUTMAP_MOVE_VERTICAL.
			mouse axis, and joy axis 2 mapped to aINPUTMAP_LOOK_HORIZONTAL, aINPUTMAP_LOOK_VERTICAL.
			crtl, b1, b6 are mapped to aINPUTMAP_FIRE.
			menu, b2, b8 are mapped to aINPUTMAP_SECONDARY_FIRE.
			space, b7 are mapped to aINPUTMAP_JUMP.

			If any of these keys will clash with previously set keys then this will not be successful. You can 
			call ResetPlayerAcionMap() to remove any previously set values.

            \param playerID Play ID created with CreatePlayer().
            \param context Application state that will trigger these events.
            \param callback Callback that will be called when actions are triggered.
        */
		virtual void CreateDefaultActionMap(uint32 playerID, const MashStringc &context, MashEventFunctor callback = MashEventFunctor()) = 0;
        
        //! Resets the action map for a player.
        /*!
            \param playerID Player ID created with CreatePlayer().
        */
		virtual void ResetPlayerAcionMap(uint32 playerID) = 0;
        
        //! Sets the callback for some action.
        /*!
            This function sets a callback after an action has been created. Note the
            callack can be set when actions are created.
         
            \param playerID player ID created with CreatePlayer().
            \param context User defined context.
            \param actionID User defined action.
            \param callback Callback that will be set to an action.
        */
		virtual void SetPlayerActionCallback(uint32 playerID, 
			const MashStringc &context, 
			uint32 actionID, 
			const MashEventFunctor &callback) = 0;

        //! Sets the active context for a player.
        /*!
			Note that changing context can cause, for example, release key events to go missing
			for listeners in the previous context. This will be like sticky keys occuring.
			To fix this, you should set a callback for when a context change occurs. This allows
			you to reset your values back to a default state. Use AddContextChangeCallback().

            \param playerID Player ID created with CreatePlayer().
            \param context New active context.
            \return Function status.
        */
		virtual eMASH_STATUS SetCurrentPlayerContext(uint32 playerID, const MashStringc &context) = 0;

		//! Adds a context change callback. Called when the selected context is left.
		/*!
			Changing context can cause, for example, release key events to go missing
			for listeners in the previous context. This will be like sticky keys occuring.
			To fix this, you should set a callback for when a context change occurs. This allows
			you to reset your values back to a default state.

			\param playerId Player ID created with CreatePlayer().
			\param contextName Context name.
			\param callback Callback to add.
			\return Handle to this callback so you can remove it using RemoveContextChangeCallback().
		*/
		virtual int32 AddContextChangeCallback(uint32 playerId, const MashStringc &contextName, MashEventFunctor callback) = 0;

		//! Remove a callback set using AddContextChangeCallback().
		virtual void RemoveContextChangeCallback(int32 h) = 0;

		//! Removes all callbacks set using AddContextChangeCallback().
		virtual void RemoveAllContextChangeCallbacks() = 0;
        
        //! Joystick axis thresholds.
        /*!
            \param controllerID Controller ID.
            \return Data about a controllers axis thresholds.
        */
		virtual const sJoystickThreshold* GetControllerThresholds(uint32 controllerID)const = 0;
        
        //! Sets a joysticks axis thresholds.
        /*!
            Axis thresholds (deadzones) remove unwanted noise from joystick axis.
            The higher the threshold, the more that movement is needed to trigger an event.
            \param controllerID Controller ID.
            \param threshold Threshold data for a joysticks axis.
        */
		virtual void SetControllerThresholds(uint32 controllerID, const sJoystickThreshold &thresholds) = 0;

        //! Creates a new controlelr ID.
        /*!
            This is used internally to create controllers as they are conected.
            \param type Controller type,
            \param isConnected True if it's currently connected. False otherwise. 
            \return New controller ID.
        */
		virtual int32 _CreateController(eINPUT_CONTROLLER_TYPE type, bool isConnected) = 0;

        //! Sets the callback for when a controller is connected.
        /*!
            \param callback Callback.
        */
		virtual void SetOnControllerConnectCallback(MashEventFunctor &callback) = 0;
        
        //! Sets the callback for when a controller is disconnected.
        /*!
            \param callback Callback.
        */
		virtual void SetOnControllerDisconnectCallback(MashEventFunctor &callback) = 0;

		//! Updates the input manager (internal use).
		virtual void Update() = 0;

		//! The cursor position is screen coordinates.
        /*!
            This can be used to emulate the mouse in a non mouse environment.
            \param x X Position.
            \param y Y position.
        */
		virtual void GetCursorPosition(int32 &x, int32 &y)const = 0;
        
        //! The cursor position is screen coordinates.
        /*!
            This can be used to emulate the mouse in a non mouse environment.
            \return Cursor position in screen coordinaes.
         */
		virtual mash::MashVector2 GetCursorPosition()const = 0;
        
        //! Internal Use.
        /*!
            Sets the cursor position in screen coordinates.
            \param x X position.
            \param y Y position.
        */
		virtual void _SetCursorPosition(int32 x, int32 y) = 0;

		//! Gets the sensitivity settings for a controller.
		/*!
			\param controllerID Controller to query.
			\return Sensitivity settings.
		*/
		virtual const sControllerSensitivity* GetControllerSensitivity(uint32 controllerID)const = 0;
		
		//! Sets the controller sensitivity.
		/*!
			Raw axis values are multiplied by this value.
			Use values > -1 && < 1 to reduce sensitivity.
			Setting a sensitivity value to 0 will result in 0 axis movement.
			Setting a sensitivity value to a negative will invert the axis value.
		*/
		virtual void SetControllerSensitivity(uint32 controllerID, const sControllerSensitivity &sensitivity) = 0;
	};
}

#endif