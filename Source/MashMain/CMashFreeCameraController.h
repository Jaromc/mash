//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _CMASH_FREE_CAMERA_CONTROLLER_H_
#define _CMASH_FREE_CAMERA_CONTROLLER_H_

#include "MashFreeMovementController.h"
#include "MashEventTypes.h"
#include "MashVector2.h"
#include "MashString.h"
namespace mash
{
	class MashInputManager;

	class CMashFreeCameraController : public MashFreeMovementController
	{
    private:
		//speed of movement
		f32 m_fLinearSpeed;
		//speed of mouse rotation
		f32 m_fRotationSpeed;
		//current rotation on the x axis
		f32 m_fRotationX;
		//current rotation on the y axis
		f32 m_fRotationY;
        
		//pointer to the input manager to handle camera movement
		MashInputManager *m_pInput;
		//stores the last position of the mouse so we can determine
		//how much the mouse has moved since the last update
		MashVector2 m_vLastMousePos;
        
		MashStringc m_inputContext;
        
		bool m_inputState;
		int32 m_contextChangeHandle;
        
		f32 m_rotationXDelta;
		f32 m_rotationYDelta;
		f32 m_moveXDelta;
		f32 m_moveYDelta;
		bool m_freeLookEnabled;
		void OnContextChange(const mash::sInputEvent &e);
		void OnLookHorizontal(const mash::sInputEvent &e);
		void OnLookVertical(const mash::sInputEvent &e);
		void OnMoveHorizontal(const mash::sInputEvent &e);
		void OnMoveVertical(const mash::sInputEvent &e);
		void OnFreeLookState(const mash::sInputEvent &e);
    public:
        CMashFreeCameraController(uint32 playerId,
								MashInputManager *pInputManager,
                                 const MashStringc &inputContext,
                                 f32 fRotationSpeed,
                                 f32 fLinearSpeed,
								 const MashFreeMovementController::sInputActionMovement *customActions = 0);
        
        ~CMashFreeCameraController();
        
        void OnNodeUpdate(MashSceneNode *sceneNode, f32 dt);
        
        f32 GetLinearSpeed()const;
        
        //! Gets the rotation speed.
        f32 GetRotationSpeed()const;
        
        //! Returns true if input is enabled, false if input is disabled.
        bool GetInputState()const;
        
        //! Set true to enable user input control, false to disable it.
        void SetInputState(bool enable);
        
        //! Sets the linear speed.
        void SetLinearSpeed(f32 s);
        
        //! Sets the roation speed.
        void SetRotationSpeed(f32 s);
	};
    
    inline void CMashFreeCameraController::SetLinearSpeed(f32 s)
    {
        m_fLinearSpeed = s;
    }
    
    inline void CMashFreeCameraController::SetRotationSpeed(f32 s)
    {
        m_fRotationSpeed = s;
    }
    
    inline f32 CMashFreeCameraController::GetLinearSpeed()const
    {
        return m_fLinearSpeed;
    }
    
    inline f32 CMashFreeCameraController::GetRotationSpeed()const
    {
        return m_fRotationSpeed;
    }
    
    inline bool CMashFreeCameraController::GetInputState()const
    {
        return m_inputState;
    }
}

#endif