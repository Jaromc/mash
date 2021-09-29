//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_ENTITY_H_
#define _MASH_ENTITY_H_

#include "MashSceneNode.h"

namespace mash
{
	class MashSkin;
	class MashModel;
	class MashSubEntity;
	class MashMaterial;

    /*!
        An Entity is a renderable object in your scene. Characters, world objects, landscapes, 
        are all examples of Entities.
     
        Entities are broken up into sub entities to support multiple materials and levels of detail (LOD).
        Sub entities are not scene nodes, they are basically a wrapper for meshes used for rendering.

        LODs can be used for high and low detail meshes based on distance from the camera.
        Also be aware that materials also support LODing for high and low detail effects
        based on distance from the camera.
     
        If many of the same Entities are being used in your scene, then you should use one
        Entity as the base and clone it to improve Efficiency using MashSceneManager::AddInstance.
     
        Entities may also contain a MashSkin for skinned animation. This skin contains all the bones
        that deform this model during rendering.
    */
	class MashEntity : public MashSceneNode
	{
	public:
		MashEntity(MashSceneNode *parent,
			mash::MashSceneManager *manager,
				const MashStringc &userName):MashSceneNode(parent, manager, userName){}

		virtual ~MashEntity(){}

		//! Helper function. Sets a material to all sub entities.
		/*!
			\param material Material to apply to all sub entities.
		*/
		virtual void SetMaterialToAllSubEntities(MashMaterial *material) = 0;

		//! Applies a material to a sub entity.
		/*!
			The material can be dropped after calling this.

			\param material Material to apply.
			\param meshIndex Mesh index.
			\param lod Mesh lod.
		*/
		virtual void SetSubEntityMaterial(MashMaterial *material, uint32 meshIndex, uint32 lod = 0) = 0;

        //! Gets a sub entity.
        /*!
            \param meshIndex A sub entity from 0 to EntityCount - 1.
            \return A sub entity at the given index.
        */
		virtual MashSubEntity* GetSubEntity(uint32 meshIndex, uint32 lod = 0) = 0;
        
		//! Gets the number of lods in the entity.
		/*!
			\return Lod count.
		*/
		virtual uint32 GetLodCount()const = 0;

		//! Gets the mesh count for a lod.
		/*!
			\param lod Lod to query.
			\return Mesh count.
		*/
		virtual uint32 GetSubEntityCount(uint32 lod)const = 0;

		//! Sets where a lod will start.
		/*!
			This should be used in order from lod 0 - (n-1).
			Lod distance (n+1) must be less than n.

			\param lod Lod to set distance for.
			\param distance Lod distance.
		*/
		virtual eMASH_STATUS SetLodDistance(uint32 lod, uint32 distance) = 0;

		//! Gets the lod distance list.
		/*!
			\return lod distance list.
		*/
		virtual const MashArray<uint32>& GetLodDistances()const = 0;

        //! Gets the model this Entity is based on.
        /*!
            \return Base model.
         */
		virtual MashModel* GetModel()const = 0;
        
        //! Sets the model this Entity is based on.
        /*!
			This is mainly used at load time and should be the first function
			called when initializing an entity.

			A call to this will invalidate a number of values within an entity including,
			mesh lod distances and materials. These will need to be reset after
			calling this. 

            \param model Base model.
        */
		virtual void SetModel(MashModel *model) = 0;
        
		//! Gets the skeleton used by this entity.
		/*!
			\return Skin.
		*/
		virtual MashSkin* GetSkin()const = 0;

		//! Sets the skeleton this entity will use.
		/*!
			This function will grab a copy of the skin and drop any previously set.
            Note that any bones in the skin must still be attached to a scene graph
            to be updated. Skins are simply containers for bones and do not update
            th nodes.
         
			\param skin New skin.
		*/
		virtual void SetSkin(MashSkin *skin) = 0;
	};
}

#endif
