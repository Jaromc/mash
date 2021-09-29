//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_POINT_SHADOW_CASTER_H_
#define _MASH_POINT_SHADOW_CASTER_H_

#include "MashShadowCaster.h"

namespace mash
{
    /*!
        Shadow caster for point light sources.
     
        The shadow scene is rendered 6 times to complete a cube map for shadows
        so this caster can be fairly slow. For this reason point light shadows
        should be used carefully.
    */
	class MashPointShadowCaster : public MashShadowCaster
	{
	public:
		enum eCASTER_TYPE
		{
            //! Standard hard shadows.
			aCASTER_TYPE_STANDARD,
            //! Uses simple filtering to blur the edges.
			aCASTER_TYPE_STANDARD_FILTERED,
            //! Uses a modified version of the exponential shadow map algorithm. 
			aCASTER_TYPE_ESM
		};
	public:
		MashPointShadowCaster():MashShadowCaster(){}
		virtual ~MashPointShadowCaster(){}

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