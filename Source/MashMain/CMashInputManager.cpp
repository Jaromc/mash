//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashInputManager.h"
#include "MashHelper.h"
#include "MashTypes.h"
#include "MashLog.h"
namespace mash
{
	CMashInputManager::sPlayer::sPlayer():eventHelper()
	{
		currentContext = controllerMapMemory.end();
		controller = -1;
	}

	CMashInputManager::sPlayer::~sPlayer()
	{
		DeleteActionMap();

		if (eventHelper)
		{
			MASH_DELETE_T(sPlayerHelper, eventHelper);
			eventHelper = 0;
		}
	}

	void CMashInputManager::sPlayer::OnUpdateEventHelper()
	{
		//assumes the event helper is valid
		eventHelper->keysPressed.reset();
		eventHelper->keysReleased.reset();
	}

	void CMashInputManager::sPlayer::OnEvent(const sInputEvent &eventData)
	{
		//TODO : This 'if' can probably go. There should always be a current context
		if (currentContext != controllerMapMemory.end())
		{
			sPlayerAction *playerAction = currentContext->second[eventData.action];

			//only if an action has been set
			if (playerAction)
			{
				//only if the user has enabled input event helpers
				if (eventHelper)
				{
					eventHelper->value[playerAction->actionID] = eventData.value;
					
					if (eventData.isPressed)
					{
						eventHelper->keysPressed.set(playerAction->actionID);
						eventHelper->keysHeld.set(playerAction->actionID);
					}
					else
					{
						eventHelper->keysPressed.reset(playerAction->actionID);
						if (eventHelper->keysHeld.test(playerAction->actionID))
						{
							eventHelper->keysHeld.reset(playerAction->actionID);
							eventHelper->keysReleased.set(playerAction->actionID);
						}
					}
				}

				const_cast<sInputEvent&>(eventData).actionID = playerAction->actionID;
				playerAction->actionCallback.Call(eventData);
			}
		}
	}

	void CMashInputManager::sPlayer::DeleteActionMap()
	{
		std::map<MashStringc, MashArray<sPlayerAction*> >::iterator actionListIter = contextActions.begin();
		for(; actionListIter != contextActions.end(); ++actionListIter)
		{
			MashArray<sPlayerAction*>::Iterator actionIter = actionListIter->second.Begin();
			for(; actionIter != actionListIter->second.End(); ++actionIter)
			{
				if (*actionIter)
				{
					MASH_DELETE_T(sPlayerAction, *actionIter);
					*actionIter = 0;
				}
			}
		}

		contextActions.clear();
		controllerMapMemory.clear();
		currentContext = controllerMapMemory.end();
	}

	void CMashInputManager::sPlayer::SetActionCallback(const MashStringc &context, uint32 action, const MashEventFunctor &callback)
	{
		std::map<MashStringc, MashArray<sPlayerAction*> >::iterator actionListIter = contextActions.find(context);
		if (actionListIter != contextActions.end())
		{
			MashArray<sPlayerAction*>::Iterator actionIter = actionListIter->second.Begin();
			for(; actionIter != actionListIter->second.End(); ++actionIter)
			{
				if ((*actionIter)->actionID == action)
				{
					(*actionIter)->actionCallback = callback;
					break;
				}
			}
		}
	}

