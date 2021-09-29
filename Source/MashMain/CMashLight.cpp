//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashLight.h"
#include "MashGeometryHelper.h"
#include "MashSphere.h"
#include "MashCamera.h"
#include "MashSceneManager.h"
namespace mash
{
	CMashLight::CMashLight(MashSceneNode *pParent,
			mash::MashSceneManager *pManager,
				const MashStringc &sUserName,
				mash::eLIGHTTYPE type,
                eLIGHT_RENDERER_TYPE lightRendererType,
                bool isMainLight):MashLight(pParent, pManager, sUserName),
				m_boundingBox(mash::MashVector3(-0.5f, -0.5f, -0.5f), mash::MashVector3(0.5f, 0.5f, 0.5f)),
				m_bIsLightEnabled(true), m_bIsShadowsEnabled(false), m_eType(aLIGHT_TYPE_COUNT), m_rendererType(aLIGHT_RENDERER_NOT_SET)
	{
        memset(&m_light, 0, sizeof(m_light));

		//type check
		if (type == mash::aLIGHT_TYPE_COUNT)
			type = mash::aLIGHT_DIRECTIONAL;

		SetLightType(type);

		/*
			Create a default light
		*/
		switch(type)
		{
		case mash::aLIGHT_DIRECTIONAL:
			{
				mash::MashVector3 defaultDir(0.0f, 0.0f, 1.0f);
				SetDefaultDirectionalLightSettings(defaultDir);
				break;
			}
		case mash::aLIGHT_SPOT:
			{
				mash::MashVector3 defaultDir(0.0f, 0.0f, 1.0f);
				SetDefaultSpotLightSettings(defaultDir);
				break;
			}
		case mash::aLIGHT_POINT:
			{
				SetDefaultPointLightSettings();
				break;
			}
		}
        
        //type check
        if (lightRendererType == aLIGHT_RENDERER_NOT_SET)
            lightRendererType = aLIGHT_RENDERER_FORWARD;
        
        SetLightRendererType(lightRendererType, isMainLight);
	}

	CMashLight::~CMashLight()
	{
		SetLightRendererType(aLIGHT_RENDERER_NOT_SET, false);
	}

	MashSceneNode* CMashLight::_CreateInstance(MashSceneNode *parent, const MashStringc &name)
	{
		CMashLight *pNewLight = (CMashLight*)m_sceneManager->AddLight(parent, name, m_eType, m_rendererType, IsMainForwardRenderedLight());

		if (!pNewLight)
			return 0;

		pNewLight->InstanceMembers(this);

		pNewLight->m_boundingBox = m_boundingBox;
		pNewLight->m_bIsLightEnabled = m_bIsLightEnabled;
		pNewLight->m_bIsShadowsEnabled = m_bIsShadowsEnabled;
		memcpy(&pNewLight->m_light, &m_light, sizeof(sMashLight));

		return pNewLight;
	}

	void CMashLight::SetLightType(mash::eLIGHTTYPE type)
	{
		if ((m_eType != type) && (type != aLIGHT_TYPE_COUNT))
		{
            if (m_eType != aLIGHT_TYPE_COUNT)
            {
                bool reenableShadows = false;
                //if the light type has changed then we need to disable then re-enable
                //shadows so that the scene manager can keep track of whats going on.
                if (m_bIsShadowsEnabled)
                {
                    SetShadowsEnabled(false);
                    reenableShadows = true;
                }
                
                m_eType = type;
                
                m_sceneManager->_OnLightTypeChange(this);
                
                if (reenableShadows)
                {
                    SetShadowsEnabled(true);
                }
            }
            else//only happens on init
            {
                m_eType = type;
            }
		}
	}

	void CMashLight::SetDefaultDirectionalLightSettings(const mash::MashVector3 &dir)
	{
		const mash::sMashColour4 defautlColour(1.0f, 1.0f, 1.0f, 1.0f);
		SetAmbient(mash::sMashColour4(0.2f, 0.2f, 0.2f, 1.0f));
		SetDiffuse(defautlColour);
		SetSpecular(defautlColour);
		SetLookAtDirection(dir, true);
		SetRange(mash::math::MaxFloat());
	}

	void CMashLight::SetDefaultSpotLightSettings(const mash::MashVector3 &dir)
	{
		const mash::sMashColour4 defautlColour(1.0f, 1.0f, 1.0f, 1.0f);
		SetAmbient(mash::sMashColour4(0.2f, 0.2f, 0.2f, 1.0f));
		SetDiffuse(defautlColour);
		SetSpecular(defautlColour);
		SetLookAtDirection(dir, true);
		m_light.position = GetWorldTransformState().translation;
		SetRange(1000.0f);
		SetAttenuation(0.0f, 0.125f, 0.0);
		SetOuterCone(math::DegsToRads(60.0f));
		SetInnerCone(math::DegsToRads(20.0f));
		SetFalloff(1.0f);
	}

	void CMashLight::SetDefaultPointLightSettings()
	{
		m_light.position = GetWorldTransformState().translation;
		const mash::sMashColour4 defautlColour(1.0f, 1.0f, 1.0f, 1.0f);
		SetAmbient(mash::sMashColour4(0.2f, 0.2f, 0.2f, 1.0f));
		SetDiffuse(defautlColour);
		SetSpecular(defautlColour);
		SetRange(1000.0f);
		SetAttenuation(0.0f, 0.125f, 0.0);
	}

