//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashCamera.h"
#include "MashGeometryHelper.h"
#include "MashSceneManager.h"
#include "MashRay.h"
namespace mash
{
	CMashCamera::CMashCamera(MashSceneNode *pParent,
				mash::MashSceneManager *pManager,
				MashVideo *pRenderDevice,
				const MashStringc &sUserName):MashCamera(pParent, pManager, sUserName), m_pRenderDevice(pRenderDevice),
				m_fFOV(0.7853981625), m_fNear(1.0f), m_fFar(1000.0f), m_cameraUpdateFlags(aCAMERA_UPDATE_ALL),
				m_bIs2DEnabled(false),
				m_vUp(0.0f, 1.0f, 0.0f), m_vTarget(0.0f, 0.0f, 0.0f),
				m_frustumSphere(10, mash::MashVector3(0.0f, 0.0f, 0.0f)),
				m_boundingBox(mash::MashVector3(-0.5f, -0.5f, -0.5f), mash::MashVector3(0.5f, 0.5f, 0.5f)),
				m_isActiveCamera(false)
	{
		m_mView.Identity();
		m_mProjection.Identity();
		m_mViewProj.Identity();

		/*
			Make sure the camera is not on the point
			its looking at
		*/
		if (GetLocalTransformState().translation.LengthSq() < 0.1f)
			AddPosition(mash::MashVector3(0.0f, 0.0f, -1.0f));

		/*
			Update the abs position so that we can get
			the correct view matrix
		*/
		//MashSceneNode::Update(0.0f, true);
		SetAspect(true);
	}

	CMashCamera::~CMashCamera()
	{
		if (m_isActiveCamera)
		{
			m_sceneManager->SetActiveCamera(0);
		}
	}

	MashSceneNode* CMashCamera::_CreateInstance(MashSceneNode *parent, const MashStringc &name)
	{
		CMashCamera *pNewCamera = (CMashCamera*)m_sceneManager->AddCamera(parent, name);

		if (!pNewCamera)
			return 0;

		pNewCamera->InstanceMembers(this);

		pNewCamera->m_fFOV = m_fFOV;
		pNewCamera->m_fNear = m_fNear;
		pNewCamera->m_fFar = m_fFar;
		pNewCamera->m_bIs2DEnabled = m_bIs2DEnabled;
		pNewCamera->m_vUp = m_vUp;
		pNewCamera->m_vTarget = m_vTarget;
		pNewCamera->m_frustumSphere = m_frustumSphere;
		pNewCamera->m_boundingBox = m_boundingBox;
		pNewCamera->m_mView = m_mView;
		pNewCamera->m_mProjection = m_mProjection;
		pNewCamera->m_mViewProj = m_mViewProj;
		pNewCamera->m_bAutoAspect = m_bAutoAspect;

		memcpy(pNewCamera->m_viewFrustrum, m_viewFrustrum, sizeof(mash::MashPlane) * 6);

		return pNewCamera;
	}

	void CMashCamera::SetFOV(f32 fFov)
	{
        if (m_fFOV != fFov)
        {
            m_fFOV = fFov;

            m_cameraUpdateFlags |= aCAMERA_UPDATE_PROJECTION;
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_PROJECTION;
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_FRUSTUM;
        }
	}

	void CMashCamera::SetZNear(f32 zNear)
	{
        float oldVal = m_fNear;
        
		if (zNear < 0.1f)
			m_fNear = 0.1f;
		else
			m_fNear = zNear;

        if (oldVal != m_fNear)
        {
            m_cameraUpdateFlags |= aCAMERA_UPDATE_PROJECTION;
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_PROJECTION;
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_FRUSTUM;
        }
	}

	void CMashCamera::SetZFar(f32 zFar)
	{
        float oldVal = m_fFar;
        
		if (zFar <= m_fNear)
			m_fFar = m_fNear + 1.0f;
		else if (zFar < 0.1f)
			m_fFar = 0.1f;
		else
			m_fFar = zFar;
        
        if (m_fFar != oldVal)
        {
            m_cameraUpdateFlags |= aCAMERA_UPDATE_PROJECTION;
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_PROJECTION;
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_FRUSTUM;
        }
	}

	void CMashCamera::_SetActiveCamera(bool state)
	{
		m_isActiveCamera = state;
		//handles if a resoultion change has occured before being set. Should maybe set a flag for this instead of doing it each time its set
		_OnViewportChange();
	}

