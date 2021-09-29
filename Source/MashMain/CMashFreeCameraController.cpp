//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashFreeCameraController.h"
#include "MashSceneNode.h"
#include "MashInputManager.h"

namespace mash
{
	CMashFreeCameraController::CMashFreeCameraController(uint32 playerId,
									MashInputManager *pInputManager,
                                   const MashStringc &inputContext,
                                   f32 fRotationSpeed,
                                   f32 fLinearSpeed,
								   const MashFreeMovementController::sInputActionMovement *customActions):MashFreeMovementController(),
    m_pInput(pInputManager),
    m_fLinearSpeed(fLinearSpeed),
    m_fRotationSpeed(fRotationSpeed),
    m_fRotationX(0.0f),
    m_fRotationY(0.0f),
    m_moveXDelta(0.0f),
    m_moveYDelta(0.0f),
    m_rotationXDelta(0.0f),
    m_rotationYDelta(0.0f),
    m_freeLookEnabled(false),
    m_inputContext(inputContext),
    m_inputState(true),
	m_contextChangeHandle(-1)
	{
		MashFreeMovementController::sInputActionMovement actionsToUse;

		if (!customActions)
		{
			pInputManager->CreateDefaultActionMap(playerId, inputContext);
			actionsToUse.moveX = aINPUTMAP_MOVE_HORIZONTAL;
			actionsToUse.moveZ = aINPUTMAP_MOVE_VERTICAL;
			actionsToUse.lookY = aINPUTMAP_LOOK_HORIZONTAL;
			actionsToUse.lookZ = aINPUTMAP_LOOK_VERTICAL;
			actionsToUse.holdToRotate = aINPUTMAP_SECONDARY_FIRE;
		}
		else
		{
			actionsToUse = *customActions;
		}

		pInputManager->SetPlayerActionCallback(playerId, inputContext, actionsToUse.moveX, 
                                               mash::MashInputEventFunctor(&CMashFreeCameraController::OnMoveHorizontal, this));
        
		pInputManager->SetPlayerActionCallback(playerId, inputContext, actionsToUse.moveZ, 
                                               mash::MashInputEventFunctor(&CMashFreeCameraController::OnMoveVertical, this));
        
		pInputManager->SetPlayerActionCallback(playerId, inputContext, actionsToUse.lookY, 
                                               mash::MashInputEventFunctor(&CMashFreeCameraController::OnLookHorizontal, this));
        
		pInputManager->SetPlayerActionCallback(playerId, inputContext, actionsToUse.lookZ, 
                                               mash::MashInputEventFunctor(&CMashFreeCameraController::OnLookVertical, this));
        
		pInputManager->SetPlayerActionCallback(playerId, inputContext, actionsToUse.holdToRotate, 
                                               mash::MashInputEventFunctor(&CMashFreeCameraController::OnFreeLookState, this));

		m_contextChangeHandle = pInputManager->AddContextChangeCallback(playerId, inputContext, mash::MashInputEventFunctor(&CMashFreeCameraController::OnContextChange, this));

		/*
			need to grab a copy for the destructor
		*/
		m_pInput->Grab();
		
	}
    
	CMashFreeCameraController::~CMashFreeCameraController()
	{
		if (m_contextChangeHandle != -1)
			m_pInput->RemoveContextChangeCallback(m_contextChangeHandle);

		m_pInput->Drop();
	}

	void CMashFreeCameraController::OnContextChange(const mash::sInputEvent &e)
	{
		m_rotationXDelta = 0;
		m_rotationYDelta = 0;
		m_moveXDelta = 0;
		m_moveYDelta = 0;
		m_freeLookEnabled = false;
	}
    
	void CMashFreeCameraController::OnLookHorizontal(const mash::sInputEvent &e)
	{
		if (m_freeLookEnabled)
			m_rotationXDelta = -e.value;
		else
			m_rotationXDelta = 0.0f;
	}
    
	void CMashFreeCameraController::OnLookVertical(const mash::sInputEvent &e)
	{
		if (m_freeLookEnabled)
			m_rotationYDelta = -e.value;
		else
			m_rotationYDelta = 0.0f;
	}
    
	void CMashFreeCameraController::OnMoveHorizontal(const mash::sInputEvent &e)
	{
		m_moveXDelta = e.value;
	}
    
	void CMashFreeCameraController::OnMoveVertical(const mash::sInputEvent &e)
	{
		m_moveYDelta = e.value;
	}
    
	void CMashFreeCameraController::OnFreeLookState(const mash::sInputEvent &e)
	{
		if (e.isPressed)
			m_freeLookEnabled = true;
		else
			m_freeLookEnabled = false;
	}
    
	void CMashFreeCameraController::SetInputState(bool enable)
	{
		m_inputState = enable;
		if (!enable)
			m_freeLookEnabled = false;
	}
    
    void CMashFreeCameraController::OnNodeUpdate(MashSceneNode *sceneNode, f32 dt)
    {
        if (dt == 0.0f)
			return;
        
		/*
         We do the input check here so when its re-enabled any input data that may have happened
         during the same frame we be registered.
         */
		if (m_inputState)
		{
			m_fRotationX += (m_rotationXDelta * dt) * m_fRotationSpeed;
			m_fRotationY += (m_rotationYDelta * dt) * m_fRotationSpeed;
            
			if (m_fRotationX > mash::math::TwoPi())
				m_fRotationX -= mash::math::TwoPi();
			else if (m_fRotationX < -mash::math::TwoPi())
				m_fRotationX += mash::math::TwoPi();
            
			//cap pitch to 80 degrees
			if (m_fRotationY > 1.4f)
				m_fRotationY = 1.4f;
			else if (m_fRotationY < -1.4f)
				m_fRotationY = -1.4f;
            
			mash::MashVector3 vRight(1.0f, 0.0f, 0.0);
			mash::MashVector3 vDirection(0.0f, 0.0f, 1.0f);
			mash::MashVector3 vUp(0.0f, 1.0f, 0.0f);
            
			MashMatrix4 mat;
			mat.SetRotationFromAxisAngle(vUp, m_fRotationX);
			vRight = mat.TransformVectorTranspose(vRight);
			vDirection = mat.TransformVectorTranspose(vDirection);
            
			mat.SetRotationFromAxisAngle(vRight, m_fRotationY);
			vUp = mat.TransformVectorTranspose(vUp);
			vDirection = mat.TransformVectorTranspose(vDirection);
            
			vDirection.Normalize();
			vRight = vUp.Cross(vDirection);
			vRight.Normalize();
            
			sceneNode->AddPosition(vRight * (m_moveXDelta * m_fLinearSpeed * dt));
			sceneNode->AddPosition(vDirection * (m_moveYDelta * m_fLinearSpeed * dt));
            
			mash::MashQuaternion qX, qY;
			qX.SetRotationY(-m_fRotationX);
			qY.SetRotationX(-m_fRotationY);
			sceneNode->SetOrientation(qX * qY);
		}
    }
}