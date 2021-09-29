//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MODEL_H_
#define _MASH_MODEL_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"

namespace mash
{
	class MashAABB;
	class MashMatrix4;
	class MashTriangleCollider;
	class MashMesh;

    /*!
        Models contain a collection of meshes and collision information.
        Create from MashSceneManager::CreateModel().
     
        LODing infomation can also be set to optimize scenes.
    */ 
	class MashModel : public MashReferenceCounter
	{
	public:
		MashModel():MashReferenceCounter(){}
		virtual ~MashModel(){}

		//! Triangle collection.
        /*!
            \return Triangle collection.
        */
		virtual MashTriangleCollider* GetTriangleCollider()const = 0;
        
        //! Sets a triangle collider for this mesh.
        /*!
            Triangle colliders are mainly used for collision and physics.
         
            \param collider Triangle collider.
        */
		virtual void SetTriangleCollider(MashTriangleCollider *collider) = 0;

        //! Creats an independent copy of this mesh.
		/*!
			\return A copy of this mesh.
		*/
		virtual MashModel* Clone()const = 0;

        //! Returns a mesh.
        /*! 
            \param index Mesh index. GetMeshCount() Can be used to determine how man meshes there are.
            \param lod Lod index. GetLodCount() can be used to determine lod count.
            \return Mesh or NULL if there was any errors.
        */
		virtual MashMesh* GetMesh(uint32 index, uint32 lod = 0)const = 0;
        
        //! Gets the number of meshes at a particular lod index.
        /*!
            \param lod Lod index. GetLodCount() Can be used to determine lod count.
            \return Mesh count at a particular lod index.
        */
		virtual uint32 GetMeshCount(uint32 lod)const = 0;
        
        //! Gets the number of lods in this mesh.
        /*!
            \return Lods within this mesh.
        */
		virtual uint32 GetLodCount()const = 0;

        //! Bounding box that contains all the meshes.
        /*!
            \return Bounding box that contains all meshes.
        */
		virtual const MashAABB& GetBoundingBox()const = 0;
        
        //! Manually set the bounding box.
        /*!
            Consider using CalculateBoundingBox().
         
            \param aabb Custom bounding box.
        */
		virtual void SetBoundingBox(const MashAABB &aabb) = 0;
        
        //! Calculates the bounding box that contains all the meshes.
        /*!
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS CalculateBoundingBox() = 0;

        //! Appends one lod of meshes to this model.
        /*!
            Your copy of the meshes can be dropped after calling this.
         
            \param meshLODArray Array of meshes for this lod.
            \param meshLODCount Number of meshes in the array.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS Append(MashMesh **meshLODArray, uint32 meshLODCount = 1) = 0;

        //! Gets the unique model ID.
        /*!
            This id is generated for each model created.
         
            \return Model id.
        */
		virtual int32 GetID()const = 0;

        //! Drops all triangle buffers for all meshes contained within this model.
        /*!
            This is just a convenience method for freeing mesh triangle buffers.
            This is automatically done by the engine at the end of the init function so
            a user wouldn't normally need to call this.
        */
		virtual void DropAllTriangleBuffers() = 0;
	};
}

#endif