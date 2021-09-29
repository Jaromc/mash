//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_SKIN_H_
#define _MASH_SKIN_H_

#include "MashReferenceCounter.h"
#include "MashArray.h"

namespace mash
{
	class MashBone;
    class MashSceneNode;

    /*!
        Skins contain bone scene nodes that are used for skinning. These can be
        created from MashSceneManager::CreateSkin.
     
        Each animated entity instance will contain a skin that contains the
        bones that affect it.
     
        When an entity is rendered, OnRender() is called by the entity to fill 
        an array with bone offsets that can then be passed on to the GPU. This
        array can be accessed from shaders using the auto parameter "autoBonePalette".
     
        For new skins, AddBone() can be called to add each bone to this skin. Note that
        a skin will not update the bones added to it. Skins are simply containers for
        bones and they still need to be attached to a scene to be updated.
        Instances can be created using CreateInstance().
    */
	class MashSkin : public MashReferenceCounter
	{
	public:
		struct sBone
		{
			MashBone *node;
			uint32 id;
		};
	public:
		MashSkin():MashReferenceCounter(){}
		virtual ~MashSkin(){}

		//! Creates an instance of this skin.
        /*!
            This function uses a scene graph of already instanced nodes and
            and returns a new skin that can be used for an entity instance. 
         
            The root node passed in to this function should contain new instances of all 
            the bones currently contained within this skin.
         
            \param newInstanceRoot Root node of newly instanced nodes that will be used to fill this skin.
            \return New skin containing the instanced nodes.
        */
		virtual MashSkin* CreateInstance(MashSceneNode *newInstanceRoot) = 0;
        
        //! Fills an empty instance with data.
        /*!
            See CreateInstance() for more info.
         
            \param original Original skin this was instanced from.
            \param newInstanceRoot Root node of newly instanced nodes that will be used to fill this skin.
        */
        virtual void FillFrom(MashSkin *original, MashSceneNode *newInstanceRoot) = 0;

        //! This is equal to the largest boneId + 1, NOT the total bone count.
        /*!
            See AddBone for more info.
        */
		virtual uint32 GetBonePaletteLength()const = 0;
        
        //! Gets the bones used in this skin.
		virtual const MashArray<sBone>& GetBones()const = 0;
        
        //! Adds a bone to this skin.
        /*!
            \param bone Bone to add.
            \param boneId This should be the id of the bone that vertices index to get there offset
                data during skinning.
        */
		virtual void AddBone(MashBone *bone, uint32 boneId) = 0;
        
        //! Called by this skins owner before its rendered.
        /*!
            This will fill an array with bone offset data that can be accessed by the GPU
            for skinning. This array is accessed on the GPU via the auto param "autoBonePalette".
         
            If an elements matrix is set to the identity matrix then that means the bone
            is in its bind pose.
        */
		virtual void OnRender() = 0;
	};
}

#endif