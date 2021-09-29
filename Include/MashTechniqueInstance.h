//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_TECHNIQUE_INSTANCE_H_
#define _MASH_TECHNIQUE_INSTANCE_H_

#include "MashReferenceCounter.h"
#include "MashDataTypes.h"
#include "MashString.h"

namespace mash
{
	class MashVideo;
	class MashSceneManager;
	class MashTexture;
	class MashTechnique;
    class MashTextureState;
    struct sTexture;

    /*!
        Technique instances are used to reduce state changes and reduce memory
        consumption. They hold pointers to an MashTechnique that holds all the effect
        and state data.
     
        Technique instances are basically used to hold and render different textures
        for techniques that share the same effects.
    */
	class MashTechniqueInstance : public MashReferenceCounter
	{
	public:
		enum eMAX_TEXTURE_COUNT
		{
            //! Maximum number of textures a technique can hold.
			aMAX_TEXTURE_COUNT = 16
		};
	public:
		MashTechniqueInstance():MashReferenceCounter(){}
		virtual ~MashTechniqueInstance(){}

        //! Creates a new technique instance from this object.
        /*!
            The returned technique instance will share the same technique and effect data as this object.
            
            \param name Technique name.
            \param copyTextures Copies texture pointers over into the new instance.
            \return New technique instance.
        */
		virtual MashTechniqueInstance* CreateInstance(const MashStringc &name, bool copyTextures = true) = 0;
        
        //! Create a new independent technique from this object.
        /*!
            The returned technique will have new technique and effects data. This should
            rarely be used.
         
            \param name Technique name.
            \param copyTextures Copies texture pointers over into the new instance.
            \return New technique instance.
        */
		virtual MashTechniqueInstance* CreateIndependentCopy(const MashStringc &name, bool copyTextures = true) = 0;

        //! Sets a texture to an index.
        /*!
            Texture parameters within effects have an index appended to the parameter name.
            That index is used here to grab that parameters texture and send it to the GPU.
         
            Also see SetTextureState() to set the texture state.
            
            \param index Index to set this texture to. Must be less than aMAX_TEXTURE_COUNT.
            \param texture Texture to set.
        */
		virtual void SetTexture(uint32 index, MashTexture *texture) = 0;
        
        //! Sets a texture state to an index.
        /*!
            This texture state will be used when a  texture at that index is set.
         
            \param index Index to set this texture state to. Must be less than aMAX_TEXTURE_COUNT.
            \param state Texture state to set.
        */
		virtual void SetTextureState(uint32 index, MashTextureState *state) = 0;
        
        //! Gets a texture from an index.
        /*!
            The returned structure will contain a texture and state state.
         
            \param index Texture index to return. 
        */
		virtual const sTexture* GetTexture(uint32 index)const = 0;
        
        //! Gets the technique name.
		virtual const MashStringc& GetTechniqueName()const = 0;

        //! Gets the shared technique pointer.
        /*!
            This object holds all render state and effect data. This is shared by all
            instances derived from the same technique.
         
            \return Shared technique;
        */
		virtual MashTechnique* GetTechnique()const = 0;
		
        //! Gets the render key for this technique instance.
        /*!
            This is used to order similar techniques together when rendering scene objects
            to reduce state changes. See MashSceneManager::SetRenderKeyHashFunction().
         
            This will call MashSceneManager::GenerateRenderKeyForTechnique() when needed.
         
            \return Generated render key.
        */
		virtual uint32 GetRenderKey() = 0;

        //! Called by materials to set this technique for rendering.
        /*!
            Called internally from MashMaterial::OnSet().
         
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS _OnSet() = 0;
        
        //! Called on technique changes.
		virtual void _OnUnload() = 0;

        //! Called to set this techniques name.
        /*!
            Used internally.
         
            \param name New technique name.
        */
		virtual void _SetTechniqueName(const MashStringc &name) = 0;
	};
}

#endif
