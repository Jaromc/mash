//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_LIGHT_H_
#define _C_MASH_LIGHT_H_

#include "MashLight.h"
#include "MashRenderSurface.h"
#include "MashString.h"

namespace mash
{
	class MashSceneManager;

	class CMashLight : public MashLight
	{
	private:
		sMashLight m_light;

		mash::MashAABB m_boundingBox;

		bool m_bIsShadowsEnabled;
		mash::eLIGHTTYPE m_eType;
		bool m_bIsLightEnabled;
		eLIGHT_RENDERER_TYPE m_rendererType;

		void OnNodeTransformChange();
		void OnPassCullImpl(f32 interpolateAmount);
	public:
		CMashLight(MashSceneNode *pParent,
			mash::MashSceneManager *pManager,
				const MashStringc &sUserName,
				mash::eLIGHTTYPE type,
                  eLIGHT_RENDERER_TYPE lightRendererType,
                  bool isMainLight);

		virtual ~CMashLight();

		MashSceneNode* _CreateInstance(MashSceneNode *parent, const MashStringc &name);
		void SetLightType(mash::eLIGHTTYPE type);
		void SetDefaultDirectionalLightSettings(const mash::MashVector3 &dir);
		void SetDefaultSpotLightSettings(const mash::MashVector3 &dir);
		void SetDefaultPointLightSettings();

		void SetDiffuse(const sMashColour4 &sDiffuse);
		void SetSpecular(const sMashColour4 &sSpecular);
		void SetAmbient(const sMashColour4 &sAmbiant);

		void SetInnerCone(f32 angleInRadians);
		void SetOuterCone(f32 angleInRadians);
		void SetRange(f32 fRange);
		void SetFalloff(f32 fFalloff);
		void SetAttenuation(f32 fAttenuation0, f32 fAttenuation1, f32 fAttenuation2);
		mash::eLIGHTTYPE GetLightType()const;

		const sMashLight* GetLightData()const;

		const mash::MashAABB& GetLocalBoundingBox()const;
		uint32 GetNodeType()const;

		bool IsAABBInRange(const mash::MashAABB &test)const;

		bool IsShadowsEnabled()const;
		void SetShadowsEnabled(bool bEnable);

		bool IsLightEnabled()const;
		void SetEnableLight(bool bEnable);

		/*
			This should be called at load times only. Runtime use
			may cost performance.
		*/
        eLIGHT_RENDERER_TYPE GetLightRendererType()const;
		void SetLightRendererType(eLIGHT_RENDERER_TYPE rendererType, bool main = true);
		bool IsMainForwardRenderedLight()const;
		bool IsForwardRenderedLight()const;

		void _SetLightPosition(const mash::MashVector3 &worldPos, const mash::MashVector3 &viewPos);
	};
    
    inline eLIGHT_RENDERER_TYPE CMashLight::GetLightRendererType()const
    {
        return m_rendererType;
    }

	inline bool CMashLight::IsForwardRenderedLight()const
	{
		return (m_rendererType == aLIGHT_RENDERER_FORWARD)?true:false;
	}

	inline const mash::MashAABB& CMashLight::GetLocalBoundingBox()const
	{
		return m_boundingBox;
	}

    inline mash::eLIGHTTYPE CMashLight::GetLightType()const
	{
		return m_eType;
	}

	inline bool CMashLight::IsLightEnabled()const
	{
		return m_bIsLightEnabled;
	}

	inline void CMashLight::SetEnableLight(bool bEnable)
	{
		m_bIsLightEnabled = bEnable;
	}

	inline bool CMashLight::IsShadowsEnabled()const
	{
		return m_bIsShadowsEnabled;
	}

	inline uint32 CMashLight::GetNodeType()const
	{
		return aNODETYPE_LIGHT;
	}

	inline const sMashLight* CMashLight::GetLightData()const
	{
		return &m_light;
	}
}

#endif