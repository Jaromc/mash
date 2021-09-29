//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_CAMERA_H_
#define _MASH_CAMERA_H_

#include "MashSceneNode.h"

namespace mash
{
	class MashVector2;

	class MashCamera : public MashSceneNode
	{
	public:
		MashCamera(MashSceneNode *parent,
			mash::MashSceneManager *manager,
				const MashStringc &userName):MashSceneNode(parent, manager, userName){}

		virtual ~MashCamera(){}

        //! Camera FOV.
		/*!
            \param fov Camera fov.
         */
		virtual void SetFOV(f32 fov) = 0;
        
        //! Near frustum.
		/*!
            \param near Near frustum.
         */
		virtual void SetZNear(f32 near) = 0;
        
        //! Far frustum.
		/*!
            \param far Far frustum.
        */
		virtual void SetZFar(f32 far) = 0;
        
        //! Camera aspect ratio.
		/*!
            Auto aspect can be selected to calculate the aspect using
            the current scene dimentions.
         
            \param auto Set to true for auto aspect.
            \param aspect Aspect for the camera if auto aspect is not used.
        */
		virtual void SetAspect(bool autoAspect, f32 aspect = 0.0f) = 0;

        //! Camera FOV.
		/*!
            \return Camera fov.
        */
		virtual f32 GetFOV()const = 0;
        
        //! Aspect ratio.
		/*!
            \return Camera aspect ratio.
        */
		virtual f32 GetAspect()const = 0;
        
        //! Near frustum.
		/*!
            \return Near frustum.
        */
		virtual f32 GetNear()const = 0;
        
        //! Far frustum.
		/*!
            \return Far frustum.
        */
		virtual f32 GetFar()const = 0;
        
        //! Current target vector (if set).
		/*!
            \return Target vector.
        */
		virtual const MashVector3& GetTarget()const = 0;
        
        //! View matrix up vector.
		/*!
            \return Up vector.
        */
		virtual const MashVector3& GetUp()const = 0;
        
        //! View matrix.
		/*!
            \return View matrix.
        */
		virtual const MashMatrix4& GetView()const = 0;
        
        //! Projection matrix.
		/*!
            \return Projection matrix.
        */
		virtual const MashMatrix4& GetProjection()const = 0;
        
        //! View prjection matrix.
		/*!
            \return View projection matrix.
        */
		virtual const MashMatrix4& GetViewProjection()const = 0;
        
        //! Current view frustum planes.
		/*!
            The array contains 6 elements for each side of the frustum.
            The array must not be deleted.
         
            \return View frustum.
         */
		virtual const MashPlane* GetViewFrustrum()const = 0;

        //! Transforms this camera into 2D or 3D space (orthographic or perspective).
		/*!
            \parm enable Set to true or false depending on which space you want to use.
         */
		virtual void Enable2D(bool enable) = 0;

        //! Camera Type.
		/*!
            \return Camera type.
        */
		virtual eCAMERA_TYPE GetCameraType()const = 0;

		//! Transforms a 2D position in screen space into a 3D view ray relative to this camera.
		/*!
			viewportWidthHeight must be passed in cause the active viewport may not be
			the viewport the scene was rendered with.

			\param viewportWidthHeight Viewport width and height.
			\param screenPos2D Screen position to transform.
            \return originOut Ray origin.
            \return dirOut Ray direction.
        */
		virtual void TransformScreenToWorldPosition(const MashVector2 &viewportWidthHeight,
			const MashVector2 &screenPos2D,
			MashVector3 &originOut,
			MashVector3 &dirOut)const = 0;

		//! Transforms a 3D position in world space into 2D screen space relative to this camera.
		/*!
			\param viewportWidthHeight Viewport width and height.
			\param worldPos World space position to transform.
			\param screenPosOut output.
		*/
		virtual void TransformWorldToScreenPosition(const MashVector2 &viewportWidthHeight,
			const MashVector3 &worldPos,
			MashVector2 &screenPosOut)const = 0;

        //! Camera frustum culling test.
		/*!
            Performs a cull test based on the current view frustum of this camera and
            world space bounding box passed in.
         
            \param boundingBox World space AABB to test.
            \return True if the bounding box is completly outside the view frustum. False otherwise.
        */
		virtual bool IsCulled(const MashAABB &boundingBox)const = 0;
        
        //! Returns the distance from this camera to the AABB.
		/*!
            The distance returned is from this cameras origin to the closest
            point on the outside of the AABB (not the origin).
         
            \param boundingBox AABB to test the distance.
            \return Distance from this camera to the AABB.
        */
		virtual f32 GetDistanceToBox(const MashAABB &boundingBox)const = 0;

		//! Is orthographic mode enabled.
		/*!
			\return True if the camera is using an orthographic matrix. False for perspective.
		*/
		virtual bool Get2DEnabled()const = 0;

		//! Is auto aspect enabled.
		/*!
			\return True if auto aspect is enabled. False otherwise.
		*/
		virtual bool GetAutoAspectEnabled()const = 0;
        
        //! Returns true if this is the current active camera used for rendering.
        virtual bool IsActiveCamera()const = 0;		

        //! Internal use only. Called on a viewport change to update the aspect ratio.
		virtual void _OnViewportChange() = 0;
        
        //! Sets this camera to the active camera.
        /*!
            Internal use only.
         
            \param state Active camera state.
        */
        virtual void _SetActiveCamera(bool state) = 0;
	};
}

#endif