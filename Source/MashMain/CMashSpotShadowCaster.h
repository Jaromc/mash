//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_SPOT_SHADOW_CASTER_H_
#define _C_MASH_SPOT_SHADOW_CASTER_H_

#include "MashSpotShadowCaster.h"
#include "MashAutoEffectParameter.h"
#include "MashRenderSurface.h"
#include "MashMatrix4.h"
#include "MashVector4.h"
#include "MashEffect.h"
#include "MashArray.h"

namespace mash
{
	class CMashSpotShadowCaster : public MashSpotShadowCaster
	{
	private:
		class ParamSpotProjection : public MashAutoEffectParameter
		{
			static int8 *m_autoName;
		public:
			ParamSpotProjection():MashAutoEffectParameter(){}
			~ParamSpotProjection(){}

			void OnSet(const MashRenderInfo *renderInfo, 
				MashEffect *effect, 
				MashEffectParamHandle *parameter,
				uint32 index = 0)
			{
				CMashSpotShadowCaster *caster = (CMashSpotShadowCaster*)renderInfo->GetShadowCaster();
				effect->SetMatrix(parameter, &caster->m_lightProjection);
			}

			const int8* GetParameterName()const{return m_autoName;}
		};

		class ParamCamViewToLightView : public MashAutoEffectParameter
		{
			static int8 *m_autoName;
		public:
			ParamCamViewToLightView():MashAutoEffectParameter(){}
			~ParamCamViewToLightView(){}

			void OnSet(const MashRenderInfo *renderInfo, 
				MashEffect *effect, 
				MashEffectParamHandle *parameter,
				uint32 index = 0)
			{
				CMashSpotShadowCaster *caster = (CMashSpotShadowCaster*)renderInfo->GetShadowCaster();
				effect->SetMatrix(parameter, &caster->m_camViewToLightView);
			}

			const int8* GetParameterName()const{return m_autoName;}
		};

		class ParamSpotLightRange: public MashAutoEffectParameter
		{
			static int8 *m_autoName;
		public:
			ParamSpotLightRange():MashAutoEffectParameter(){}
			~ParamSpotLightRange(){}

			void OnSet(const MashRenderInfo *renderInfo, 
				MashEffect *effect, 
				MashEffectParamHandle *parameter,
				uint32 index = 0)
			{
				CMashSpotShadowCaster *caster = (CMashSpotShadowCaster*)renderInfo->GetShadowCaster();
				effect->SetVector4(parameter, &caster->m_currentLightInvRange);
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

		eSHADOW_SAMPLES m_samples;
		f32 m_sampleSize;
		MashRenderSurface *m_renderTarget;
		MashTextureState *m_textureState;
		MashVector4 m_currentLightInvRange;

		MashMatrix4 m_camViewToLightView;
		MashMatrix4 m_lightProjection;
		MashVector4 m_worldPosition;
		static bool m_isCasterTypeInitialised;

		//for esm
		int32 m_casterType;
		f32 m_emsDarkeningFactor;
		//bool m_rebuildESMMaterial;
		bool m_useBackface;

		uint32 m_activePass;
		bool m_rebuidRenderTarget;
		void OnPixelReceiverBuild(MashArray<sEffectMacro> &macros);
		eMASH_STATUS RebuildRenderTarget();
		void OnLoad();
	public:
		CMashSpotShadowCaster(MashVideo *renderer, eCASTER_TYPE casterType);
		~CMashSpotShadowCaster();

		void Serialise(MashXMLWriter *writer);
		void Deserialise(MashXMLReader *reader);
		int32 GetShadowCasterType()const;

		void SetBias(f32 bias);
		void SetTextureFormat(eSHADOW_MAP_FORMAT format);
		void SetTextureSize(int32 shadowMapSize);
		void SetSamples(eSHADOW_SAMPLES samples);
		void SetSampleSize(f32 size);
		eSHADOW_SAMPLES GetSampleCount()const;
		f32 GetSampleSize()const;

		f32 GetBias()const;
		eSHADOW_MAP_FORMAT GetTextureFormat()const;
		int32 GetTextureSize()const;
		uint32 GetNumPasses()const;

		virtual eMASH_STATUS OnPassSetup(MashLight *light, const MashCamera *camera, const MashAABB &sceneAABB);
		eMASH_STATUS OnPass(uint32 pass, MashLight *light, const MashCamera *camera, const MashAABB &sceneAABB);
		eLIGHTTYPE GetShadowType()const;
		void OnUnload(){}//not used
		void OnPassEnd(){}
		eMASH_STATUS OnInitialise();
		MashTexture* GetShadowMap(MashTextureState const **textureStateOut, uint32 textureIndex);

		void SetESMDarkeningFactor(f32 darkening);
		f32 GetESMDarkeningFactor()const;

		void UseBackfaceGeometry(bool enable);
		bool GetBackfaceRenderingEnabled()const;
	};

	inline bool CMashSpotShadowCaster::GetBackfaceRenderingEnabled()const
	{
		return m_useBackface;
	}

	inline f32 CMashSpotShadowCaster::GetESMDarkeningFactor()const
	{
		return m_emsDarkeningFactor;
	}

	inline eSHADOW_SAMPLES CMashSpotShadowCaster::GetSampleCount()const
	{
		return m_samples;
	}

	inline f32 CMashSpotShadowCaster::GetSampleSize()const
	{
		return m_sampleSize;
	}

	inline uint32 CMashSpotShadowCaster::GetNumPasses()const
	{
		return 1;
	}

	inline eLIGHTTYPE CMashSpotShadowCaster::GetShadowType()const
	{
		return aLIGHT_SPOT;
	}

	inline f32 CMashSpotShadowCaster::GetBias()const
	{
		return m_bias;
	}

	inline eSHADOW_MAP_FORMAT CMashSpotShadowCaster::GetTextureFormat()const
	{
		return m_textureFormat;
	}

	inline int32 CMashSpotShadowCaster::GetTextureSize()const
	{
		return m_textureSize;
	}
}

#endif