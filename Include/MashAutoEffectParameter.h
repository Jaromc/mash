//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_AUTO_EFFECT_PARAMETER_H_
#define _MASH_AUTO_EFFECT_PARAMETER_H_

#include "MashReferenceCounter.h"
#include "MashTypes.h"
#include "MashEnum.h"
#include "MashMatrix4.h"

namespace mash
{
	class MashRenderInfo;
	class MashVideo;
	class MashEffect;
	class MashEffectParamHandle;
	class MashTextureState;

    /*!
        Auto effect parameters are detected by the engine when
        effects are loaded. This allows easy parameter handling
        for sending data to the GPU when effects are set for rendering.
    */
	class MashAutoEffectParameter : public MashReferenceCounter
	{
	public:
		MashAutoEffectParameter():MashReferenceCounter(){}
		virtual ~MashAutoEffectParameter(){}

        //! Uploads data for this parameter to the GPU.
		/*!
            Called for each parameter in an effect when it's time to
            send its data to the GPU.

            \param renderInfo Currenrt render data.
            \param effect Currenrt effect.
            \param parameter This effect parameter.
            \param index Mainly used for texture parameters that were postfixed with an index.
         */
		virtual void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0) = 0;

        //! The name used for this parameter in effect files.
		/*!
            This string will be compared to those found in effect files.
         
            \return A string containing the paramerter name.
         */
		virtual const int8* GetParameterName()const = 0;
	};

	class MashParameterWVP : public MashAutoEffectParameter
	{
	public:
		MashParameterWVP():MashAutoEffectParameter(){}
		~MashParameterWVP(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_WORLD_VIEW_PROJ];}
	};

	class MashParameterViewProj : public MashAutoEffectParameter
	{
	public:
		MashParameterViewProj():MashAutoEffectParameter(){}
		~MashParameterViewProj(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_VIEW_PROJECTION];}
	};

	class MashParameterWorld : public MashAutoEffectParameter
	{
	public:
		MashParameterWorld():MashAutoEffectParameter(){}
		~MashParameterWorld(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_WORLD];}
	};

	class MashParameterView : public MashAutoEffectParameter
	{
	public:
		MashParameterView():MashAutoEffectParameter(){}
		~MashParameterView(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_VIEW];}
	};

	class MashParameterProj : public MashAutoEffectParameter
	{
	public:
		MashParameterProj():MashAutoEffectParameter(){}
		~MashParameterProj(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_PROJECTION];}
	};

	class MashParameterWorldInvTrans : public MashAutoEffectParameter
	{
	public:
		MashParameterWorldInvTrans():MashAutoEffectParameter(){}
		~MashParameterWorldInvTrans(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_WORLD_INV_TRANSPOSE];}
	};

	class MashParameterViewInvTrans : public MashAutoEffectParameter
	{
	public:
		MashParameterViewInvTrans():MashAutoEffectParameter(){}
		~MashParameterViewInvTrans(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_VIEW_INV_TRANSPOSE];}
	};

	class MashParameterWorldViewInvTrans : public MashAutoEffectParameter
	{
	public:
		MashParameterWorldViewInvTrans():MashAutoEffectParameter(){}
		~MashParameterWorldViewInvTrans(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_WORLD_VIEW_INV_TRANSPOSE];}
	};

	class MashParameterInvViewProj : public MashAutoEffectParameter
	{
	private:
		mash::MashMatrix4 m_lastData;
	public:
		MashParameterInvViewProj():MashAutoEffectParameter(){}
		~MashParameterInvViewProj(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_INV_VIEW_PROJ];}
	};

	class MashParameterInvView : public MashAutoEffectParameter
	{
	private:
		mash::MashMatrix4 m_lastData;
	public:
		MashParameterInvView():MashAutoEffectParameter(){}
		~MashParameterInvView(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_INV_VIEW];}
	};

	class MashParameterInvProj : public MashAutoEffectParameter
	{
	private:
		mash::MashMatrix4 m_lastData;
	public:
		MashParameterInvProj():MashAutoEffectParameter(){}
		~MashParameterInvProj(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_INV_PROJECTION];}
	};

	class MashParameterTexture : public MashAutoEffectParameter
	{
	private:
		MashTextureState *m_pDefaultState;
	public:
		MashParameterTexture(MashVideo *pRenderer);
		~MashParameterTexture();

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_SAMPLER];}
	};

	class MashParameterLight : public MashAutoEffectParameter
	{
	public:
		MashParameterLight():MashAutoEffectParameter(){}
		~MashParameterLight(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_LIGHT];}
	};

	class MashParameterCameraNearFar : public MashAutoEffectParameter
	{
	public:
		MashParameterCameraNearFar():MashAutoEffectParameter(){}
		~MashParameterCameraNearFar(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_CAMERA_NEAR_FAR];}
	};

	class MashParameterShadowMap : public MashAutoEffectParameter
	{
	public:
		MashParameterShadowMap(MashVideo *pRenderer);
		~MashParameterShadowMap(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_SCENE_SHADOWMAP];}
	};

	class MashParameterGBufferDiffuseSampler : public MashAutoEffectParameter
	{
	public:
		MashParameterGBufferDiffuseSampler(MashVideo *pRenderer);
		~MashParameterGBufferDiffuseSampler(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_GBUFFER_DIFFUSE_SAMPLER];}
	};

	class MashParameterGBufferLightSampler : public MashAutoEffectParameter
	{
	public:
		MashParameterGBufferLightSampler(MashVideo *pRenderer);
		~MashParameterGBufferLightSampler(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_GBUFFER_LIGHT_SAMPLER];}
	};

	class MashParameterGBufferLightSpecualrSampler : public MashAutoEffectParameter
	{
	public:
		MashParameterGBufferLightSpecualrSampler(MashVideo *pRenderer);
		~MashParameterGBufferLightSpecualrSampler(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_GBUFFER_LIGHT_SPECULAR_SAMPLER];}
	};

	class MashParameterGBufferSpecualrSampler : public MashAutoEffectParameter
	{
	public:
		MashParameterGBufferSpecualrSampler(MashVideo *pRenderer);
		~MashParameterGBufferSpecualrSampler(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_GBUFFER_SPECULAR_SAMPLER];}
	};

	class MashParameterGBufferNormalSampler : public MashAutoEffectParameter
	{
	public:
		MashParameterGBufferNormalSampler(MashVideo *pRenderer);
		~MashParameterGBufferNormalSampler(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_GBUFFER_NORMAL_SAMPLER];}
	};

	class MashParameterGBufferDepthSampler : public MashAutoEffectParameter
	{
	public:
		MashParameterGBufferDepthSampler(MashVideo *pRenderer);
		~MashParameterGBufferDepthSampler(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_GBUFFER_DEPTH_SAMPLER];}
	};
	
	class MashParamBonePaletteArray : public MashAutoEffectParameter
	{
	public:
		MashParamBonePaletteArray():MashAutoEffectParameter(){}
		~MashParamBonePaletteArray(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_BONE_PALETTE_ARRAY];}
	};

	class MashParamLightWorldPosition : public MashAutoEffectParameter
	{
	public:
		MashParamLightWorldPosition():MashAutoEffectParameter(){}
		~MashParamLightWorldPosition(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_LIGHT_WORLD_POSITION];}
	};

	class MashParamWorldView : public MashAutoEffectParameter
	{
	private:
		mash::MashMatrix4 m_lastMatrix;
	public:
		MashParamWorldView():MashAutoEffectParameter(){}
		~MashParamWorldView(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_WORLD_VIEW];}
	};

	class MashParamWorldPosition : public MashAutoEffectParameter
	{
	public:
		MashParamWorldPosition():MashAutoEffectParameter(){}
		~MashParamWorldPosition(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_WORLD_POSITION];}
	};

	class MashParamShadowsEnabled : public MashAutoEffectParameter
	{
	public:
		MashParamShadowsEnabled():MashAutoEffectParameter(){}
		~MashParamShadowsEnabled(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_effectAutoNames[aEFFECT_SHADOWS_ENABLED];}
	};

	class MashParamGUIAlphaMaskThreshold : public MashAutoEffectParameter
	{
	private:
		f32 m_threshold;
	public:
		MashParamGUIAlphaMaskThreshold():m_threshold(0.0f){}
		~MashParamGUIAlphaMaskThreshold(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		void SetValue(f32 f){m_threshold = f;}
		const int8* GetParameterName()const{return g_additionalEffectAutoNames[aADDITIONAL_EFFECT_GUI_ALPHAMASK_THRESHOLD];}
	};

	class MashParamGUIBaseColour : public MashAutoEffectParameter
	{
	private:
		sMashColour4 m_colour;
	public:
		MashParamGUIBaseColour():m_colour(){}
		~MashParamGUIBaseColour(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		void SetValue(const sMashColour4 &c){m_colour = c;}
		const int8* GetParameterName()const{return g_additionalEffectAutoNames[aADDITIONAL_EFFECT_GUI_BASE_COLOUR];}
	};

	class MashParamGUIFontColour : public MashAutoEffectParameter
	{
	private:
		sMashColour4 m_colour;
	public:
		MashParamGUIFontColour():m_colour(){}
		~MashParamGUIFontColour(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		void SetValue(const sMashColour4 &c){m_colour = c;}
		const int8* GetParameterName()const{return g_additionalEffectAutoNames[aADDITIONAL_EFFECT_GUI_FONT_COLOUR];}
	};

	class MashParameterParticleBuffer : public MashAutoEffectParameter
	{
	public:
		MashParameterParticleBuffer(){}
		~MashParameterParticleBuffer(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_additionalEffectAutoNames[aADDITIONAL_EFFECT_PARTICLE_BUFFER];}
	};

	class MashParamSoftParticleScale : public MashAutoEffectParameter
	{
	public:
		MashParamSoftParticleScale():MashAutoEffectParameter(){}
		~MashParamSoftParticleScale(){}

		void OnSet(const MashRenderInfo *renderInfo, 
			MashEffect *effect, 
			MashEffectParamHandle *parameter,
			uint32 index = 0);

		const int8* GetParameterName()const{return g_additionalEffectAutoNames[aADDITIONAL_EFFECT_SOFT_PARTICLE_SCALE];}
	};

}

#endif