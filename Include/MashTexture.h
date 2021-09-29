//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_TEXTURE_H_
#define _MASH_TEXTURE_H_

#include "MashResource.h"
#include "MashString.h"

namespace mash
{
    /*!
        Texture states describe how the texture should be read from within a shader.
    */
	class MashTextureState : public MashReferenceCounter
	{
	private:
		sSamplerState m_samplerData;
	public:
		MashTextureState(const sSamplerState &_stateData):MashReferenceCounter(),
			m_samplerData(_stateData){}

		virtual ~MashTextureState(){}

		const sSamplerState* GetSamplerState()const{return &m_samplerData;}
	};

    /*!
        This maybe a 1D, 2D, or cube texture.
    */
	class MashTexture : public MashResource
	{
	public:
		MashTexture():MashResource(){}
		virtual ~MashTexture(){}

        //! Clones this resource.
        /*!
            Copies all texture data into a new buffer.
            
            \param name New texture name.
            \return New texture.
        */
		virtual MashTexture* Clone(const MashStringc &name)const = 0;

        //! Locks this buffer for reading or writing.
        /*!
            This is only valid if this texture was created as a dynmaic resource.
            Be sure to call Unlock() when your done with the data.
         
            \param type Lock type.
            \param data This will return a pointer to the start of this buffer.
            \param level Mip level to lock. 0 is default.
            \param face Texture face of type eCUBEMAP_FACE.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS Lock(eBUFFER_LOCK type, void **data, uint32 level = 0, uint32 face = 0) = 0;
        
        //! Unlocks the texture buffer.
        /*!
            Called after Lock() so the buffer is ready for rendering.
         
            \param level Mip level to unlock. 0 is default.
            \param face Texture face of type eCUBEMAP_FACE.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS Unlock(uint32 level = 0, uint32 face = 0) = 0;

        //! Gets the texture name
		virtual const MashStringc& GetName()const = 0;
        
        //! Gets the number of miplevels in this texture.
		virtual uint32 GetMipmapCount()const = 0;

        //! Gets the texture size.
        /*!
            \param width Pixel width.
            \param height Pixel height.
        */
		virtual void GetSize(uint32 &width, uint32 &height)const = 0;
        
        //! Gets this textures unique runtime id.
		virtual uint32 GetTextureID()const = 0;
	};

}

#endif