	void CMashCamera::SetAspect(bool bAuto, f32 fAspect)
	{
		f32 oldAspect = m_fAspect;

		m_bAutoAspect = bAuto;

		if (!bAuto && fAspect > 0.0f)
			m_fAspect = fAspect;
		else
		{
			int32 iWidth = m_pRenderDevice->GetViewport().width;
			int32 iHeight = m_pRenderDevice->GetViewport().height;

			m_fAspect = (f32)iWidth / (f32)iHeight;
		}

		if (oldAspect != m_fAspect)
        {
			m_cameraUpdateFlags |= aCAMERA_UPDATE_PROJECTION;
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_PROJECTION;
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_FRUSTUM;
        }
	}

	f32 CMashCamera::GetDistanceToBox(const mash::MashAABB &boundingBox)const
	{
		mash::MashVector3 vPos(GetWorldTransformState().translation);
		mash::MashRay ray(vPos, boundingBox.GetCenter() - vPos);

		f32 fDistance = 0.0f;
		collision::Ray_AABB(boundingBox, ray, fDistance);

		return fDistance;
	}

	bool CMashCamera::IsCulled(const mash::MashAABB &boundingBox)const
	{
        const MashPlane *frustumArray = GetViewFrustrum();
        
		mash::MashVector3 P, N;
		for(int32 i = 0; i < 6; ++i)
		{
			P = boundingBox.min;
			if (frustumArray[i].normal.x >= 0.0f)
				P.x = boundingBox.max.x;
			if (frustumArray[i].normal.y >= 0.0f)
				P.y = boundingBox.max.y;
			if (frustumArray[i].normal.z >= 0.0f)
				P.z = boundingBox.max.z;

			if ((P.Dot(frustumArray[i].normal) + frustumArray[i].dist) < 0.0f)
			{
				return true; //this means the box is behind the plane
			}
		}
		
		return false;
	}
    
    void CMashCamera::OnNodeTransformChange()
    {
        m_cameraUpdateFlags |= aCAMERA_UPDATE_ALL;
    }

	void CMashCamera::SetLookAt(const mash::MashVector3 &vTarget)
	{
		if (vTarget != m_vTarget)
        {
            m_vTarget = vTarget;
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW;
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_PROJECTION;
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_FRUSTUM;
        }
	}
    
    const mash::MashPlane* CMashCamera::GetViewFrustrum()const
	{
        if (m_cameraUpdateFlags & aCAMERA_UPDATE_VIEW_FRUSTUM)
        {
            CalculateViewFrustrum();
            m_cameraUpdateFlags &= ~aCAMERA_UPDATE_VIEW_FRUSTUM;
        }
        
		return m_viewFrustrum;
	}
    
    const mash::MashMatrix4& CMashCamera::GetView()const
	{
        if (m_cameraUpdateFlags & aCAMERA_UPDATE_VIEW)
        {
            CalculateView();
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_PROJECTION;
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_FRUSTUM;
            m_cameraUpdateFlags &= ~aCAMERA_UPDATE_VIEW;
        }
        
		return m_mView;
	}
    
	const mash::MashMatrix4& CMashCamera::GetProjection()const
	{
        if (m_cameraUpdateFlags & aCAMERA_UPDATE_PROJECTION)
        {
            CalculateProjection();
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_PROJECTION;
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_FRUSTUM;
            m_cameraUpdateFlags &= ~aCAMERA_UPDATE_PROJECTION;
        }
        
		return m_mProjection;
	}
    
	const mash::MashMatrix4& CMashCamera::GetViewProjection()const
	{
        if (m_cameraUpdateFlags & aCAMERA_UPDATE_VIEW_PROJECTION)
        {
            m_mViewProj = GetView() * GetProjection();
            m_cameraUpdateFlags |= aCAMERA_UPDATE_VIEW_FRUSTUM;
            m_cameraUpdateFlags &= ~aCAMERA_UPDATE_VIEW_PROJECTION;
        }
		return m_mViewProj;
	}

	void CMashCamera::CalculateProjection()const
	{
		if (!m_bIs2DEnabled)
			m_mProjection.CreatePerspectiveFOV(m_fFOV, m_fAspect, m_fNear, m_fFar);
		else
		{
			int32 iWidth = m_pRenderDevice->GetViewport().width;
			int32 iHeight = m_pRenderDevice->GetViewport().height;
			m_mProjection.CreateOrthographic(iWidth, iHeight, m_fNear, m_fFar);
		}
	}

