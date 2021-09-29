//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_POINT_SHADOW_CASTER_H_
#define _C_MASH_POINT_SHADOW_CASTER_H_

#include "MashPointShadowCaster.h"
#include "MashAutoEffectParameter.h"
#include "MashRenderSurface.h"
#include "MashMatrix4.h"
#include "MashVector4.h"
#include "MashEffect.h"
#include "MashMaterial.h"

namespace mash
{
	class CMashPointShadowCaster : public MashPointShadowCaster
	{
		class ParamLightPosRange : public MashAutoEffectParameter
		{
			static int8 *m_autoName;
		public:
			ParamLightPosRange():MashAutoEffectParameter(){}
			~ParamLightPosRange(){}

			void OnSet(const MashRenderInfo *renderInfo, 
				MashEffect *effect, 
				MashEffectParamHandle *parameter,
				uint32 index = 0)
			{
				CMashPointShadowCaster *caster = (CMashPointShadowCaster*)renderInfo->GetShadowCaster();
				effect->SetVector4(parameter, &caster->m_worldPosAndRange);
			}

			const int8* GetParameterName()const{return m_autoName;}
		};

		class ParamLightViewProjection : public MashAutoEffectParameter
		{
			static int8 *m_autoName;
		public:
			ParamLightViewProjection():MashAutoEffectParameter(){}
			~ParamLightViewProjection(){}

			void OnSet(const MashRenderInfo *renderInfo, 
				MashEffect *effect, 
				MashEffectParamHandle *parameter,
				uint32 index = 0)
			{
				CMashPointShadowCaster *caster = (CMashPointShadowCaster*)renderInfo->GetShadowCaster();
				effect->SetMatrix(parameter, &caster->m_lightViewProjection);
			}

			const int8* GetParameterName()const{return m_autoName;}
		};

	protected:
		MashVideo *m_renderer;
		f32 m_bias;
		int32 m_textureSize;
		eSHADOW_MAP_FORMAT m_textureFormat;
		int32 m_blendState;
		int32 m_rasterizerState;

		MashRenderSurface *m_renderTarget;
		MashTextureState *m_textureState;

		MashMatrix4 m_lightViewProjection;
		MashMatrix4 m_lightProjection;
		MashVector4 m_worldPosAndRange;
		static bool m_isCasterTypeInitialised;

		//for esm
		eCASTER_TYPE m_casterType;
		f32 m_emsDarkeningFactor;
		bool m_useBackface;

		uint32 m_activePass;
		bool m_rebuidRenderTarget;
		void OnPixelReceiverBuild(MashArray<sEffectMacro> &macros);
		eMASH_STATUS OnRebuildRenderTarget();
		//eMASH_STATUS RebuildBlurMaterial();
		void OnLoad();
	public:
		CMashPointShadowCaster(MashVideo *renderer, eCASTER_TYPE casterType);
		~CMashPointShadowCaster();

		void Serialise(MashXMLWriter *writer);
		void Deserialise(MashXMLReader *reader);
		int32 GetShadowCasterType()const;

		void SetBias(f32 bias);
		void SetTextureFormat(eSHADOW_MAP_FORMAT format);
		void SetTextureSize(int32 shadowMapSize);

		f32 GetBias()const;
		eSHADOW_MAP_FORMAT GetTextureFormat()const;
		int32 GetTextureSize()const;
		uint32 GetNumPasses()const;

		virtual eMASH_STATUS OnPassSetup(MashLight *light, const MashCamera *camera, const MashAABB &sceneAABB);
		eMASH_STATUS OnPass(uint32 pass, MashLight *light, const MashCamera *camera, const MashAABB &sceneAABB);
		eLIGHTTYPE GetShadowType()const;
		void OnUnload(){}//not used
		void OnPassEnd(){};
		eMASH_STATUS OnInitialise();
		MashTexture* GetShadowMap(MashTextureState const **textureStateOut, uint32 textureIndex);

		void SetESMDarkeningFactor(f32 darkening);
		f32 GetESMDarkeningFactor()const;

		void UseBackfaceGeometry(bool enable);
		bool GetBackfaceRenderingEnabled()const;
	};

	inline bool CMashPointShadowCaster::GetBackfaceRenderingEnabled()const
	{
		return m_useBackface;
	}

	inline f32 CMashPointShadowCaster::GetESMDarkeningFactor()const
	{
		return m_emsDarkeningFactor;
	}

	inline uint32 CMashPointShadowCaster::GetNumPasses()const
	{
		return 6;
	}

	inline eLIGHTTYPE CMashPointShadowCaster::GetShadowType()const
	{
		return aLIGHT_POINT;
	}

	inline f32 CMashPointShadowCaster::GetBias()const
	{
		return m_bias;
	}

	inline eSHADOW_MAP_FORMAT CMashPointShadowCaster::GetTextureFormat()const
	{
		return m_textureFormat;
	}

	inline int32 CMashPointShadowCaster::GetTextureSize()const
	{
		return m_textureSize;
	}
}

#endif