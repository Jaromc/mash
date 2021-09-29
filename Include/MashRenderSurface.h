//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_RENDER_SURFACE_H_
#define _MASH_RENDER_SURFACE_H_

#include "MashResource.h"
#include "MashEnum.h"

namespace mash
{
	class MashTexture;

    /*!
        These are render targets that can be used for post processing effects or
        any other special purpose.
     
        Surfaces may have 1 or more targets that pixel shaders may write to.
     
        To Set a render target call MashVideo::SetRenderTarget(). The default render target must
        be set again before the end of a frame. This can be done by calling MashVideo::SetRenderTargetDefault().
     
        When a surface is set the viewport will also change to match the surface dimension. This feature can be
        enabled or disabled using SetAutoViewport(), Most of the time however you want to leave this set to true.
    */
	class MashRenderSurface : public MashResource
	{
	public:
		MashRenderSurface():MashResource(){}
		virtual ~MashRenderSurface(){}

        //! Creates an independent copy of this surface.
		virtual MashRenderSurface* Clone()const = 0;

        //! Not used. 
		virtual eMASH_STATUS Lock(eBUFFER_LOCK type, void **data, int32 level = 0, int32 face = 0){return aMASH_FAILED;}
        
        //! Not used.
		virtual eMASH_STATUS Unlock(uint32 level = 0, uint32 face = 0){return aMASH_FAILED;}
        
        //! Gets the resource type.
		virtual eRESOURCE_TYPE GetType()const;
        
        //! Gets a target surface. This texture can be passed to shaders.
        /*!
            \param texture Target surface to return.
            \return Texture.
        */
		virtual MashTexture* GetTexture(uint32 texture)const = 0;
        
        //! Gets the surface count.
		virtual uint32 GetTextureCount()const = 0;

        //! Calculates the dimentions of the textures.
		virtual mash::MashVector2 GetDimentions()const = 0;
        
        //! Enables or disables the auto viewport
        /*
            This is only set to false in special situations.
            When a render surface is set, a viewport is also set that matches this surfaces area.
            This is the default setting. Pass in false to disable this feature.
         
            \param state Enable or disable auto viewport.
        */
		virtual void SetAutoViewport(bool state) = 0;
        
        //! Gets the auto viewport state.
		virtual bool GetAutoViewport()const = 0;

		//! Generate mips for a surface.
		/*!
			This will only succeed if the render target was create with mipmaps enabled.

			\param surface Surface to generate mips for. Use -1 to generate for all surfaces.
		*/
		virtual void GenerateMips(int32 surface = -1) = 0;

        //! Used internally to clear the targets.
        /*!
            \param clearFlags Bitwise flags of type eCLEAR_FLAG.
            \param colour Clear colour.
            \param depth Clear depth value.
        */
		virtual void _ClearTargets(uint32 clearFlags, const sMashColour4 &colour, f32 depth) = 0;
		
        //! Used internally to free any resources before a screen resize.
		virtual eMASH_STATUS OnPreResize() = 0;
        
		//! Used internally to rebuild resources after a screen resize.
        /*!
            \param width New screen width.
            \param height New screen height.
        */
		virtual eMASH_STATUS OnPostResize(uint32 width, uint32 height) = 0;
        
        //! Called internally to set the render surface.
        /*!
            \param surface Surface to set. Set to -1 to set all surfaces.
         */
		virtual eMASH_STATUS OnSet(int32 surface = -1) = 0;
        
        //! Called internally on a surface change.
		virtual void OnDismount() = 0;
	};

	inline eRESOURCE_TYPE MashRenderSurface::GetType()const
	{
		return aRESOURCE_RENDER_SURFACE;
	}
}	

#endif