	void CMashCamera::CalculateView()const
	{
		if (!m_bIs2DEnabled)
		{
			mash::MashVector3 vAbsPos = GetRenderTransformState().translation;
			
			mash::MashVector3 vTgTv = m_vTarget - vAbsPos;
			vTgTv.Normalize();

			mash::MashVector3 up(0.0f, 1.0f, 0.0f);
			mash::MashVector3 right =  up.Cross(vTgTv);
			right.Normalize();
			m_vUp = vTgTv.Cross(right);
			m_vUp.Normalize();//Is this normalize needed?

			mash::MashVector3 vUp = m_vUp;

			if (math::FloatEqualTo(vTgTv.Dot(vUp), 1.0f))
			{
				vUp.x += 0.5f;
			}

			m_mView.CreateCameraLookAt(vAbsPos, m_vTarget, vUp);
		}
		else
		{
			m_mView.Identity();

			int32 iWidth = m_pRenderDevice->GetViewport().width;
			int32 iHeight = m_pRenderDevice->GetViewport().height;

			m_mView.m22 = -1.0f;
			m_mView.m41 = -iWidth + iWidth * 0.5f;
			m_mView.m42 = iHeight - iHeight * 0.5f;
			m_mView.m43 = m_fNear + 0.1f;
		}
	}

	/*
		From http://www.flipcode.com/archives/Frustum_Culling.shtml
	*/
	/*void CMashCamera::CreateFrustumSphere()
	{
		// calculate the radius of the frustum sphere
		f32 fViewLen = m_fFar - m_fNear;
		// use some trig to find the height of the frustum at the far plane
		f32 fHeight = fViewLen * tan(m_fFOV * 0.5f);
		// with an aspect ratio of 1, the width will be the same
		f32 fWidth = fHeight;
		// halfway point between near/far planes starting at the origin and extending along the z axis
		mash::MashVector3 P(0.0f, 0.0f, m_fNear + fViewLen * 0.5f);
		// the calculate far corner of the frustum
		mash::MashVector3 Q(fWidth, fHeight, fViewLen);
		// the vector between P and Q
		mash::MashVector3 vDiff(P - Q);
		// the radius becomes the length of this vector
		m_frustumSphere.radius = vDiff.Length();
		// get the look vector of the camera from the view matrix
		mash::MashVector3 vLookVector = m_mView.GetForward();
		// calculate the center of the sphere
		m_frustumSphere.center = GetWorldTransformState().translation + (vLookVector * (fViewLen * 0.5f) + m_fNear);
	}*/