	/*
		Inserts the action and/or context if not found.
	*/
	//should be called SetActionKeys...
	void CMashInputManager::sPlayer::SetKeyActions(const MashArray<eINPUT_EVENT> &keysForAction, const MashStringc &context, uint32 action, MashEventFunctor callback)
	{
		//first reset old values

		/*
			Create the context if it has not yet been created
		*/
		std::map<MashStringc, MashArray<sPlayerAction*> >::iterator actionListIter = contextActions.find(context);
		if (actionListIter == contextActions.end())
			actionListIter = contextActions.insert(std::make_pair(context, MashArray<sPlayerAction*>())).first;

		/*
			Loop through and find any actions previously created with the same ID.
			If a match is found then we zero out any previous key data set.
		*/
		MashArray<eINPUT_EVENT>::Iterator key;
		MashArray<sPlayerAction*>::Iterator actionIter = actionListIter->second.Begin();
		for(; actionIter != actionListIter->second.End(); ++actionIter)
		{
			if ((*actionIter)->actionID == action)
			{
				key = (*actionIter)->keysForAction.Begin();
				for(; key != (*actionIter)->keysForAction.End(); ++key)
				{
					controllerMapMemory[context][*key] = 0;
				}
				break;
			}
		}

		//if the action didnt exist before then create it now. Note the break in the previous loop.
		if (actionIter == actionListIter->second.End())
		{
			sPlayerAction *newAction = MASH_NEW_T_COMMON(sPlayerAction);
			newAction->actionID = action;
			actionListIter->second.PushBack(newAction);
			actionIter = actionListIter->second.End() - 1;
		}

		//set the new data
		(*actionIter)->actionCallback = callback;
		(*actionIter)->keysForAction = keysForAction;

		//relink callbacks
		actionListIter = controllerMapMemory.insert(std::make_pair(context, MashArray<sPlayerAction*>(aKEYEVENT_SIZE, 0))).first;
		key = (*actionIter)->keysForAction.Begin();
		for(; key != (*actionIter)->keysForAction.End(); ++key)
		{
			actionListIter->second[*key] = *actionIter;
		}

		//set default context if needed
		if (currentContext == controllerMapMemory.end())
			currentContext = actionListIter;
	}

	CMashInputManager::CMashInputManager():MashInputManager(), m_sendMouseXZeroMsg(false), m_sendMouseYZeroMsg(false),
		m_mouseZeroMsgController(0), m_inputHelpersEnabled(false)
	{
		ResetEventValueSign();

		//register itself to recieve input events
        MashInputEventFunctor evFunctor(&CMashInputManager::OnEvent, this);
		RegisterReceiver(evFunctor);
	}

	CMashInputManager::~CMashInputManager()
	{
		MashArray<sPlayer*>::Iterator playerIter = m_players.Begin();
		MashArray<sPlayer*>::Iterator playerEnd = m_players.End();
		for(; playerIter != playerEnd; ++playerIter)
		{
			MASH_DELETE_T(sPlayer, *playerIter);
		}

		m_players.Clear();

		RemoveAllRecivers();
	}

	eMASH_STATUS CMashInputManager::Initialise()
	{
		return aMASH_OK;
	}

	void CMashInputManager::_EndScene()
	{
		if (m_sendMouseXZeroMsg)
		{
			sInputEvent newEvent;
			newEvent.eventType = sInputEvent::aEVENTTYPE_MOUSE;
			newEvent.controllerID = m_mouseZeroMsgController;
			newEvent.action = aMOUSEEVENT_AXISX;
			newEvent.value = 0.0f;
			ImmediateBroadcast(newEvent);
			m_sendMouseXZeroMsg = false;
		}

		if (m_sendMouseYZeroMsg)
		{
			sInputEvent newEvent;
			newEvent.eventType = sInputEvent::aEVENTTYPE_MOUSE;
			newEvent.controllerID = m_mouseZeroMsgController;
			newEvent.action = aMOUSEEVENT_AXISY;
			newEvent.value = 0.0f;
			ImmediateBroadcast(newEvent);
			m_sendMouseYZeroMsg = false;
		}
	}

