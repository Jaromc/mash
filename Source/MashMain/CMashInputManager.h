//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_INPUT_MANAGER_H_
#define _C_MASH_INPUT_MANAGER_H_

#include "MashInputManager.h"
#include "MashVector2.h"
#include "MashRectangle2.h"
#include "MashTypes.h"
#include <map>
#include <bitset>
namespace mash
{
	class CMashInputManager : public MashInputManager
	{
	protected:
		struct sPlayerAction
		{
			MashArray<eINPUT_EVENT> keysForAction;
			MashEventFunctor actionCallback;
			uint32 actionID;

			sPlayerAction():actionCallback(), actionID(0){}
			~sPlayerAction(){}
		};

		struct sPlayerHelper
		{
			std::bitset<aINPUTMAP_MAX> keysPressed;
			std::bitset<aINPUTMAP_MAX> keysHeld;
			std::bitset<aINPUTMAP_MAX> keysReleased;
			f32 value[aINPUTMAP_MAX];//axis/value

			sPlayerHelper()
			{
				keysPressed.reset();
				keysHeld.reset();
				keysReleased.reset();
				memset(value, 0, sizeof(value));
			}

			~sPlayerHelper(){}
		};

		class sPlayer
		{
		public:
			/*
				This holds the actions, sorted by context, that are created
				by the user. For example, under the context "play" you might
				have the actions "move horizontal", "move vertical", etc...
			*/
			std::map<MashStringc, MashArray<sPlayerAction*> > contextActions;

			/*
				For each context, there is one pointer slot created for each
				input event possible (so its a little memory hungry). These
				pointers then point back to the action created by the user.
				For example, the events 'A', 'D', 'LT', 'RT', may all point
				back to the "move horizontal" action.
			*/
			std::map<MashStringc, MashArray<sPlayerAction*> > controllerMapMemory;

			/*
				Direct pointer for speed.
			*/
			std::map<MashStringc, MashArray<sPlayerAction*> >::iterator currentContext;

			int32 controller;
			sPlayerHelper *eventHelper;

			sPlayer();
			~sPlayer();
			void OnUpdateEventHelper();
			void OnEvent(const sInputEvent &eventData);
			void DeleteActionMap();
			void SetActionCallback(const MashStringc &context, uint32 action, const MashEventFunctor &callback);
			/*
				Inserts the action and/or context if not found.
			*/
			//should be called SetActionKeys...
			void SetKeyActions(const MashArray<eINPUT_EVENT> &keysForAction, const MashStringc &context, uint32 action, MashEventFunctor callback = MashEventFunctor());
		};

		struct sController
		{
			sPlayer *playerListener;
			bool isConnected;
			eINPUT_CONTROLLER_TYPE type;
			sJoystickThreshold axisThresholds;
			sControllerSensitivity axisSensitivity;

			sController():playerListener(0), isConnected(false)
			{
				axisThresholds.axis1 = 0.1f;
				axisThresholds.axis2 = 0.1f;
				axisThresholds.throttle1 = 0.1f;
				axisThresholds.throttle2 = 0.1f;

				axisSensitivity.mouseAxisX = 1.0f;
				axisSensitivity.mouseAxisY = 1.0f;
				axisSensitivity.joyAxis1X = 1.0f;
				axisSensitivity.joyAxis1Y = 1.0f;
				axisSensitivity.joyAxis2X = 1.0f;
				axisSensitivity.joyAxis2Y = 1.0f;
				axisSensitivity.throttle1 = 1.0f;
				axisSensitivity.throttle2 = 1.0f;
			}
		};

		struct sContextChangeCallback
		{
			MashEventFunctor contextCallback;
			int32 han;
			MashStringc contextName;
			uint32 playerId;

			sContextChangeCallback(uint32 pid, const MashStringc &name, MashEventFunctor _c):contextCallback(_c),
				contextName(name), playerId(pid)
			{
				static int32 nexthandle = 0;
				han = nexthandle++;
			}
		};

		/*
			for each controller is a list of key states
		*/
		MashArray<sController> m_controllers;
		MashArray<sPlayer*> m_players;
		int8 m_transformEventsToNegative[aKEYEVENT_SIZE];

        //Needs to be float for OSX
		float m_cursorPositionX;
		float m_cursorPositionY;

		MashEventFunctor m_controllerConnectCallback;
		MashEventFunctor m_controllerDisconnectCallback;

		//used to emulate a joystick with the mouse
		bool m_sendMouseXZeroMsg;
		bool m_sendMouseYZeroMsg;
		uint16 m_mouseZeroMsgController;

