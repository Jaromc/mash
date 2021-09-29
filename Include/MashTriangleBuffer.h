//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_TRIANGLE_BUFFER_H_
#define _MASH_TRIANGLE_BUFFER_H_

#include "MashReferenceCounter.h"
#include "MashTypes.h"
#include "MashArray.h"

namespace mash
{
    /*!
        Triangle buffers hold triangle data for meshes. These can be added to
        MashTriangleColliders for collision detection or used as physics collider objects.
     
        These can be created from MashSceneManager::CreateTriangleBuffer().
     
        Call Set() to set triangle buffer data.
    */
	class MashTriangleBuffer : public MashReferenceCounter
	{
	public:
		MashTriangleBuffer():MashReferenceCounter(){}
		virtual ~MashTriangleBuffer(){}

        //! Gets the vertex list.
        /*!
            This is an optimized list of points that must be used with the
            index list to loop through all triangles.
         
            \return Vertex list. This list must not be modified.
        */
		virtual const MashArray<MashVector3>& GetVertexList()const = 0;

		//! Gets the normal list.
		/*!
			This list is the size of GetVertexList().

			\return Normal list. This list must not be modified.
		*/
		virtual const MashArray<MashVector3>& GetNormalList()const = 0;

		//! Gets the position of a normal within the index list.
        /*!
            Equivalent to GetNormalList[(triangle * 3) + point];
         
            \return Triangle point in the index list.
        */
		virtual uint32 GetNormalIndex(uint32 triangle, uint32 point)const = 0;;

		//! Gets the index list for normals.
        /*!
            The normals for a triangle are made up from every 3 indices. So elements 0,1,2 == triangle 1,
            elements 3,4,5 == triangle 2, and so on...
         
            \return Normal index list. This list must not be modifies.
        */
		virtual const MashArray<uint32>& GetNormalIndexList()const = 0;
        
        //! Gets the index list.
        /*!
            Triangles are made up from every 3 indices. So elements 0,1,2 == triangle 1,
            elements 3,4,5 == triangle 2, and so on...
         
            \return Index list. This list must not be modifies.
        */
		virtual const MashArray<uint32>& GetIndexList()const = 0;
        
        //! Gets the triangle list.
        /*!
            This list contains extra data about each triangle.
         
            \return Triangle list.
        */
		virtual const MashArray<sTriangleRecord>& GetTriangleList()const = 0;
        
        //! Gets skinning data about each trinagle.
        /*!
            This list is only created if the original mesh had skinning data.
         
            \return Triangle list.
        */
		virtual const MashArray<sTriangleSkinnngRecord>& GetTriangleSkinningList()const = 0;
        
        //! Gets the triangle count.
		virtual uint32 GetTriangleCount()const = 0;

        //! Gets the normal for a triangles point.
        /*!
			If the buffers vertices have been smoothed out then each point may have
			a different normal.

            \param triangle Triangle to query.
			\param point Points 0, 1 or 2. 
            \return Triangle normal.
        */
		virtual const MashVector3& GetNormal(uint32 triangle, uint32 point)const = 0;
        
        //! Gets the point from a triangle.
        /*!
            \param triangle Triangle the point belongs to.
            \param point Points 0, 1 or 2.
            \return Triangle point.
        */
		virtual const MashVector3& GetPoint(uint32 triangle, uint32 point)const = 0;
        
        //! Gets the position of a point within the index list.
        /*!
            Equivalent to GetIndexList[(triangle * 3) + point];
         
            \return Triangle point in the index list.
        */
		virtual uint32 GetIndex(uint32 triangle, uint32 point)const = 0;

        //! Fills this buffer with triangle data.
        /*!
            Any previous data set will be dropped.
         
            This data will be copied over into the buffer so your copy can be deleted
            after calling this.
         
            \param uniquePointCount Number of unique points in the array.
            \param uniquePoints An array of unique points.
            \param indexCount Index count.
            \param indexList Index list. This should contain 3 elements per triangle.
			\param normalCount Number of normals in the array.
            \param normalList An array of normals.
            \param normalIndexCount Normal index count.
            \param normalIndexList Normal index list. This should contain 3 elements per triangle.
            \param triangleRecordList One element per triangle.
            \param triangleSkinningRecordList One element per triangle. May be NULL.
        */
		virtual void Set(uint32 uniquePointCount, const MashVector3 *uniquePoints,
			uint32 indexCount, uint32 *indexList,
			uint32 normalCount, const MashVector3 *normalList,
			uint32 normalIndexCount, uint32 *normalIndexList,
			const sTriangleRecord *triangleRecordList, const sTriangleSkinnngRecord *triangleSkinningRecordList) = 0;
	};
}

#endif