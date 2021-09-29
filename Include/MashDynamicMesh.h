//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_DYNAMIC_MESH_H_
#define _MASH_DYNAMIC_MESH_H_

#include "MashMesh.h"

namespace mash
{
    /*!
        Dynamic meshes can be used for meshes that have there vertex
        data changed often at runtime. The buffers used are optimised
        for frequent data changes. 
     
        These can be created from MashSceneManager::CreateDynamicMesh().
    */
	class MashDynamicMesh : public MashMesh
	{
	public:
		MashDynamicMesh():MashMesh(){}
		virtual ~MashDynamicMesh(){}

        //! Adds new geometry to the current buffers.
		/*! 
			calculateBoundingBox should only be set to true for procedurally created meshes.
			Imported objects have their bounds calculated to include maximum animated (skinned) movement.
			This is important for broad phase collision detection and shadow map generation.

			If you know what the maximum bounds will be then use SetBoundingBox().

            \param vertexBuffer Vertex data to append.
            \param vertexCount Number of elements in the new vertex buffer.
            \param vertexType Vertex decl for the new data. NOTE, this must be the same for all geometry appended until ClearGeometry() is called.
            \param indexBuffer Vertex indices.
            \param indexCount Indice count.
            \param primitiveType Primitive type of the data (must be the same for all appended data).
            \param primitiveCount Primitive count of the new data.
			\param calculateBoundingBox Creates a tight bounds around the new points. 
            \return returns the status of the function.
        */
		virtual eMASH_STATUS AppendGeometry(const void *vertexBuffer,
				uint32 vertexCount,
				const MashVertex *vertexType,
				const void *indexBuffer,
				eFORMAT indexFormat,
				uint32 indexCount,
				ePRIMITIVE_TYPE primitiveType,
				uint32 primitiveCount,
				bool calculateBoundingBox) = 0;

        //! Sets the buffer size for this mesh.
		/*!
            If you plan on doing alot of calls to AppendGeometry() at run time then
            its a good idea to set the API buffer sizes once at startup to be equal to
            the maximum size the buffers may reach. Otherwise the buffers may have
            to dynamically grow several times.
            If this upper limit is reached then the buffers will simply reserve more space.
         
            \param vertexBufferSizeInBytes Vertex buffer size in bytes.
            \param indexBufferSizeInBytes Index buffer size in bytes.
        */
		virtual eMASH_STATUS SetReservedBufferSize(uint32 vertexBufferSizeInBytes, uint32 indexBufferSizeInBytes) = 0;

        //! Sets the vertex and index buffers back to zero.
		/*!
            This doesnt free any memory. Instead, it just resets the element
            counters to zero.
            You may also use different vertex and primitive types after this is called.
        */
		virtual void ClearGeometry() = 0;
	};
}

#endif