	void CMashInputManager::OnEvent(const sInputEvent &data)
	{
#ifdef MASH_DEBUG
		if (data.controllerID >= m_controllers.Size())
		{
			MASH_LOG_BOUNDS_ERROR(data.controllerID, 0, m_controllers.Size(), "event controller", "CMashInputManager::OnEvent")
			return;
		}
#endif

		if (m_transformEventsToNegative[data.action])
		{
			/*
				Make sure the value is not already negative so we
				dont flip it back to positive
			*/
			if (data.value > 0.0f)
				const_cast<sInputEvent&>(data).value = -data.value;
		}

		switch(data.eventType)
		{
		case sInputEvent::aEVENTTYPE_MOUSE:
			{
				switch(data.action)
				{
				case aMOUSEEVENT_AXISX:
					{
						m_cursorPositionX += data.value;
						//sends a zero movement message at the end of a frame to emulate a joysticks return to center
						m_sendMouseXZeroMsg = true;
						m_mouseZeroMsgController = data.controllerID;

						const_cast<sInputEvent&>(data).value *= m_controllers[data.controllerID].axisSensitivity.mouseAxisX;
						break;
					}
				case aMOUSEEVENT_AXISY:
					{
						m_cursorPositionY += data.value;
						m_sendMouseYZeroMsg = true;
						m_mouseZeroMsgController = data.controllerID;

						const_cast<sInputEvent&>(data).value *= m_controllers[data.controllerID].axisSensitivity.mouseAxisY;
						break;
					}
				};
				break;
			}
		case sInputEvent::aEVENTTYPE_JOYSTICK:
			{
				switch(data.action)
				{
				case aJOYEVENT_AXIS_1_X:
					{
						const_cast<sInputEvent&>(data).value *= m_controllers[data.controllerID].axisSensitivity.joyAxis1X;
						break;
					}
				case aJOYEVENT_AXIS_1_Y:
					{
						const_cast<sInputEvent&>(data).value *= m_controllers[data.controllerID].axisSensitivity.joyAxis1Y;
						break;
					}
				case aJOYEVENT_AXIS_2_X:
					{
						const_cast<sInputEvent&>(data).value *= m_controllers[data.controllerID].axisSensitivity.joyAxis2X;
						break;
					}
				case aJOYEVENT_AXIS_2_Y:
					{
						const_cast<sInputEvent&>(data).value *= m_controllers[data.controllerID].axisSensitivity.joyAxis2Y;
						break;
					}
				case aJOYEVENT_THROTTLE_1:
					{
						const_cast<sInputEvent&>(data).value *= m_controllers[data.controllerID].axisSensitivity.throttle1;
						break;
					}
				case aJOYEVENT_THROTTLE_2:
					{
						const_cast<sInputEvent&>(data).value *= m_controllers[data.controllerID].axisSensitivity.throttle2;
						break;
					}
				}
			}
		case sInputEvent::aEVENTTYPE_CONTROLLER_CONNECT:
			{
				m_controllers[data.controllerID].isConnected = true;

				m_controllerConnectCallback.Call(data);

				break;
			}
		case sInputEvent::aEVENTTYPE_CONTROLLER_DISCONNECT:
			{
				m_controllers[data.controllerID].isConnected = false;

				m_controllerDisconnectCallback.Call(data);

				break;
			}
		};

		sPlayer *playerListener = m_controllers[data.controllerID].playerListener;
		if (playerListener)
			playerListener->OnEvent(data);
	}

	void CMashInputManager::Update()
	{
		if (m_inputHelpersEnabled)
		{
			uint32 playerCount = m_players.Size();
			for(uint32 i = 0; i < playerCount; ++i)
				m_players[i]->OnUpdateEventHelper();
		}
	}

	void CMashInputManager::ResetEventValueSign()
	{
		memset(m_transformEventsToNegative, 0, sizeof(m_transformEventsToNegative));
	}

	int32 CMashInputManager::AddContextChangeCallback(uint32 playerId, const MashStringc &contextName, MashEventFunctor callback)
	{
		m_contextChangeCallbacks.PushBack(sContextChangeCallback(playerId, contextName, callback));
		return m_contextChangeCallbacks.Back().han;
	}

	void CMashInputManager::RemoveContextChangeCallback(int32 h)
	{
		const uint32 c = m_contextChangeCallbacks.Size();
		for(uint32 i = 0; i < c; ++i)
		{
			if (m_contextChangeCallbacks[i].han == h)
			{
				m_contextChangeCallbacks.Erase(m_contextChangeCallbacks.Begin() + i);
				break;
			}
		}
	}

