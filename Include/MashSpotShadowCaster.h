//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_SPOT_SHADOW_CASTER_H_
#define _MASH_SPOT_SHADOW_CASTER_H_

#include "MashShadowCaster.h"

namespace mash
{
    /*!
        Shadow caster for spot shadow light sources.
    */
	class MashSpotShadowCaster : public MashShadowCaster
	{
	public:
		enum eCASTER_TYPE
		{
            //! Standard filtered shadows.
			aCASTER_TYPE_STANDARD,
            //! Uses a modified version of the exponential shadow map algorithm. 
			aCASTER_TYPE_ESM
		};
	public:
		MashSpotShadowCaster():MashShadowCaster(){}
		virtual ~MashSpotShadowCaster(){}

        //! Sets the number of shadow map samples per pixel.
        /*!
            This should be used with SetSampleSize() to blur a shadows edges.

            \param samples Number of samples to take when calculating the final shadow value for a pixel.
         */
		virtual void SetSamples(eSHADOW_SAMPLES samples) = 0;
        
        //! Sets the shadow map sample area
        /*!
            This should be a fairly small value and is dependent on the texture size.
            A value of 1.0 means samples will be takes from random places across the
            whole shadow map, which would produce bad visual results. A very small value
            will mean that samples will most likely never deviate from the current text,
            thus producing hard shadow edges.

            \param size A valid range is between 0.0 - 1.0. About 0.003 is a good place to start.
         */
		virtual void SetSampleSize(f32 size) = 0;
        
        //! Returns the sample count.
		virtual eSHADOW_SAMPLES GetSampleCount()const = 0;
        
        //! Returns the shadow sample size.
		virtual f32 GetSampleSize()const = 0;
        
        //! Sets the shadow bias.
        /*!
            This can help reduce shadow acne. But higher value can increase peter panning.
            If this doesn't work enough for your scene then consider UseBackfaceGeometry().

            \param bias A valid range is usually between 0.0 - 1.0. About 0.0005 is a good place to start.
         */
		virtual void SetBias(f32 bias) = 0;
        
        //! Sets the shadow map format.
        /*!
            Higher values can produce a better shadow with less shadow acne at the cost
            application performance.

            \param format Texture format.
         */
		virtual void SetTextureFormat(eSHADOW_MAP_FORMAT format) = 0;
        
        //! Sets the shadow map texture size.
        /*!
            Higher values can produce sharper shadow maps at the cost of bandwidth.

            \param shadowMapSize. Eg, 512, 1024, 2048. 
         */
		virtual void SetTextureSize(int32 shadowMapSize) = 0;
        
         //! Returns the current bias value.
		virtual f32 GetBias()const = 0;
        
        //! Returns the current texture format.
		virtual eSHADOW_MAP_FORMAT GetTextureFormat()const = 0;
        
        //! Returns the current texture size.
		virtual int32 GetTextureSize()const = 0;
        
        //! Enables or disables back face rendering.
        /*!
            Enabling this can almost remove all shadow acne. The shadow bias should be
            set to 0.0 to reduce peter panning.

            Note, front facing geometry will not receive shadows from backfacing geometry
            with this enabled. Depending on your scene this may be undesirable.
         */
		virtual void UseBackfaceGeometry(bool enable) = 0;
        
        //! Returns true if backface rendering is enabled.
		virtual bool GetBackfaceRenderingEnabled()const = 0;

		//! Sets the exponential shadow map darkening factor.
        /*!
            Only valid if this caster was created with aCASTER_TYPE_ESM.

            Higher values will make the shadows darker and reduce light bleeding
            near the base of shadow casters. Higher values can also produce
            harder shadow edges.

            \param darkening Value starting from 0.0. Default is 30.0.
         */
		virtual void SetESMDarkeningFactor(f32 darkening) = 0;
        
        //! Returns the exponential shadow map darkening factor.
		virtual f32 GetESMDarkeningFactor()const = 0;
	};
}

#endif