	void CMashCamera::CalculateViewFrustrum()const
	{
        //update the viewproj matrix if needed
        GetViewProjection();
        
		m_viewFrustrum[aVF_LEFT].normal.x = (m_mViewProj.m14 + m_mViewProj.m11);
		m_viewFrustrum[aVF_LEFT].normal.y = (m_mViewProj.m24 + m_mViewProj.m21);
		m_viewFrustrum[aVF_LEFT].normal.z = (m_mViewProj.m34 + m_mViewProj.m31);
		m_viewFrustrum[aVF_LEFT].dist = (m_mViewProj.m44 + m_mViewProj.m41);

		m_viewFrustrum[aVF_RIGHT].normal.x = (m_mViewProj.m14 - m_mViewProj.m11);
		m_viewFrustrum[aVF_RIGHT].normal.y = (m_mViewProj.m24 - m_mViewProj.m21);
		m_viewFrustrum[aVF_RIGHT].normal.z = (m_mViewProj.m34 - m_mViewProj.m31);
		m_viewFrustrum[aVF_RIGHT].dist = (m_mViewProj.m44 - m_mViewProj.m41);

		m_viewFrustrum[aVF_TOP].normal.x = (m_mViewProj.m14 - m_mViewProj.m12);
		m_viewFrustrum[aVF_TOP].normal.y = (m_mViewProj.m24 - m_mViewProj.m22);
		m_viewFrustrum[aVF_TOP].normal.z = (m_mViewProj.m34 - m_mViewProj.m32);
		m_viewFrustrum[aVF_TOP].dist = (m_mViewProj.m44 - m_mViewProj.m42);

		m_viewFrustrum[aVF_BOTTOM].normal.x = (m_mViewProj.m14 + m_mViewProj.m12);
		m_viewFrustrum[aVF_BOTTOM].normal.y = (m_mViewProj.m24 + m_mViewProj.m22);
		m_viewFrustrum[aVF_BOTTOM].normal.z = (m_mViewProj.m34 + m_mViewProj.m32);
		m_viewFrustrum[aVF_BOTTOM].dist = (m_mViewProj.m44 + m_mViewProj.m42);

		m_viewFrustrum[aVF_NEAR].normal.x = m_mViewProj.m13;
		m_viewFrustrum[aVF_NEAR].normal.y = m_mViewProj.m23;
		m_viewFrustrum[aVF_NEAR].normal.z = m_mViewProj.m33;
		m_viewFrustrum[aVF_NEAR].dist = m_mViewProj.m43;

		m_viewFrustrum[aVF_FAR].normal.x = (m_mViewProj.m14 - m_mViewProj.m13);
		m_viewFrustrum[aVF_FAR].normal.y = (m_mViewProj.m24 - m_mViewProj.m23);
		m_viewFrustrum[aVF_FAR].normal.z = (m_mViewProj.m34 - m_mViewProj.m33);
		m_viewFrustrum[aVF_FAR].dist = (m_mViewProj.m44 - m_mViewProj.m43);

		//normalize the normals
		for(int32 i = 0; i < 6; ++i)
		{
			f32 fOneOverLength = 1.0f / sqrt(m_viewFrustrum[i].normal.x * m_viewFrustrum[i].normal.x +
				m_viewFrustrum[i].normal.y * m_viewFrustrum[i].normal.y + 
				m_viewFrustrum[i].normal.z * m_viewFrustrum[i].normal.z);

			m_viewFrustrum[i].normal *= fOneOverLength;
			m_viewFrustrum[i].dist *= fOneOverLength;
		}
	}

	void CMashCamera::Enable2D(bool bEnable)
	{
        if (m_bIs2DEnabled != bEnable)
        {
            m_bIs2DEnabled = bEnable;

            m_cameraUpdateFlags |= aCAMERA_UPDATE_ALL;
        }
	}

	void CMashCamera::OnPassCullImpl(f32 interpolateAmount)
	{
        mash::MashVector3 targetVector(0.0f, 0.0f, 1.0f);
        targetVector = GetRenderTransformState().orientation.TransformVector(targetVector);
        SetLookAt(targetVector + GetRenderTransformState().translation);
    }

	void CMashCamera::_OnViewportChange()
	{
		if (GetAutoAspectEnabled())
			SetAspect(true);

        m_cameraUpdateFlags |= aCAMERA_UPDATE_ALL;
	}

	void CMashCamera::TransformWorldToScreenPosition(const mash::MashVector2 &viewportWidthHeight,
			const mash::MashVector3 &worldPos,
			mash::MashVector2 &screenPosOut)const
	{
		MashVector4 screenPos4D = worldPos;

		screenPos4D = GetViewProjection().TransformVector(screenPos4D);
		screenPos4D.x /= screenPos4D.w;
		screenPos4D.y /= screenPos4D.w;

		screenPosOut.x = screenPos4D.x;
		screenPosOut.y = screenPos4D.y;

		screenPosOut.x = viewportWidthHeight.x * (0.5f * screenPosOut.x + 0.5f);
		screenPosOut.y = viewportWidthHeight.y * (-0.5f * screenPosOut.y + 0.5f);
	}

	void CMashCamera::TransformScreenToWorldPosition(const mash::MashVector2 &viewportWidthHeight,
				const mash::MashVector2 &screenPos2D,
				mash::MashVector3 &vOriginOut,
				mash::MashVector3 &vDirOut)const
	{
		mash::MashVector3 vC;

		vC.x = (((screenPos2D.x * 2.0f) / viewportWidthHeight.x) - 1.0f) / GetProjection().m11;
		vC.y = -(((screenPos2D.y * 2.0f) / viewportWidthHeight.y) - 1.0f) / GetProjection().m22;
		vC.z = 1.0f;

		mash::MashMatrix4 mMat = GetView();
		mMat.Invert();

		mMat.TransformRotation(vC, vDirOut);

		vOriginOut.x = mMat.m41;
		vOriginOut.y = mMat.m42;
		vOriginOut.z = mMat.m43;	
	}
}