	void CMashInputManager::RemoveAllContextChangeCallbacks()
	{
		m_contextChangeCallbacks.Clear();
	}

	void CMashInputManager::SetEventValueSign(eINPUT_EVENT e, bool isNegative)
	{
		if (isNegative)
			m_transformEventsToNegative[e] = 1;
		else
			m_transformEventsToNegative[e] = 0;
	}

	eINPUT_CONTROLLER_TYPE CMashInputManager::GetControllerType(uint32 controller)const
	{

#ifdef MASH_DEBUG
			if (controller >= m_controllers.Size())
			{
				MASH_LOG_BOUNDS_ERROR(controller, 0, m_controllers.Size(), "controller", "CMashInputManager::GetControllerType")
				return aINPUTCONTROLLER_COUNT;
			}
#endif

		return m_controllers[controller].type;
	}

	int32 CMashInputManager::_CreateController(eINPUT_CONTROLLER_TYPE type, bool isConnected)
	{
		sController newController;
		newController.playerListener = 0;
		newController.isConnected = isConnected;
		newController.type = type;

        newController.axisThresholds.axis1 = 0.2f;
        newController.axisThresholds.axis2 = 0.2f;
        newController.axisThresholds.throttle1 = 0.01f;
        newController.axisThresholds.throttle2 = 0.01f;

		m_controllers.PushBack(newController);
		
		return m_controllers.Size() - 1;
	}

	eMASH_STATUS CMashInputManager::AssignPlayerToController(uint32 playerID, int32 controllerID)
	{
 #ifdef MASH_DEBUG
			if (playerID >= m_players.Size())
			{
				MASH_LOG_BOUNDS_ERROR(playerID, 0, m_players.Size(), "playerID", "CMashInputManager::AssignPlayerToController")
				return aMASH_FAILED;
			}
#endif

		uint32 controllerStart = 0;
		uint32 controllerEnd = 0;
		if (controllerID == -1)
		{
			controllerStart = 0;
			controllerEnd = m_controllers.Size();
		}
		else
		{
 #ifdef MASH_DEBUG
			if (controllerID >= m_controllers.Size())
			{
				MASH_LOG_BOUNDS_ERROR(controllerID, 0, m_controllers.Size(), "controllerID", "CMashInputManager::AssignPlayerToController")
				return aMASH_FAILED;
			}
#endif

			controllerStart = controllerID;
			controllerEnd = controllerID+1;
		}

		for(controllerStart; controllerStart < controllerEnd; ++controllerStart)
		{
			m_controllers[controllerStart].playerListener = m_players[playerID];
		}

		return aMASH_OK;
	}

