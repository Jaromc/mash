//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_DIRECTIONAL_SHADOW_CASCADE_CASTER_H_
#define _MASH_DIRECTIONAL_SHADOW_CASCADE_CASTER_H_

#include "MashShadowCaster.h"

namespace mash
{
    //! Shadow caster for directional cascaded shadow maps.
    /*!
        Cascaded shadow maps can improve shadows over large distances. It does this by
        partitioning a cameras view frustum from near to the far clipping plane. 
        Keep in mind there is 1 render target is created per cascade. And 1 render pass
        of all shadow objects per cascade. So performance can be greatly affected for many 
        cascades.
     
        Simple, small scenes may be fine with 1 cascade. This is similar to a normal
        shadow mapping algorithm.
     
        Settings should only be changed at initialisation if possible. Changing at runtime
        is possible however shaders maybe recompiled to be optimised for current settings.
    */
	class MashDirectionalShadowCascadeCaster : public MashShadowCaster
	{
	public:
		enum eCASTER_TYPE
		{
            //! Uses the default shadow caster. 
			aCASTER_TYPE_STANDARD,
            //! Uses a modified version of the exponential shadow map algorithm.
			aCASTER_TYPE_ESM
		};
	public:
		MashDirectionalShadowCascadeCaster():MashShadowCaster(){}
		virtual ~MashDirectionalShadowCascadeCaster(){}

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

        //! Sets the number of cascades.
        /*!
            This will divide a shadow scene up into segments to produce sharper
            shadows over a greater area.
         
            Keep in mind 1 render target is created per cascade. And 1 render pass
            of all shadow objects per cascade is done. So performance can be greatly
            affected for many cascades.
         
            \param count Number of cascades. Set to 1 by default. 3 is usually a good value for large scenes.
        */
		virtual void SetCascadeCount(uint32 count) = 0;
        
        //! Blurs cascade edges between two cascades.
        /*!
            This can help reduces the sharpness of cascade transition.
         
            \param dist Value between 0.0 and 1.0. 0.0 would produce sharp edges. 0.1 is a good default value.
        */
		virtual void SetCascadeEdgeBlendDistance(f32 dist) = 0;
        
        //! Sets the distribution of cascade from near to far of a cameras frustum.
        /*!
            Values of 0.0 means the cascades are distributed across the frustum linearly.
            Values closer to 1.0 move the distribution closer to the near frustum. This
            will produce sharper shadows closer to the viewer.
         
            \param div Distribution of shadow cascades between 0.0 and 1.0. Default is 0.5.
        */
		virtual void SetCascadeDivider(f32 div) = 0;
        
        //! Returns the cascade count.
		virtual uint32 GetCascadeCount()const = 0;
        
        //! Returns the edge blend distance.
		virtual f32 GetCascadeEdgeBlendDistance()const = 0;
        
        //! Returns the cascade Distribution.
		virtual f32 GetCascadeDivider()const = 0;
        
        //! Returns the number of render passes need to produce the final shadow map.
		virtual uint32 GetNumPasses()const = 0;
        
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

        //! Shadows will be cast from the active camera to this distance.
        /*!
            Any shadow casting node within this distance will cast a shadow.
            If this is false then the shadow map is calculated with a tight bounds
            around the shadow scene. This is fine for a static scene and can improve
            the quality of shadows. But for a dynamic scene this will cause shadow quality 
            to change as objects move closer or further away from the camera.
         
            By default a fixed distance is used.
         
            \param enable Enable a fixed shadow distance.
            \param distance Shadow distance.            
        */
		virtual void SetFixedShadowDistance(bool enable, f32 distance = 1000.0f) = 0;
        
        //! Returns the fixed shadow distance.
		virtual f32 GetFixedShadowDistance()const = 0;
        
        //! Returns true if fixed shadow distance is enabled.
		virtual bool IsFixedShadowDistanceEnabled()const = 0;

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