	bool CMashLight::IsAABBInRange(const mash::MashAABB &test)const
	{
		if (m_eType == aLIGHT_DIRECTIONAL)
			return true;

		mash::MashSphere lightRadius(m_light.range, m_light.position);
		if (mash::collision::AABB_Sphere(test, lightRadius))
			return true;

		return false;
	}

	void CMashLight::SetDiffuse(const mash::sMashColour4 &diffuse)
	{
		m_light.diffuse = diffuse;
	}

	void CMashLight::SetSpecular(const mash::sMashColour4 &specular)
	{
		m_light.specular = specular;
	}

	void CMashLight::SetAmbient(const mash::sMashColour4 &ambiant)
	{
		m_light.ambient = ambiant;
	}

	void CMashLight::SetRange(f32 fRange)
	{
		/*
			Directional lights extend forever
		*/
		if (m_eType == mash::aLIGHT_DIRECTIONAL)
			fRange = mash::math::MaxFloat();

		m_light.range = fRange;
	}

	void CMashLight::SetFalloff(f32 fFalloff)
	{
		m_light.falloff = fFalloff;
	}

	void CMashLight::SetAttenuation(f32 fAttenuation0, f32 fAttenuation1, f32 fAttenuation2)
	{
		m_light.atten.x = fAttenuation0;
		m_light.atten.y = fAttenuation1;
		m_light.atten.z = fAttenuation2;
	}

	void CMashLight::SetInnerCone(f32 angleInRadians)
	{
		m_light.innerCone = cos(angleInRadians);
	}

	void CMashLight::SetOuterCone(f32 angleInRadians)
	{
		m_light.outerCone = cos(angleInRadians);
	}

	void CMashLight::SetShadowsEnabled(bool bEnable)
	{
		/*
			Make sure a shadow caster is created for this light type
		*/

		if (!m_bIsShadowsEnabled && bEnable)
			m_sceneManager->_OnShadowsEnabled(this);
		else if (m_bIsShadowsEnabled && !bEnable)
			m_sceneManager->_OnShadowsDisabled(this);

		m_bIsShadowsEnabled = bEnable;
	}

	bool CMashLight::IsMainForwardRenderedLight()const
	{
		MashLight *firstLight = m_sceneManager->GetFirstForwardRenderedLight();
		if (firstLight && (firstLight == this))
			return true;

		return false;
	}

	void CMashLight::SetLightRendererType(eLIGHT_RENDERER_TYPE newRendererType, bool main)
	{        
		if ((m_rendererType != newRendererType) || (main != IsMainForwardRenderedLight()))
        {
            bool reenableShadows = false;
            
            //if the light type has changed then we need to disable then re-enable
            //shadows so that the scene manager can keep track of whats going on.
            if ((m_rendererType != newRendererType) && m_bIsShadowsEnabled)
            {
                SetShadowsEnabled(false);
                
                //if the light is not changing to a defined type then don't re-enable 
                if (newRendererType != aLIGHT_RENDERER_NOT_SET)
                    reenableShadows = true;
            }
           
            //if we are changing to a forward renderer then add it to the scene manager
            if (newRendererType == aLIGHT_RENDERER_FORWARD)
            {
                m_sceneManager->_AddForwardRenderedLight(this, main);
            }
            //if the old render type was forward then remove it from the scene manager
            else if (m_rendererType == aLIGHT_RENDERER_FORWARD)
            {
                m_sceneManager->_RemoveForwardRenderedLight(this);
            }
         
            m_rendererType = newRendererType;
            
            //enable shadows with new light type
            if (reenableShadows)
                SetShadowsEnabled(true);
        }
	}

	void CMashLight::_SetLightPosition(const mash::MashVector3 &worldPos, const mash::MashVector3 &viewPos)
	{
		m_light.position = worldPos;
		m_light.viewSpacePosition = viewPos;
	}

	void CMashLight::OnNodeTransformChange()
	{
	}

	void CMashLight::OnPassCullImpl(f32 interpolateAmount)
	{
		/*
			Note : Directional lights may have their position updated within
			the shadow caster, if shadowing is enabled.
		*/
		m_light.position = GetRenderTransformState().translation;

		const mash::MashMatrix4 *viewMatrix = &m_sceneManager->GetActiveCamera()->GetView();

		/*
			Directional light positions are only needed for shadows. If shadows are
			enabled then the caster will update this.
		*/
        m_light.viewSpacePosition = viewMatrix->TransformVector(m_light.position);

		if (m_eType != aLIGHT_POINT)
		{
			mash::MashVector3 targetVector(0.0f, 0.0f, 1.0f);
			m_light.direction = GetRenderTransformState().orientation.TransformVector(targetVector);
			m_light.direction.Normalize();
		}

        //invert transposed for direction minus translation
		MashMatrix4 view = *viewMatrix;
		view.Invert();
		view.Transpose();
		m_light.viewSpaceDirection = view.TransformRotation(m_light.direction);
		m_light.viewSpaceDirection.Normalize();
        
        m_sceneManager->_AddLightToCurrentRenderScene(this);
	}
}