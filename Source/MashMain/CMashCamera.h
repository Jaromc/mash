//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_CAMERA_H_
#define _C_MASH_CAMERA_H_

#include "MashCamera.h"
#include "MashVideo.h"
#include "MashPlane.h"
#include "MashVector2.h"
#include "MashPlane.h"
#include "MashSphere.h"
namespace mash
{
	class MashSceneManager;

	class CMashCamera : public MashCamera
	{
    protected:
        enum eCAMERA_UPDATE_FLAGS
        {
            aCAMERA_UPDATE_PROJECTION = 1,
            aCAMERA_UPDATE_VIEW = 2,
            aCAMERA_UPDATE_VIEW_PROJECTION = 4,
            aCAMERA_UPDATE_VIEW_FRUSTUM = 8,
            aCAMERA_UPDATE_ALL = (aCAMERA_UPDATE_PROJECTION | aCAMERA_UPDATE_VIEW | aCAMERA_UPDATE_VIEW_PROJECTION | aCAMERA_UPDATE_VIEW_FRUSTUM)
        };
        
        mutable uint32 m_cameraUpdateFlags;
	private:
        
		MashVideo *m_pRenderDevice;

		mutable mash::MashVector3 m_vUp;
		mash::MashVector3 m_vTarget;

		f32 m_fFOV;
		f32 m_fAspect;
		f32 m_fNear;
		f32 m_fFar;

		mutable mash::MashPlane m_viewFrustrum[6];

		mash::MashSphere m_frustumSphere;

		mutable mash::MashMatrix4 m_mView;
		mutable mash::MashMatrix4 m_mProjection;
		mutable mash::MashMatrix4 m_mViewProj;

		bool m_bIs2DEnabled;
		bool m_bAutoAspect;

		bool m_isActiveCamera;

		virtual void CalculateProjection()const;
		virtual void CalculateView()const;
		virtual void CalculateViewFrustrum()const;

		mash::MashAABB m_boundingBox;

        void OnNodeTransformChange();
		void OnPassCullImpl(f32 interpolateAmount);
	public:
		CMashCamera(MashSceneNode *pParent,
			mash::MashSceneManager *pManager,
				MashVideo *pRenderDevice,
				const MashStringc &sUserName);

		virtual ~CMashCamera();

		MashSceneNode* _CreateInstance(MashSceneNode *parent, const MashStringc &name);

		void SetFOV(f32 fFov);
		void SetZNear(f32 fNear);
		void SetZFar(f32 fFar);
		void SetAspect(bool bAuto, f32 fAspect = 0.0f);

		f32 GetFOV()const;
		f32 GetAspect()const;
		f32 GetNear()const;
		f32 GetFar()const;

		bool Get2DEnabled()const;
		bool GetAutoAspectEnabled()const;

		const mash::MashVector3& GetUp()const;
		const mash::MashVector3& GetTarget()const;

		const mash::MashMatrix4& GetView()const;
		const mash::MashMatrix4& GetProjection()const;
		const mash::MashMatrix4& GetViewProjection()const;
		const mash::MashPlane* GetViewFrustrum()const;

		void SetLookAt(const mash::MashVector3 &vTarget);
		void SetTargetNode(mash::MashSceneNode *target);

		void Enable2D(bool bEnable);
		const mash::MashAABB& GetLocalBoundingBox()const;

		void TransformWorldToScreenPosition(const mash::MashVector2 &viewportWidthHeight,
			const mash::MashVector3 &worldPos,
			mash::MashVector2 &screenPosOut)const;

		void TransformScreenToWorldPosition(const mash::MashVector2 &viewportWidthHeight,
		const mash::MashVector2 &screenPos2D,
		mash::MashVector3 &vOriginOut,
		mash::MashVector3 &vDirOut)const;

		uint32 GetNodeType()const;

		bool IsCulled(const mash::MashAABB &boundingBox)const;
		f32 GetDistanceToBox(const mash::MashAABB &boundingBox)const;

		eCAMERA_TYPE GetCameraType()const;

		void _SetActiveCamera(bool state);
		bool IsActiveCamera()const;
		void _OnViewportChange();
	};

	inline bool CMashCamera::IsActiveCamera()const
	{
		return m_isActiveCamera;
	}

	inline bool CMashCamera::Get2DEnabled()const
	{
		return m_bIs2DEnabled;
	}

	inline bool CMashCamera::GetAutoAspectEnabled()const
	{
		return m_bAutoAspect;	
	}

	inline eCAMERA_TYPE CMashCamera::GetCameraType()const
	{
		return aCAMERA_TYPE_FIXED;
	}

	inline const mash::MashAABB& CMashCamera::GetLocalBoundingBox()const
	{
		return m_boundingBox;
	}

	inline uint32 CMashCamera::GetNodeType()const
	{
		return aNODETYPE_CAMERA;
	}

	inline const mash::MashVector3& CMashCamera::GetUp()const
	{
		return m_vUp;
	}

	inline const mash::MashVector3& CMashCamera::GetTarget()const
	{
		return m_vTarget;
	}

	inline f32 CMashCamera::GetFOV()const
	{
		return m_fFOV;
	}

	inline f32 CMashCamera::GetAspect()const
	{
		return m_fAspect;
	}

	inline f32 CMashCamera::GetNear()const
	{
		return m_fNear;
	}

	inline f32 CMashCamera::GetFar()const
	{
		return m_fFar;
	}
}

#endif