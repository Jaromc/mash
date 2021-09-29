//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MESH_BUILDER_H_
#define _MASH_MESH_BUILDER_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"

namespace mash
{
	class MashMesh;
	class MashVector3;
	class MashAABB;
	class MashVertex;
	class MashTriangleBuffer;

    /*!
        Can be used to create some basic meshes or manipulate current ones.
    */
	class MashMeshBuilder : public MashReferenceCounter
	{
	public:
		enum eMESH_UPDATE_FLAGS
		{
			/*!
				This will use the triangle layout of a mesh to calculate new normals.
				If the mesh is made up of shared positions using indices then surface
				normals will be smooth. However, if the mesh is made up of non-shared vertices
				then the resulting normals will not be smooth, each face will be defined when
				light hits it.
			*/
			aMESH_UPDATE_NORMALS = 1,
			aMESH_UPDATE_TANGENTS = 2,
			aMESH_UPDATE_WELD = 4,
			aMESH_UPDATE_CHANGE_VERTEX_FORMAT = 8,
			aMESH_UPDATE_16BIT_INDICES = 16,

			/*!
				This flag will set mesh data even if no data needs to be updated.
				This is useful when creating new meshes. You can just pass in an
				empty mesh and let the update funtion fill it for you.
			*/
			aMESH_UPDATE_FILL_MESH = 32
		};

		struct sMesh
		{
			const uint8 *vertices;
			uint32 vertexCount;
			const uint8 *indices;
			uint32 indexCount;
			eFORMAT indexFormat;
			ePRIMITIVE_TYPE primitiveType;
			const mash::MashVector4 *boneWeightArray;
			const mash::MashVector4 *boneIndexArray;

			const sMashVertexElement *currentVertexElements;
			uint32 currentVertexElementCount;

			sMesh():currentVertexElements(0), currentVertexElementCount(0), boneIndexArray(0),
				boneWeightArray(0), vertices(0), vertexCount(0), indices(0), indexCount(0){}
		};
	public:
		MashMeshBuilder():MashReferenceCounter(){}
		virtual ~MashMeshBuilder(){}

        //! Creates a cube mesh.
        /*!
            \param mesh Mesh to create the cube with.
            \param width Width of the mesh to create.
            \param height Height of the mesh to create.
            \param depth Depth of the mesh to create.
            \param vertexFormat Destination format of the new mesh. This should match the material it will use.
            \param facing Facing direction of the mesh.
            \param translation Translation of the mesh.
            \return Ok on success, Failed otherwise.
        */
		virtual eMASH_STATUS CreateCube(MashMesh *mesh, 
				uint32 width, 
				uint32 height, 
				uint32 depth, 
				MashVertex *vertexFormat,
				const mash::MashVector3 &facing = mash::MashVector3(0.0f, 1.0f, 0.0f),
				const mash::MashVector3 &translation = mash::MashVector3(0.0f, 0.0f, 0.0f)) = 0;

        //! Creates a sphere mesh.
        /*!
            \param mesh Mesh to create the sphere with.
            \param radius Radius of the sphere.
            \param tessellateLevel Tessellation of the sphere.
            \param vertexFormat Destination format of the new mesh. This should match the material it will use.
            \param translation Translation of the mesh.
            \return Ok on success, Failed otherwise.
        */
		virtual eMASH_STATUS CreateSphere(MashMesh *mesh, 
				f32 radius, 
				uint32 tessellateLevel,
				MashVertex *vertexFormat,
				const mash::MashVector3 &translation = mash::MashVector3(0.0f, 0.0f, 0.0f)) = 0;

        //! Creates a plane mesh.
        /*!
            \param mesh Mesh to create the plane with.
            \param tessellateLevel Tessellation of the plane.
            \param width Width of the plane.
            \param height Height of the plane.
            \param vertexFormat Destination format of the new mesh. This should match the material it will use.
            \param facing Facing direction of the mesh.
            \param translation Translation of the mesh.
            \return Ok on success, Failed otherwise.
        */
		virtual eMASH_STATUS CreatePlane(MashMesh *mesh,
				uint32 tessellateLevel,
				uint32 width,
				uint32 height,
				MashVertex *vertexFormat,
				const mash::MashVector3 &facing = mash::MashVector3(0.0f, 1.0f, 0.0f),
				const mash::MashVector3 &translation = mash::MashVector3(0.0f, 0.0f, 0.0f)) = 0;

        //! Updates a mesh based on the flags given.
        /*!
            \param mesh Mesh to update.
            \param flags Bitwise flags of type eMESH_UPDATE_FLAGS.
            \param toVertex If the vertex format is not changing then set this to the meshes current format.
            \param weldVertexNormalTolerance Used for welding if that flag is set. Matching vertices with normals less than this value will be welded. This is used to stop a box becoming a sphere.
            \return Ok on success, Failed otherwise.
        */
		virtual eMASH_STATUS UpdateMesh(MashMesh *mesh, uint32 flags, MashVertex *toVertex = 0, f32 weldVertexNormalTolerance = mash::math::MaxFloat())const = 0;

        //! Updates a mesh based on the flags given.
        /*!
            This function can create a mesh from raw data.
         
            It is safe to pass pointers into the sMesh struct that live in the destination mesh.
			Internally this function will duplicate the data only if it has detected that
			duplication is necessary.
         
            \param destinationMesh Mesh data will be written here.
            \param mesh Raw mesh data.
            \param toVertex Destination vertex format.
            \param flags Bitwise flags of type eMESH_UPDATE_FLAGS.
            \param weldVertexNormalTolerance Used for welding if that flag is set. Matching vertices with normals less than this value will be welded. This is used to stop a box becoming a sphere.
            \return Ok on success, Failed otherwise.
            
        */
		virtual eMASH_STATUS UpdateMeshEx(MashMesh *destinationMesh, sMesh *mesh, MashVertex *toVertex, uint32 flags, f32 weldVertexNormalTolerance = mash::math::MaxFloat())const = 0;

        //! Calculates the AABB the encumpases the vertex list.
        /*!
            \param vertexDecl Vertex declaration of the elements in the vertex list.
            \param vertexList Raw vertex elements that are in the format of vertexDecl.
            \param vertexCount Number of vertices in the vertex list.
            \param boundingBox Returned bounding box.
            \return Ok on success, Failed otherwise.
        */
		virtual eMASH_STATUS CalculateBoundingBox(const MashVertex *vertexDecl, 
			const void *vertexList,
			uint32 vertexCount,
			mash::MashAABB &boundingBox)const = 0;
	};
}

#endif