	int32 CMashInputManager::SetPlayerActionMap(uint32 playerID,
		const MashStringc &context, 
		uint32 actionID, 
		const MashArray<eINPUT_EVENT> &keysForAction,
		MashEventFunctor callback)
	{
		const int32 rSucceeded = 1;
		const int32 rFailed = 0;
		const int32 rClash = 2;

#ifdef MASH_DEBUG
			if (playerID >= m_players.Size())
			{
				MASH_LOG_BOUNDS_ERROR(playerID, 0, m_players.Size(), "playerID", "CMashInputManager::SetPlayerActionMap")
				return rFailed;
			}
#endif

		sPlayer *player = m_players[playerID];

		int32 assign = rSucceeded;
		
		m_lastKeyClashes.Clear();
		//check for key clashes from previously set actions

		//We are only interested in a matching context. Clashes in other context are valid
		std::map<MashStringc, MashArray<sPlayerAction*> >::iterator actionListIter = player->contextActions.find(context);
		if (actionListIter != player->contextActions.end())
		{
			//for each action look at the keys set
			MashArray<sPlayerAction*>::Iterator actionIter = actionListIter->second.Begin();
			for(; actionIter != actionListIter->second.End(); ++actionIter)
			{
				//we dont need to check the same action if already set cause we are about to overwrite it
				if ((*actionIter)->actionID != actionID)
				{
					//for each of the new keys for this action, check for a clash
					const uint32 newKeyCount = keysForAction.Size();
					for(uint32 i = 0; i < newKeyCount; ++i)
					{
						//for each previous key within this action, check for clashes
						MashArray<eINPUT_EVENT>::Iterator key = (*actionIter)->keysForAction.Begin();
						for(; key != (*actionIter)->keysForAction.End(); ++key)
						{
							if (keysForAction[i] == *key)
							{
								m_lastKeyClashes.PushBack(sKeyClashInfo(*key, actionID, (*actionIter)->actionID));
								assign = rClash;
							}
						}
					}
				}
			}
		}

		//only set if there was no clashes.
		if (m_lastKeyClashes.Empty())
		{
			player->SetKeyActions(keysForAction, context, actionID, callback);
			SetCurrentPlayerContext(playerID, context);
		}
		else
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING,
					"CMashInputManager::SetPlayerActionMapy",
					"Key clashes were found for action id '%d'. Check GetPreviousKeyClashes() for clash keys.",
					actionID);
		}

		return assign;
	}

	const sJoystickThreshold* CMashInputManager::GetControllerThresholds(uint32 controllerID)const
	{
#ifdef MASH_DEBUG
			if (controllerID >= m_controllers.Size())
			{
				MASH_LOG_BOUNDS_ERROR(controllerID, 0, m_controllers.Size(), "controllerID", "CMashInputManager::GetControllerThresholds")
				return 0;
			}
#endif

		return &m_controllers[controllerID].axisThresholds;
	}

	const sControllerSensitivity* CMashInputManager::GetControllerSensitivity(uint32 controllerID)const
	{
#ifdef MASH_DEBUG
			if (controllerID >= m_controllers.Size())
			{
				MASH_LOG_BOUNDS_ERROR(controllerID, 0, m_controllers.Size(), "controllerID", "CMashInputManager::GetControllerSensitivity")
				return 0;
			}
#endif

		return &m_controllers[controllerID].axisSensitivity;
	}

	void CMashInputManager::SetControllerThresholds(uint32 controllerID, const sJoystickThreshold &thresholds)
	{
#ifdef MASH_DEBUG
			if (controllerID >= m_controllers.Size())
			{
				MASH_LOG_BOUNDS_ERROR(controllerID, 0, m_controllers.Size(), "controllerID", "CMashInputManager::SetControllerThresholds")
				return;
			}
#endif

		m_controllers[controllerID].axisThresholds.axis1 = fabs(thresholds.axis1);
		m_controllers[controllerID].axisThresholds.axis2 = fabs(thresholds.axis2);
		m_controllers[controllerID].axisThresholds.throttle1 = fabs(thresholds.throttle1);
		m_controllers[controllerID].axisThresholds.throttle2 = fabs(thresholds.throttle2);
	}

	void CMashInputManager::SetControllerSensitivity(uint32 controllerID, const sControllerSensitivity &sensitivity)
	{
#ifdef MASH_DEBUG
			if (controllerID >= m_controllers.Size())
			{
				MASH_LOG_BOUNDS_ERROR(controllerID, 0, m_controllers.Size(), "controllerID", "CMashInputManager::SetControllerSensitivity")
				return;
			}

#endif

		m_controllers[controllerID].axisSensitivity = sensitivity;
	}
	
	int32 CMashInputManager::CreatePlayer()
	{
		uint32 playerID = m_players.Size();
		sPlayer *newPlayer = MASH_NEW_T_COMMON(sPlayer);

		if (m_inputHelpersEnabled)
			newPlayer->eventHelper = MASH_NEW_T_COMMON(sPlayerHelper)();

		m_players.PushBack(newPlayer);

		return playerID;
	}

	eMASH_STATUS CMashInputManager::SetCurrentPlayerContext(uint32 playerID, const MashStringc &context)
	{
#ifdef MASH_DEBUG
			if (playerID >= m_players.Size())
			{
				MASH_LOG_BOUNDS_ERROR(playerID, 0, m_players.Size(), "playerID", "CMashInputManager::SetCurrentPlayerContext")
				return aMASH_FAILED;
			}
#endif

		sPlayer *player = m_players[playerID];
		MashStringc oldContext;
		
		//make sure last context is valid.
		if (player->currentContext != player->controllerMapMemory.end())
			oldContext = player->currentContext->first;

		player->currentContext = player->controllerMapMemory.find(context);
		if (player->currentContext != player->controllerMapMemory.end())
		{
			const uint32 c = m_contextChangeCallbacks.Size();
			for(uint32 i = 0; i < c; ++i)
			{
				if ((m_contextChangeCallbacks[i].playerId == playerID) && m_contextChangeCallbacks[i].contextName == oldContext)
				{
					sInputEvent e;
					e.eventType = sInputEvent::aEVENTTYPE_CONTROLLER_CONTEXT_CHANGE;
					m_contextChangeCallbacks[i].contextCallback.Call(e);
					break;
				}
			}

			return aMASH_OK;
		}

		return aMASH_FAILED;
	}

	void CMashInputManager::ResetPlayerAcionMap(uint32 playerID)
	{
#ifdef MASH_DEBUG
			if (playerID >= m_players.Size())
			{
				MASH_LOG_BOUNDS_ERROR(playerID, 0, m_players.Size(), "playerID", "CMashInputManager::ResetPlayerAcionMap")
				return;
			}
#endif

		m_players[playerID]->DeleteActionMap();
	}

	void CMashInputManager::SetPlayerActionCallback(uint32 playerID, 
			const MashStringc &context, 
			uint32 actionID, 
			const MashEventFunctor &callback)
	{
#ifdef MASH_DEBUG
			if (playerID >= m_players.Size())
			{
				MASH_LOG_BOUNDS_ERROR(playerID, 0, m_players.Size(), "playerID", "CMashInputManager::SetPlayerActionCallback")
				return;
			}
#endif

		m_players[playerID]->SetActionCallback(context, actionID, callback);
	}

	void CMashInputManager::CreateDefaultActionMap(uint32 playerID, const MashStringc &context, MashEventFunctor callback)
	{
		SetEventValueSign(aKEYEVENT_A, true);
		SetEventValueSign(aKEYEVENT_S, true);
		SetEventValueSign(aKEYEVENT_LEFT, true);
		SetEventValueSign(aKEYEVENT_DOWN, true);

		MashArray<mash::eINPUT_EVENT> actions;
		actions.PushBack(aJOYEVENT_AXIS_1_X);
		actions.PushBack(aKEYEVENT_A);
		actions.PushBack(aKEYEVENT_D);
		actions.PushBack(aKEYEVENT_LEFT);
		actions.PushBack(aKEYEVENT_RIGHT);

		SetPlayerActionMap(playerID, context, aINPUTMAP_MOVE_HORIZONTAL, actions, callback);

		actions.Clear();
		actions.PushBack(aJOYEVENT_AXIS_1_Y);
		actions.PushBack(aKEYEVENT_W);
		actions.PushBack(aKEYEVENT_S);
		actions.PushBack(aKEYEVENT_UP);
		actions.PushBack(aKEYEVENT_DOWN);

		SetPlayerActionMap(playerID, context, aINPUTMAP_MOVE_VERTICAL, actions, callback);

		actions.Clear();
		actions.PushBack(aJOYEVENT_AXIS_2_X);
		actions.PushBack(aMOUSEEVENT_AXISX);

		SetPlayerActionMap(playerID, context, aINPUTMAP_LOOK_HORIZONTAL, actions, callback);

		actions.Clear();
		actions.PushBack(aJOYEVENT_AXIS_2_Y);
		actions.PushBack(aMOUSEEVENT_AXISY);

		SetPlayerActionMap(playerID, context, aINPUTMAP_LOOK_VERTICAL, actions, callback);

		actions.Clear();
		actions.PushBack(aJOYEVENT_B6);
		actions.PushBack(aMOUSEEVENT_B1);
		actions.PushBack(aKEYEVENT_CTRL);

		SetPlayerActionMap(playerID, context, aINPUTMAP_FIRE, actions, callback);

		actions.Clear();
		actions.PushBack(aJOYEVENT_B8);
		actions.PushBack(aMOUSEEVENT_B2);
		actions.PushBack(aKEYEVENT_MENU);

		SetPlayerActionMap(playerID, context, aINPUTMAP_SECONDARY_FIRE, actions, callback);

		actions.Clear();
		actions.PushBack(aJOYEVENT_B7);
		actions.PushBack(aKEYEVENT_SPACE);

		SetPlayerActionMap(playerID, context, aINPUTMAP_JUMP, actions, callback);

	}

	bool CMashInputManager::HelperIsKeyHeld(uint32 player, uint32 action)const
	{
#ifdef MASH_DEBUG
			if (m_players.Size() <= player)
			{
				MASH_LOG_BOUNDS_ERROR(player, 0, m_players.Size(), "player", "CMashInputManager::HelperIsKeyHeld")
				return false;
			}
#endif

		sPlayerHelper *helper = m_players[player]->eventHelper;
		if (helper)
			return helper->keysHeld.test(action);//return (bool)helper->keysHeld[action];

		return false;
	}

	bool CMashInputManager::HelperIsKeyPressed(uint32 player, uint32 action)const
	{
#ifdef MASH_DEBUG
			if (m_players.Size() <= player)
			{
				MASH_LOG_BOUNDS_ERROR(player, 0, m_players.Size(), "player", "CMashInputManager::HelperIsKeyPressed")
				return false;
			}
#endif

		sPlayerHelper *helper = m_players[player]->eventHelper;
		if (helper)
			return helper->keysPressed.test(action);//return (bool)helper->keysPressed[action];

		return false;
	}

	bool CMashInputManager::HelperIsKeyReleased(uint32 player, uint32 action)const
	{
#ifdef MASH_DEBUG
			if (m_players.Size() <= player)
			{
				MASH_LOG_BOUNDS_ERROR(player, 0, m_players.Size(), "player", "CMashInputManager::HelperIsKeyReleased")
				return false;
			}
#endif

		sPlayerHelper *helper = m_players[player]->eventHelper;
		if (helper)
			return helper->keysReleased.test(action);//return (bool)helper->keysReleased[action];

		return false;
	}

	f32 CMashInputManager::HelperGetKeyValue(uint32 player, uint32 action)const
	{
#ifdef MASH_DEBUG
			if (m_players.Size() <= player)
			{
				MASH_LOG_BOUNDS_ERROR(player, 0, m_players.Size(), "player", "CMashInputManager::HelperGetKeyValue")
				return 0.0f;
			}
#endif

		sPlayerHelper *helper = m_players[player]->eventHelper;
		if (helper)
        {
			return helper->value[action];
        }

		return 0.0f;
	}

	void CMashInputManager::EnabledInputHelpers(bool value)
	{
		if (m_inputHelpersEnabled != value)
		{
			m_inputHelpersEnabled = value;
			if (!m_inputHelpersEnabled)
			{
				uint32 playerCount = m_players.Size();
				for(uint32 i = 0; i < playerCount; ++i)
				{
					if (m_players[i]->eventHelper)
					{
						MASH_DELETE_T(sPlayerHelper, m_players[i]->eventHelper);
						m_players[i]->eventHelper = 0;
					}
				}
			}
			else
			{
				uint32 playerCount = m_players.Size();
				for(uint32 i = 0; i < playerCount; ++i)
				{
					if (!m_players[i]->eventHelper)
						m_players[i]->eventHelper = MASH_NEW_T_COMMON(sPlayerHelper)();
				}
			}
		}
	}
}
