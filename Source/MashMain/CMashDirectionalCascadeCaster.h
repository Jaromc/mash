//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _CMASH_DIRECTIONAL_CASCADE_CASTER_H_
#define _CMASH_DIRECTIONAL_CASCADE_CASTER_H_

#include "MashDirectionalShadowCascadeCaster.h"
#include "MashRenderSurface.h"
#include "MashMaterial.h"
#include "MashAutoEffectParameter.h"
#include "MashCamera.h"
#include "MashLight.h"
#include "MashMatrix4.h"
#include "MashVector2.h"
#include "MashEffect.h"

namespace mash
{
	class CMashDirectionalCascadeCaster : public MashDirectionalShadowCascadeCaster
	{
		class MashParamCamViewToLightVPArray : public MashAutoEffectParameter
		{
			static int8 *m_autoName;
		public:
			MashParamCamViewToLightVPArray():MashAutoEffectParameter(){}
			~MashParamCamViewToLightVPArray(){}

			void OnSet(const MashRenderInfo *renderInfo, 
				MashEffect *effect, 
				MashEffectParamHandle *parameter,
				uint32 index = 0)
			{
				CMashDirectionalCascadeCaster *caster = (CMashDirectionalCascadeCaster*)renderInfo->GetShadowCaster();
				effect->SetMatrix(parameter, caster->GetCamViewToLightVPArray(), caster->GetCascadeCount());
			}

			const int8* GetParameterName()const{return m_autoName;}
		};

		class MashParamCamViewToLightVP : public MashAutoEffectParameter
		{
			static int8 *m_autoName;
		public:
			MashParamCamViewToLightVP():MashAutoEffectParameter(){}
			~MashParamCamViewToLightVP(){}

			void OnSet(const MashRenderInfo *renderInfo, 
				MashEffect *effect, 
				MashEffectParamHandle *parameter,
				uint32 index = 0)
			{
				CMashDirectionalCascadeCaster *caster = (CMashDirectionalCascadeCaster*)renderInfo->GetShadowCaster();
				effect->SetMatrix(parameter, &caster->GetCamViewToLightVPArray()[caster->GetActivePass()]);
			}

			const int8* GetParameterName()const{return m_autoName;}
		};


		class MashParamCascadeClipPlanes : public MashAutoEffectParameter
		{
			static int8 *m_autoName;
		public:
			MashParamCascadeClipPlanes():MashAutoEffectParameter(){}
			~MashParamCascadeClipPlanes(){}

			void OnSet(const MashRenderInfo *renderInfo, 
				MashEffect *effect, 
				MashEffectParamHandle *parameter,
				uint32 index = 0)
			{
				CMashDirectionalCascadeCaster *caster = (CMashDirectionalCascadeCaster*)renderInfo->GetShadowCaster();
				effect->SetVector2(parameter, caster->GetCascadeClipPlanesArray(), caster->GetCascadeCount());
			}

			const int8* GetParameterName()const{return m_autoName;}
		};

		class MashParamCamViewToLightView : public MashAutoEffectParameter
		{
			static int8 *m_autoName;
		public:
			MashParamCamViewToLightView():MashAutoEffectParameter(){}
			~MashParamCamViewToLightView(){}

			void OnSet(const MashRenderInfo *renderInfo, 
				MashEffect *effect, 
				MashEffectParamHandle *parameter,
				uint32 index = 0)
			{
				CMashDirectionalCascadeCaster *caster = (CMashDirectionalCascadeCaster*)renderInfo->GetShadowCaster();
				effect->SetMatrix(parameter, &caster->GetCamViewToLightView());
			}

			const int8* GetParameterName()const{return m_autoName;}
		};

		class MashParamCurrentCascade : public MashAutoEffectParameter
		{
			static int8 *m_autoName;
		public:
			MashParamCurrentCascade():MashAutoEffectParameter(){}
			~MashParamCurrentCascade(){}

			void OnSet(const MashRenderInfo *renderInfo, 
				MashEffect *effect, 
				MashEffectParamHandle *parameter,
				uint32 index = 0)
			{
				CMashDirectionalCascadeCaster *caster = (CMashDirectionalCascadeCaster*)renderInfo->GetShadowCaster();
				int32 pass = caster->GetActivePass();
				effect->SetInt(parameter, &pass);
			}

			const int8* GetParameterName()const{return m_autoName;}
		};

	private:
		MashVideo *m_renderer;
		uint32 m_cascadeCount;
		f32 m_edgeBlendDistance;
		f32 m_cascadeDivider;
		f32 m_bias;
		int32 m_textureSize;
		eSHADOW_MAP_FORMAT m_textureFormat;
		int32 m_blendState;
		int32 m_rasterizerState;
		f32 m_emsDarkeningFactor;
		bool m_useBackface;

		uint32 m_activePass;
		bool m_rebuidRenderTarget;
		f32 m_fixedShadowDistance;
		bool m_fixedShadowDistanceEnabled;

		f32 *m_cascadePartitions;
		MashMatrix4 *m_camViewToLightVP;
		MashVector2 *m_cascadeClipPlanes;
		MashMatrix4 m_camViewToLightView;
		static bool m_isCasterTypeInitialised;

		eSHADOW_SAMPLES m_samples;
		f32 m_sampleSize;
		MashRenderSurface *m_renderTarget;
		MashTextureState *m_textureState;

		eCASTER_TYPE m_casterType;

