//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MESH_H_
#define _MASH_MESH_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"

namespace mash
{
	class MashVertex;
	class MashMeshBuffer;
	class MashAABB;
	class MashVector4;
	class MashGenericArray;
	class MashTriangleBuffer;

    /*!
        A mesh hold vertex and index data. Meshes may be dynamic or static, see
        those classes for more information about them.
    */
	class MashMesh : public MashReferenceCounter
	{			
	public:
		//! Constructor.
		MashMesh():MashReferenceCounter(){}
		//! Destructor.
		virtual ~MashMesh(){}

        //! Sets the data you want to save.
		/*!
			If you want some data to stay alive after init then you
			must set a flag for it. Any objects that have the Grab() Drop()
			interface will only be dropped. Meaning there may be copies
			of it alive after this function is called. The flags can be
            found in eSAVE_MESH_DATA_FLAGS. These flags can help reduce
            memory consumption.
            
            DeleteInitialiseData() is normally called by the engine after the initialise function.
            This data may also be delete as it's loaded if that option was selected.
         
            As long as everything you need has been created during the init function
            then most of the time you would leave this to its default setting
            which is to delete everything. Things that use this data are triangle
            colliders, physics objects, and materials. Again, if they have been created during
            the init function then you don't need to touch this.
         
            \param flags Save flags.
		*/
		virtual void SetSaveInitialiseDataFlags(uint8 flags) = 0;
        
        //! Gets the save init flags/
		virtual uint8 GetSaveInitialiseDataFlags()const = 0;
        
        //! Deletes init data.
        /*!
            This is automatically called by the engine after the init function.
            If the mesh is loaded after the init function then you may want
            to call this directly to free up unneeded memory. 
            After calling this raw data, bone data and triangle data may be invalid.
            See SetSaveInitialiseDataFlags() for more info.
        */
		virtual void DeleteInitialiseData() = 0;
        
        //! Creates an independent copy of this mesh.
        /*!
            \return Clone or null if an error has occured.
        */
		virtual MashMesh* Clone()const = 0;
        
        //! Number of vertices in this mesh.
        /*!
            \return Vertex count.
        */
		virtual uint32 GetVertexCount()const = 0;

        //! Number of indices in this mesh.
        /*!
            \return Index count.
        */
		virtual uint32 GetIndexCount()const = 0;
        
        //! Index format.
        /*!
            \return Index format.
        */
		virtual eFORMAT GetIndexFormat()const = 0;

        //! Primitive count.
        /*!
            \return Primtive count.
        */
		virtual uint32 GetPrimitiveCount()const = 0;
        
        //! Primitive type.
        /*!
            \return Primitive type.
        */
		virtual ePRIMITIVE_TYPE GetPrimitiveType()const = 0;
        
        //! Vertex declaration.
        /*!
			The mesh geometry is only concerned about stream 0 (geometry stream).

            \return Vertex declaration.
        */
		virtual MashVertex* GetVertexDeclaration()const = 0;

        //! Bounding box.
        /*!
			You can set this bounds yourself if a different bounds is required by
			calling SetBoundingBox() or by using the mesh builder.

            \return AABB containing all vertices. 
        */
		virtual const MashAABB& GetBoundingBox()const = 0;

        //! Sets the geometry for this mesh.
        /*!
			calculateBoundingBox should only be set to true for procedurally created meshes.
            This can be set manually by SetBoundingBox().
			Imported objects have their bounds calculated already.

            \param vertexBuffer Raw vertex buffer.
            \param vertexCount Number of element in the vertex buffer.
            \param VertexType Vertex format within the vertex buffer.
            \param indexBuffer Raw index buffer. May be null.
            \param indexCount Number of elements in the index buffer.
            \param indexFormat Format of the indices.
            \param primitiveType Primitive type of the vertex/index buffers.
            \param primitiveCount Number of primitives.
			\param calculateBoundingBox Creates a tight bounds around the new points. 
            \return Function status. 
        */
		virtual eMASH_STATUS SetGeometry(const void *vertexBuffer,
			uint32 vertexCount,
			const MashVertex *vertexType,
			const void *indexBuffer,
			uint32 indexCount,
			eFORMAT indexFormat,
			ePRIMITIVE_TYPE primitiveType,
			uint32 primitiveCount,
			bool calculateBoundingBox) = 0;

        //! Sets bone weights.
        /*!
            If this mesh is skinned, then this array must contain one element
            per vertex describing a bones weight on a vertex.
         
            4 influences are allowed per vertex between 0 and 1.
         
            \param weights Bones weights.
            \param count Number of elements in the array. This will be equal to the vertex count.
        */
		virtual void SetBoneWeights(const MashVector4 *weights, uint32 count) = 0;
        
        //! Sets bone indices.
        /*!
            If this mesh is skinned, then this array must contain one element
            per vertex describing which bones affect a vertex.
         
            4 influences are allowed per vertex and each bone ID will index
            a bone in some skeleton.
         
            \param indices Bones that affect each vertex.
            \param count Number of elements in the array. This will be equal to the vertex count.
         */
		virtual void SetBoneIndices(const MashVector4 *indices, uint32 count) = 0;

        //! Bone indices.
        /*!
            \return Bone indices. The number of elements is equal to the vertex count or NULL will be returned if not set.
        */
		virtual const MashVector4* GetBoneIndices()const = 0;
        
        //! Bone weights.
        /*!
            \return Bone weights. The number of elements is equal to the vertex count or NULL will be returned if not set.
        */
		virtual const MashVector4* GetBoneWeights()const = 0;

        //! Mesh buffer.
        /*!
            A mesh buffer is a compiled version of this mesh.
            \return Mesh buffer.
        */
		virtual MashMeshBuffer* GetMeshBuffer()const = 0;
        
        //! Raw vertices.
        /*!
            \return Raw vertex information.
        */
		virtual const MashGenericArray& GetRawVertices()const = 0;
        
        //! Raw indices.
        /*!
            \return Raw index information.
        */
		virtual const MashGenericArray& GetRawIndices()const = 0;

        //! Gets the triangle buffer.
        /*!
            \return Triangle buffer if set.
        */
		virtual MashTriangleBuffer* GetTriangleBuffer()const = 0;

		//! Sets the triangle buffer.
        /*!
            You should drop your copy of the buffer after calling this.
            
            \param buffer Triangle buffer to set.
        */
		virtual void SetTriangleBuffer(MashTriangleBuffer *buffer) = 0;

        //! Sets a vertex declaration.
        /*!
            Used internally.
         
            \param vertex Vertex declaration this mesh uses.
        */
		virtual void _SetVertexDeclaration(MashVertex *vertex) = 0;

		//! Sets the bounding box for this mesh.
        /*!
            This box is used for collision detection and other scene functions.
         
            \param boundingBox AABB to set for this mesh.
        */
		virtual void SetBoundingBox(const MashAABB &boundingBox) = 0;
	};
}

#endif