		bool m_inputHelpersEnabled;

		MashArray<sKeyClashInfo> m_lastKeyClashes;
		MashArray<sContextChangeCallback> m_contextChangeCallbacks;
	public:
		CMashInputManager();
		virtual ~CMashInputManager();

		eMASH_STATUS Initialise();
		
		void OnEvent(const sInputEvent &data);

		void ResetEventValueSign();
		void SetEventValueSign(eINPUT_EVENT e, bool isNegative);
		const MashArray<sKeyClashInfo>& GetPreviousKeyClashes()const;

		int32 SetPlayerActionMap(uint32 playerID,
			const MashStringc &context, 
			uint32 actionID, 
			const MashArray<eINPUT_EVENT> &keysForAction = MashArray<eINPUT_EVENT>(),
			MashEventFunctor callback = MashEventFunctor());

		eMASH_STATUS AssignPlayerToController(uint32 playerID, int32 controllerID);
		int32 CreatePlayer();
		eINPUT_CONTROLLER_TYPE GetControllerType(uint32 controller)const;
		uint32 GetControllerCount()const;

		eMASH_STATUS SetCurrentPlayerContext(uint32 playerID, const MashStringc &context);

		void CreateDefaultActionMap(uint32 playerID, const MashStringc &context, MashEventFunctor callback = MashEventFunctor());
		void ResetPlayerAcionMap(uint32 playerID);

		void SetPlayerActionCallback(uint32 playerID, 
			const MashStringc &context, 
			uint32 actionID, 
			const MashEventFunctor &callback);

		void SetControllerSensitivity(uint32 controllerID, const sControllerSensitivity &sensitivity);
		void SetControllerThresholds(uint32 controllerID, const sJoystickThreshold &thresholds);
		const sJoystickThreshold* GetControllerThresholds(uint32 controllerID)const;
		const sControllerSensitivity* GetControllerSensitivity(uint32 controllerID)const;
		int32 _CreateController(eINPUT_CONTROLLER_TYPE type, bool isConnected);

		void SetOnControllerConnectCallback(MashEventFunctor &callback);
		void SetOnControllerDisconnectCallback(MashEventFunctor &callback);

		void Update();
		void EnabledInputHelpers(bool value);
		bool HelperIsKeyHeld(uint32 player, uint32 action)const;
		bool HelperIsKeyPressed(uint32 player, uint32 action)const;
		bool HelperIsKeyReleased(uint32 player, uint32 action)const;
		f32 HelperGetKeyValue(uint32 player, uint32 action)const;

		int32 AddContextChangeCallback(uint32 playerId, const MashStringc &contextName, MashEventFunctor callback);
		void RemoveContextChangeCallback(int32 h);
		void RemoveAllContextChangeCallbacks();

		void GetCursorPosition(int32 &x, int32 &y)const;
		mash::MashVector2 GetCursorPosition()const;

		void _SetCursorPosition(int32 x, int32 y);

		/*
			This function is called at the end of each frame to send
			a zero movement mouse axis message to emulate a joysticks
			return to center.
			Without this, special mouse code would be needed by the user
			to reset the mouse delta state after each frame.
		*/
		void _EndScene();
	};

	inline const MashArray<MashInputManager::sKeyClashInfo>& CMashInputManager::GetPreviousKeyClashes()const
	{
		return m_lastKeyClashes;
	}

	inline void CMashInputManager::SetOnControllerConnectCallback(MashEventFunctor &callback)
	{
		m_controllerConnectCallback = callback;
	}

	inline void CMashInputManager::SetOnControllerDisconnectCallback(MashEventFunctor &callback)
	{
		m_controllerDisconnectCallback = callback;
	}

	inline void CMashInputManager::_SetCursorPosition(int32 x, int32 y)
	{
		m_cursorPositionX = x;
		m_cursorPositionY = y;
	}

	/*inline uint32 CMashInputManager::GetControllerCount()const
	{
		return m_gamepads.Size();
	}*/

	inline void CMashInputManager::GetCursorPosition(int32 &x, int32 &y)const
	{
		x = m_cursorPositionX;
		y = m_cursorPositionY;
	}

	inline mash::MashVector2 CMashInputManager::GetCursorPosition()const
	{
		return mash::MashVector2(m_cursorPositionX, m_cursorPositionY);
	}

	inline uint32 CMashInputManager::GetControllerCount()const
	{
		return m_controllers.Size();
	}
}

#endif