		eMASH_STATUS OnRebuildRenderTarget();
		MashRenderSurface* GetSceneRenderTarget();
		void OnPixelReceiverBuild(MashArray<sEffectMacro> &macros);
		void GetFrustumPoints(const MashCamera *camera, f32 near, f32 far, MashVector3 *pointsOut);
		void UpdateCascadePartitions();
	public:
		CMashDirectionalCascadeCaster(MashVideo *renderer, eCASTER_TYPE casterType);
		~CMashDirectionalCascadeCaster();

		void Serialise(MashXMLWriter *writer);
		void Deserialise(MashXMLReader *reader);
		int32 GetShadowCasterType()const;

		void SetSamples(eSHADOW_SAMPLES samples);
		void SetSampleSize(f32 size);
		eSHADOW_SAMPLES GetSampleCount()const;
		f32 GetSampleSize()const;

		MashTexture* GetShadowMap(MashTextureState const **textureStateOut, uint32 textureIndex);
		void OnLoad();
		void OnPassEnd(){}//does nothing
		eMASH_STATUS OnInitialise();

		void SetCascadeCount(uint32 count);
		void SetCascadeEdgeBlendDistance(f32 dist);
		void SetCascadeDivider(f32 div);
		void SetBias(f32 bias);
		void SetTextureFormat(eSHADOW_MAP_FORMAT format);
		void SetTextureSize(int32 shadowMapSize);

		f32 GetBias()const;
		eSHADOW_MAP_FORMAT GetTextureFormat()const;
		int32 GetTextureSize()const;
		uint32 GetCascadeCount()const;
		f32 GetCascadeEdgeBlendDistance()const;
		f32 GetCascadeDivider()const;
		uint32 GetNumPasses()const;

		virtual eMASH_STATUS OnPassSetup(MashLight *light, const MashCamera *camera, const MashAABB &sceneAABB);
		eMASH_STATUS OnPass(uint32 pass, MashLight *light, const MashCamera *camera, const MashAABB &sceneAABB);
		eLIGHTTYPE GetShadowType()const;

		const MashMatrix4* GetCamViewToLightVPArray()const;
		const MashVector2* GetCascadeClipPlanesArray()const;
		const MashMatrix4& GetCamViewToLightView()const;
		uint32 GetActivePass()const;
		void OnUnload(){}//not used

		void UseBackfaceGeometry(bool enable);
		bool GetBackfaceRenderingEnabled()const;

		void SetFixedShadowDistance(bool enable, f32 distance);
		f32 GetFixedShadowDistance()const;
		bool IsFixedShadowDistanceEnabled()const;

		void SetESMDarkeningFactor(f32 darkening);
		f32 GetESMDarkeningFactor()const;
	};

	inline bool CMashDirectionalCascadeCaster::GetBackfaceRenderingEnabled()const
	{
		return m_useBackface;
	}

	inline bool CMashDirectionalCascadeCaster::IsFixedShadowDistanceEnabled()const
	{
		return m_fixedShadowDistanceEnabled;
	}

	inline f32 CMashDirectionalCascadeCaster::GetFixedShadowDistance()const
	{
		return m_fixedShadowDistance;
	}

	inline f32 CMashDirectionalCascadeCaster::GetESMDarkeningFactor()const
	{
		return m_emsDarkeningFactor;
	}

	inline uint32 CMashDirectionalCascadeCaster::GetNumPasses()const
	{
		return m_cascadeCount;
	}

	inline uint32 CMashDirectionalCascadeCaster::GetActivePass()const
	{
		return m_activePass;
	}

	inline const MashMatrix4* CMashDirectionalCascadeCaster::GetCamViewToLightVPArray()const
	{
		return m_camViewToLightVP;
	}

	inline const MashVector2* CMashDirectionalCascadeCaster::GetCascadeClipPlanesArray()const
	{
		return m_cascadeClipPlanes;
	}

	inline const MashMatrix4& CMashDirectionalCascadeCaster::GetCamViewToLightView()const
	{
		return m_camViewToLightView;
	}

	inline eLIGHTTYPE CMashDirectionalCascadeCaster::GetShadowType()const
	{
		return aLIGHT_DIRECTIONAL;
	}

	inline f32 CMashDirectionalCascadeCaster::GetBias()const
	{
		return m_bias;
	}

	inline eSHADOW_MAP_FORMAT CMashDirectionalCascadeCaster::GetTextureFormat()const
	{
		return m_textureFormat;
	}

	inline int32 CMashDirectionalCascadeCaster::GetTextureSize()const
	{
		return m_textureSize;
	}

	inline uint32 CMashDirectionalCascadeCaster::GetCascadeCount()const
	{
		return m_cascadeCount;
	}

	inline f32 CMashDirectionalCascadeCaster::GetCascadeEdgeBlendDistance()const
	{
		return m_edgeBlendDistance;
	}

	inline f32 CMashDirectionalCascadeCaster::GetCascadeDivider()const
	{
		return m_cascadeDivider;
	}

	inline MashRenderSurface* CMashDirectionalCascadeCaster::GetSceneRenderTarget()
	{
		return m_renderTarget;
	}

	inline eSHADOW_SAMPLES CMashDirectionalCascadeCaster::GetSampleCount()const
	{
		return m_samples;
	}

	inline f32 CMashDirectionalCascadeCaster::GetSampleSize()const
	{
		return m_sampleSize;
	}
}

#endif