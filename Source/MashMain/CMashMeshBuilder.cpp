//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashMeshBuilder.h"
#include "MashMaterial.h"
#include "MashVideo.h"
#include "MashGenericArray.h"
#include "MashTriangleBuffer.h"
#include "MashVector3.h"
#include "MashMatrix4.h"
#include "MashAABB.h"
#include "MashQuaternion.h"
#include "MashMesh.h"
#include "MashSkin.h"
#include "MashHelper.h"
namespace mash
{
	CMashMeshBuilder::CMashMeshBuilder(mash::MashVideo *pRenderDevice)
	{
		m_pRenderDevice = pRenderDevice;
	}

	eMASH_STATUS CMashMeshBuilder::CreatePlane(MashMesh *mesh,
			uint32 iTessellateLevel,
			uint32 iWidth,
			uint32 iHeight,
			MashVertex *pVertexFormat,
			const mash::MashVector3 &facing,
			const mash::MashVector3 &translation)
	{
		if (!pVertexFormat)
			return aMASH_FAILED;

		iTessellateLevel = math::Clamp<uint32>(1, UINT_MAX, iTessellateLevel);
		iWidth = math::Clamp<uint32>(1, UINT_MAX, iWidth);
		iHeight = math::Clamp<uint32>(1, UINT_MAX, iHeight);
		
		int32 iNumVertsX = iTessellateLevel+1;
		int32 iNumVertsY = iTessellateLevel+1;
		f32 fUvX = 1.0f / iTessellateLevel;
		f32 fUvY = 1.0f / iTessellateLevel;
		f32 fScaleX = (f32)iWidth / (f32)iTessellateLevel;
		f32 fScaleY = (f32)iHeight / (f32)iTessellateLevel;

		int32 iVertexCount = iNumVertsX * iNumVertsY;
		int32 iIndexCount = iTessellateLevel * iTessellateLevel * 6;

		MashGenericArray vertices;
		MashGenericArray indices;

		vertices.Reserve(sizeof(mash::MashVertexStandard::sMashVertexStandard) * iVertexCount);
		indices.Reserve(sizeof(uint32) * iIndexCount);

		mash::sMashColour colour = mash::sMashColour(255,255,255,255);

		mash::MashQuaternion rotateTo;
		rotateTo.RotateTo(mash::MashVector3(0.0f, 1.0f, 0.0f), facing);

		f32 halfWidth = iWidth / 2.0f;
		f32 halfHieght = iHeight / 2.0f;

		int32 iIndex = 0;
		for(int32 y = 0; y < iNumVertsY; ++y)
		{
			for(int32 x = 0; x < iNumVertsX; ++x)
			{
				mash::MashVertexStandard::sMashVertexStandard newVert(mash::MashVector3((f32)x * fScaleX, 0.0f,(f32)-y * fScaleY));
				newVert.texCoord = mash::MashVector2((f32)x * fUvX, (f32)y * fUvY);
				newVert.normal = mash::MashVector3(0.0f, 1.0f, 0.0f);

				newVert.position.x -= halfWidth;
				newVert.position.z += halfHieght;
				newVert.position = rotateTo.TransformVector(newVert.position);
				newVert.position += translation;

				newVert.normal = rotateTo.TransformVector(newVert.normal);
				newVert.normal.Normalize();

				vertices.Append(&newVert, sizeof(mash::MashVertexStandard::sMashVertexStandard));

				++iIndex;
			}
		}

		iIndex = 0;
		for(int32 y = 0; y < iTessellateLevel; ++y)
		{
			for(int32 x = 0; x < iTessellateLevel; ++x)
			{
				uint32 i1 = (y * iNumVertsX) + x;
				uint32 i2 = ((y + 1) * iNumVertsX) + x;
				uint32 i3 = (y * iNumVertsX) + x + 1;
				uint32 i4 = ((y+1) * iNumVertsX) + x;
				uint32 i5 = ((y+1) * iNumVertsX) + x + 1;
				uint32 i6 = (y * iNumVertsX) + x + 1;
				
				indices.Append(&i1, sizeof(uint32));
				indices.Append(&i3, sizeof(uint32));
				indices.Append(&i2, sizeof(uint32));
				indices.Append(&i5, sizeof(uint32));
				indices.Append(&i4, sizeof(uint32));
				indices.Append(&i6, sizeof(uint32));
				iIndex += 6;
			}
		}

		mash::MashVertexStandard primitiveVertexType;
		sMesh primitiveMesh;
		primitiveMesh.boneIndexArray = 0;
		primitiveMesh.boneWeightArray = 0;
		primitiveMesh.currentVertexElements = primitiveVertexType.m_elements;
		primitiveMesh.currentVertexElementCount = sizeof(primitiveVertexType.m_elements) / sizeof(sMashVertexElement);
		primitiveMesh.indexCount = iIndexCount;
		primitiveMesh.indexFormat = aFORMAT_R32_UINT;
		primitiveMesh.indices = (uint8*)indices.Pointer();
		primitiveMesh.vertexCount = iVertexCount;
		primitiveMesh.vertices = (uint8*)vertices.Pointer();
		primitiveMesh.primitiveType = aPRIMITIVE_TRIANGLE_LIST;

		uint32 flags = aMESH_UPDATE_FILL_MESH | aMESH_UPDATE_CHANGE_VERTEX_FORMAT;
		if (UpdateMeshEx(mesh, &primitiveMesh, pVertexFormat, flags) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Plane mesh creation failed",
				"CMashMeshBuilder::CreatePlane");

			return aMASH_FAILED;
		}

		mash::MashAABB boundingBox;
		if (CalculateBoundingBox(mesh->GetVertexDeclaration(), mesh->GetRawVertices().Pointer(), mesh->GetVertexCount(), boundingBox) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
				"Failed to create bounding box for mesh.",
				"CMashColladaLoader::CreatePlane");

			return aMASH_FAILED;
		}

		mesh->SetBoundingBox(boundingBox);

		return aMASH_OK;
	}

	eMASH_STATUS CMashMeshBuilder::CreateSphere(MashMesh *mesh, 
		f32 fRadius, 
		uint32 iTessellateLevel, 
		MashVertex *pVertexFormat,
		const mash::MashVector3 &translation)
	{
		if (!pVertexFormat)
			return aMASH_FAILED;

		const int32 NUM_SEGMENTS = 16;
		const int32 NUM_RINGS = 16;

		const uint32 iVertexCount = ( NUM_RINGS + 1 ) * ( NUM_SEGMENTS + 1 );
		const uint32 iIndexCount = 2 * NUM_RINGS * ( NUM_SEGMENTS + 1 );

		mash::sMashColour colour = mash::sMashColour(255,255,255,255);

		MashGenericArray vertices;
		MashGenericArray indices;

		vertices.Reserve(sizeof(mash::MashVertexStandard::sMashVertexStandard) * iVertexCount);
		indices.Reserve(sizeof(uint32) * iIndexCount);

		// Establish constants used in sphere generation
		f32 fDeltaRingAngle = ( mash::math::Pi() / NUM_RINGS );
		f32 fDeltaSegAngle = ( 2.0f * mash::math::Pi() / NUM_SEGMENTS );
		uint32 uiVertexIndex = 0 ; 

		// Generate the group of rings for the sphere
		for( uint32 ring = 0; ring < (NUM_RINGS + 1) ; ring++ )
		{
			f32 r0 = sinf( ring * fDeltaRingAngle );
			f32 z0 = cosf( ring * fDeltaRingAngle );

			// Generate the group of segments for the current ring
			for( uint32 seg = 0; seg < NUM_SEGMENTS + 1 ; seg++ )
			{
				f32 x0 = r0 * sinf( seg * fDeltaSegAngle );
				f32 y0 = r0 * cosf( seg * fDeltaSegAngle );

				mash::MashVertexStandard::sMashVertexStandard newVertex;
				newVertex.position = mash::MashVector3( -x0 * fRadius, -y0 * fRadius, -z0 * fRadius );
				newVertex.normal = newVertex.position;
				newVertex.normal.Normalize();
				newVertex.position += translation;
				newVertex.texCoord = mash::MashVector2((f32) seg / (f32) NUM_SEGMENTS, (f32) ring / (f32) NUM_RINGS);
				vertices.Append(&newVertex, sizeof(mash::MashVertexStandard::sMashVertexStandard));

				// add two indices except for last ring 
				if ( ring != NUM_RINGS ) 
				{
					indices.Append(&uiVertexIndex, sizeof(uint32));
					uint32 index = uiVertexIndex + (int32)(NUM_SEGMENTS + 1) ; 
					indices.Append(&index, sizeof(uint32));
					uiVertexIndex++;
				}; 
			}; // end for seg 
		} // end for ring 

		mash::MashVertexStandard primitiveVertexType;
		sMesh primitiveMesh;
		primitiveMesh.boneIndexArray = 0;
		primitiveMesh.boneWeightArray = 0;
		primitiveMesh.currentVertexElements = primitiveVertexType.m_elements;
		primitiveMesh.currentVertexElementCount = sizeof(primitiveVertexType.m_elements) / sizeof(sMashVertexElement);
		primitiveMesh.indexCount = iIndexCount;
		primitiveMesh.indexFormat = aFORMAT_R32_UINT;
		primitiveMesh.indices = (uint8*)indices.Pointer();
		primitiveMesh.vertexCount = iVertexCount;
		primitiveMesh.vertices = (uint8*)vertices.Pointer();
		primitiveMesh.primitiveType = aPRIMITIVE_TRIANGLE_STRIP;

		uint32 flags = aMESH_UPDATE_FILL_MESH | aMESH_UPDATE_CHANGE_VERTEX_FORMAT;
		if (UpdateMeshEx(mesh, &primitiveMesh, pVertexFormat, flags) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Sphere mesh creation failed",
					"CMashMeshBuilder::CreateSphere");

			return aMASH_FAILED;
		}

		mash::MashAABB boundingBox;
		if (CalculateBoundingBox(mesh->GetVertexDeclaration(), mesh->GetRawVertices().Pointer(), mesh->GetVertexCount(), boundingBox) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
				"Failed to create bounding box for mesh.",
				"CMashColladaLoader::CreateSphere");

			return aMASH_FAILED;
		}

		mesh->SetBoundingBox(boundingBox);

		return aMASH_OK;
	}

	eMASH_STATUS CMashMeshBuilder::CreateCube(MashMesh *mesh, 
		uint32 iWidth, 
		uint32 iHeight, 
		uint32 iDepth, 
		MashVertex *pVertexFormat,
		const mash::MashVector3 &facing,
		const mash::MashVector3 &translation)
	{
		if (!pVertexFormat)
			return aMASH_FAILED;

		mash::sMashColour colour = mash::sMashColour(255,255,255,255);
		f32 halfWidth = iWidth * 0.5f;
		f32 halfHeight = iHeight * 0.5f;
		f32 halfDepth = iDepth * 0.5f;

		uint32 indices[36] = {
			0, 1, 2, 
			3, 4, 5, 
			6, 7, 8, 
			9, 10, 11, 
			12, 13,14, 
			15, 16, 17, 
			18, 19, 20, 
			21, 22, 23, 
			24, 25, 26, 
			27, 28, 29, 
			30, 31, 32, 
			33, 34, 35
		};

		mash::MashVector3 vertPos[8] = {
			mash::MashVector3(-halfWidth,-halfHeight,-halfDepth),
			mash::MashVector3(-halfWidth,halfHeight,-halfDepth),
			mash::MashVector3(halfWidth,halfHeight,-halfDepth),
			mash::MashVector3(halfWidth,-halfHeight,-halfDepth),
			mash::MashVector3(-halfWidth,-halfHeight,halfDepth),
			mash::MashVector3(halfWidth,-halfHeight,halfDepth),
			mash::MashVector3(halfWidth,halfHeight,halfDepth),
			mash::MashVector3(-halfWidth,halfHeight,halfDepth)
		};

		mash::MashQuaternion rotateTo;
		rotateTo.RotateTo(mash::MashVector3(0.0f, 0.0f, -1.0f), facing);

		for(uint32 i = 0; i < 8; ++i)
		{
			vertPos[i] = rotateTo.TransformVector(vertPos[i]);
			vertPos[i] += translation;
		}

		mash::MashVector3 normals[6] = {
			mash::MashVector3(0.0f,0.0f,-1.0f),
			mash::MashVector3(0.0f,-0.0f,1.0f),
			mash::MashVector3(0.0f,-1.0f,-0.0f),
			mash::MashVector3(1.0f,0.0f,-0.0f),
			mash::MashVector3(0.0f,1.0f,0.0f),
			mash::MashVector3(-1.0f,0.0f,-0.0f)
		};

		for(uint32 i = 0; i < 6; ++i)
		{
			normals[i] = rotateTo.TransformVector(normals[i]);
			normals[i].Normalize();
		}

		mash::MashVector2 texCoord[4] = {
			mash::MashVector2(1.0f,0.0f),
			mash::MashVector2(1.0f,1.0f),
			mash::MashVector2(0.0f,1.0f),
			mash::MashVector2(0.0f,0.0f)
		};

		mash::MashVertexStandard::sMashVertexStandard vertices[36] = {
			mash::MashVertexStandard::sMashVertexStandard(vertPos[0], normals[0], texCoord[0]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[1], normals[0], texCoord[1]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[2], normals[0], texCoord[2]),
																		
			mash::MashVertexStandard::sMashVertexStandard(vertPos[2], normals[0], texCoord[2]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[3], normals[0], texCoord[3]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[0], normals[0], texCoord[0]),
																		
			mash::MashVertexStandard::sMashVertexStandard(vertPos[4], normals[1], texCoord[3]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[5], normals[1], texCoord[0]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[6], normals[1], texCoord[1]),
																		
			mash::MashVertexStandard::sMashVertexStandard(vertPos[6], normals[1], texCoord[1]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[7], normals[1], texCoord[2]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[4], normals[1], texCoord[3]),
																		
			mash::MashVertexStandard::sMashVertexStandard(vertPos[0], normals[2], texCoord[3]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[3], normals[2], texCoord[0]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[5], normals[2], texCoord[1]),
																		
			mash::MashVertexStandard::sMashVertexStandard(vertPos[5], normals[2], texCoord[1]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[4], normals[2], texCoord[2]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[0], normals[2], texCoord[3]),
																		
			mash::MashVertexStandard::sMashVertexStandard(vertPos[3], normals[3], texCoord[3]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[2], normals[3], texCoord[0]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[6], normals[3], texCoord[1]),
																		
			mash::MashVertexStandard::sMashVertexStandard(vertPos[6], normals[3], texCoord[1]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[5], normals[3], texCoord[2]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[3], normals[3], texCoord[3]),
																		
			mash::MashVertexStandard::sMashVertexStandard(vertPos[2], normals[4], texCoord[3]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[1], normals[4], texCoord[0]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[7], normals[4], texCoord[1]),
																		
			mash::MashVertexStandard::sMashVertexStandard(vertPos[7], normals[4], texCoord[1]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[6], normals[4], texCoord[2]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[2], normals[4], texCoord[3]),
																		
			mash::MashVertexStandard::sMashVertexStandard(vertPos[1], normals[5], texCoord[3]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[0], normals[5], texCoord[0]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[4], normals[5], texCoord[1]),
																		
			mash::MashVertexStandard::sMashVertexStandard(vertPos[4], normals[5], texCoord[1]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[7], normals[5], texCoord[2]),
			mash::MashVertexStandard::sMashVertexStandard(vertPos[1], normals[5], texCoord[3])
		};

		mash::MashVertexStandard primitiveVertexType;
		sMesh cubeMesh;
		cubeMesh.boneIndexArray = 0;
		cubeMesh.boneWeightArray = 0;
		cubeMesh.currentVertexElements = primitiveVertexType.m_elements;
		cubeMesh.currentVertexElementCount = sizeof(primitiveVertexType.m_elements) / sizeof(sMashVertexElement);
		cubeMesh.indexCount = 36;
		cubeMesh.indexFormat = aFORMAT_R32_UINT;
		cubeMesh.indices = (uint8*)indices;
		cubeMesh.vertexCount = 36;
		cubeMesh.vertices = (uint8*)vertices;
		cubeMesh.primitiveType = aPRIMITIVE_TRIANGLE_LIST;

		uint32 flags = aMESH_UPDATE_FILL_MESH | aMESH_UPDATE_CHANGE_VERTEX_FORMAT;
		if (UpdateMeshEx(mesh, &cubeMesh, pVertexFormat, flags) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Cube mesh creation failed",
					"CMashMeshBuilder::CreateCube");

			return aMASH_FAILED;
		}

		mash::MashAABB boundingBox;
		if (CalculateBoundingBox(mesh->GetVertexDeclaration(), mesh->GetRawVertices().Pointer(), mesh->GetVertexCount(), boundingBox) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
				"Failed to create bounding box for mesh.",
				"CMashColladaLoader::CreateCube");

			return aMASH_FAILED;
		}

		mesh->SetBoundingBox(boundingBox);
		return aMASH_OK;
	}

	eMASH_STATUS CMashMeshBuilder::CalculateBoundingBox(const MashVertex *pVertexDecl, 
		const void *pVertexList,
		uint32 iVertexCount,
		mash::MashAABB &boundingBox)const
	{
		if (!pVertexDecl || !pVertexList || (iVertexCount == 0))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to generate bounding box",
				"CMashMeshBuilder::CalculateBoundingBox");

			return aMASH_FAILED;
		}

		if (!pVertexDecl->Contains(mash::aDECLUSAGE_POSITION))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Vertex structure does not contain position data",
				"CMashMeshBuilder::CalculateBoundingBox");

			return aMASH_FAILED;
		}

		boundingBox = mash::MashAABB();

		uint8 *pVertices = (uint8*)pVertexList;

		const uint32 iVertexSizeInBytes = pVertexDecl->GetStreamSizeInBytes(0);

		int32 iPositionStride = -1;

		//cache some data
		const sMashVertexElement *pVertexElements = pVertexDecl->GetVertexElements();
		const uint32 iNumElements = pVertexDecl->GetVertexElementCount();

		for(int32 iElement = 0; iElement < iNumElements; ++iElement)
		{
			if (pVertexElements[iElement].usage == mash::aDECLUSAGE_POSITION)
			{
				iPositionStride = pVertexElements[iElement].stride;
				break;
			}
		}

		for(uint32 i = 0; i < iVertexCount; ++i)
		{
			//get position data
			mash::MashVector3 vPosition;
			memcpy(vPosition.v, &pVertices[(i * iVertexSizeInBytes) + iPositionStride], sizeof(mash::MashVector3));

			boundingBox.Add(vPosition);
		}

		boundingBox.Repair();

		return aMASH_OK;
	}

	void CMashMeshBuilder::GenerateNormalsIfNeeded(const mash::MashGenericArray &vertices,
			uint32 vertexCount,
			const MashArray<uint32> &indices,
			const MashVertex *vertex,
			MashArray<mash::MashVector3> &normalsOut)const
	{
		uint8 *vertexData = (uint8*)vertices.Pointer();
		uint32 vertexStreamSize = vertex->GetStreamSizeInBytes(0);
		uint32 elmLocation = 0;
		uint32 elmSize = 0;
		if (GetVertexElementData(vertex, aDECLUSAGE_NORMAL, sizeof(mash::MashVector3), elmLocation, elmSize))
		{
			for(uint32 i = 0; i < vertexCount; ++i)
			{
				const mash::MashVector3 *normalData = (mash::MashVector3*)&vertexData[(i * vertexStreamSize) + elmLocation];
				normalsOut.PushBack(*normalData);
			}
		}
		else
		{
			uint32 indexCount = indices.Size();
			uint32 triangleCount = indexCount / 3;

			MashArray<mash::MashVector3> newNormals;
			newNormals.Resize(vertexCount, mash::MashVector3());

			uint32 posElmLocation = 0;
			uint32 posElmSize = 0;
			GetVertexElementData(vertex, aDECLUSAGE_POSITION, sizeof(mash::MashVector3), posElmLocation, posElmSize);

			mash::MashVector3 normal(0.0f, 0.0f, 0.0f);
			uint32 index0, index1, index2;
			for(uint32 triangle = 0; triangle < triangleCount; ++triangle)
			{
				index0 = indices[(triangle * 3)];
				index1 = indices[(triangle * 3) + 1];
				index2 = indices[(triangle * 3) + 2];

				mash::MashVector3 *pos0 = (mash::MashVector3*)&vertexData[(index0 * vertexStreamSize) + posElmLocation];
				mash::MashVector3 *pos1 = (mash::MashVector3*)&vertexData[(index1 * vertexStreamSize) + posElmLocation];
				mash::MashVector3 *pos2 = (mash::MashVector3*)&vertexData[(index2 * vertexStreamSize) + posElmLocation];

				mash::MashVector3 e1 = *pos1 - *pos0;
				mash::MashVector3 e2 = *pos2 - *pos0;

				normal = e1.Cross(e2);

				normalsOut[index0] += normal;
				normalsOut[index1] += normal;
				normalsOut[index2] += normal;
			}

			for(uint32 i = 0; i < vertexCount; ++i)
				normalsOut[i].Normalize();
		}
	}

	void CMashMeshBuilder::UpdateNormals(const uint8 *vertices,
			uint32 vertexCount,
			const MashArray<uint32> &indices,
			const MashVertex *vertex)const
	{
		uint32 elmLocation = 0;
		uint32 elmSize = 0;
		if (GetVertexElementData(vertex, aDECLUSAGE_NORMAL, sizeof(mash::MashVector3), elmLocation, elmSize))
		{
			uint32 vertexStreamSize = vertex->GetStreamSizeInBytes(0);
			uint32 indexCount = indices.Size();
			uint32 triangleCount = indexCount / 3;

			MashArray<mash::MashVector3> newNormals;
			newNormals.Resize(vertexCount, mash::MashVector3());

			uint8 *vertexData = (uint8*)vertices;

			uint32 posElmLocation = 0;
			uint32 posElmSize = 0;
			GetVertexElementData(vertex, aDECLUSAGE_POSITION, sizeof(mash::MashVector3), posElmLocation, posElmSize);

			mash::MashVector3 normal(0.0f, 0.0f, 0.0f);
			uint32 index0, index1, index2;
			for(uint32 triangle = 0; triangle < triangleCount; ++triangle)
			{
				index0 = indices[(triangle * 3)];
				index1 = indices[(triangle * 3) + 1];
				index2 = indices[(triangle * 3) + 2];

				mash::MashVector3 *pos0 = (mash::MashVector3*)&vertexData[(index0 * vertexStreamSize) + posElmLocation];
				mash::MashVector3 *pos1 = (mash::MashVector3*)&vertexData[(index1 * vertexStreamSize) + posElmLocation];
				mash::MashVector3 *pos2 = (mash::MashVector3*)&vertexData[(index2 * vertexStreamSize) + posElmLocation];

				mash::MashVector3 e1 = *pos1 - *pos0;
				mash::MashVector3 e2 = *pos2 - *pos0;

				normal = e1.Cross(e2);

				newNormals[index0] += normal;
				newNormals[index1] += normal;
				newNormals[index2] += normal;
			}

			for(uint32 i = 0; i < vertexCount; ++i)
				newNormals[i].Normalize();
			
			for(uint32 i = 0; i < vertexCount; ++i)
			{
				uint8 *normalData = (uint8*)&vertexData[(i * vertexStreamSize) + elmLocation];
				memcpy(normalData, newNormals[i].v, elmSize);
			}
		}
	}

	void CMashMeshBuilder::UpdateTangents(const uint8 *vertices,
			uint32 vertexCount,
			const MashArray<uint32> &indices,
			const MashVertex *vertex)const
	{
		uint32 normalElmLocation = 0;
		uint32 normalElmSize = 0;
		uint32 uvElmLocation = 0;
		uint32 uvElmSize = 0;
		uint32 posElmLocation = 0;
		uint32 posElmSize = 0;
		uint32 tangentElmLocation = 0;
		uint32 tangentElmSize = 0;
		GetVertexElementData(vertex, aDECLUSAGE_POSITION, sizeof(mash::MashVector3), posElmLocation, posElmSize);
		bool hasTangents = GetVertexElementData(vertex, aDECLUSAGE_TANGENT, sizeof(mash::MashVector3), tangentElmLocation, tangentElmSize);
		bool hasNormals = GetVertexElementData(vertex, aDECLUSAGE_NORMAL, sizeof(mash::MashVector3), normalElmLocation, normalElmSize);
		bool hasUvs = GetVertexElementData(vertex, aDECLUSAGE_TEXCOORD, sizeof(mash::MashVector2), uvElmLocation, uvElmSize);

		if (hasTangents && hasNormals && hasUvs)
		{
			uint32 vertexStreamSize = vertex->GetStreamSizeInBytes(0);
			uint32 indexCount = indices.Size();
			uint32 triangleCount = indexCount / 3;

			MashArray<mash::MashVector3> newTangents;
			newTangents.Resize(vertexCount, mash::MashVector3());

			uint8 *vertexData = (uint8*)vertices;

			mash::MashVector3 tangent(0.0f, 0.0f, 0.0f);
			uint32 index0, index1, index2;
			for(uint32 triangle = 0; triangle < triangleCount; ++triangle)
			{
				index0 = indices[(triangle * 3)];
				index1 = indices[(triangle * 3) + 1];
				index2 = indices[(triangle * 3) + 2];

				mash::MashVector3 *pos0 = (mash::MashVector3*)&vertexData[(index0 * vertexStreamSize) + posElmLocation];
				mash::MashVector3 *pos1 = (mash::MashVector3*)&vertexData[(index1 * vertexStreamSize) + posElmLocation];
				mash::MashVector3 *pos2 = (mash::MashVector3*)&vertexData[(index2 * vertexStreamSize) + posElmLocation];

				mash::MashVector3 e1 = *pos1 - *pos0;
				mash::MashVector3 e2 = *pos2 - *pos0;

				mash::MashVector2 *uvsp0 = (mash::MashVector2*)&vertexData[(index0 * vertexStreamSize) + uvElmLocation];
				mash::MashVector2 *uvsp1 = (mash::MashVector2*)&vertexData[(index1 * vertexStreamSize) + uvElmLocation];
				mash::MashVector2 *uvsp2 = (mash::MashVector2*)&vertexData[(index2 * vertexStreamSize) + uvElmLocation];

				mash::MashVector2 uv1 = *uvsp1 - *uvsp0;
				mash::MashVector2 uv2 = *uvsp2 - *uvsp0;

				f32 d = uv1.y * uv2.x - uv1.x * uv2.y;

				if (d != 0.0f)
				{
					f32 div = 1.0f / d;
					tangent = (e1 * -uv2.y + e2 * uv1.y) * div;
					newTangents[index0] += tangent;
					newTangents[index1] += tangent;
					newTangents[index2] += tangent;
				}
			}

			for(uint32 i = 0; i < vertexCount; ++i)
			{
				/*
					tbn vectors may not be orthogonal at this point. So we use
					Gram-Schmidt orthogonalization to fit it.
				*/
				mash::MashVector3 *normal = (mash::MashVector3*)&vertexData[(i * vertexStreamSize) + normalElmLocation];

				newTangents[i] -= (*normal) * newTangents[i].Dot(*normal);
				newTangents[i].Normalize();
			}

			for(uint32 i = 0; i < vertexCount; ++i)
			{
				uint8 *data = (uint8*)&vertexData[(i * vertexStreamSize) + tangentElmLocation];
				memcpy(data, newTangents[i].v, tangentElmSize);
			}
		}
	}

	uint32 CMashMeshBuilder::MeshConvHashingFunction(const MashVertexMeshConversion::sMashVertexMeshConversion &item)
	{
		//take the floating point into account for numbers less than 1
		return ((uint32)(item.position.x * 98625731) ^ (uint32)(item.position.y * 10313717) ^ (uint32)(item.position.z * 77606537));
	}

	void CMashMeshBuilder::WeldVertices(const MashVertexMeshConversion::sMashVertexMeshConversion *vertices,
			const mash::MashVector4 *boneWeights,
			const mash::MashVector4 *boneIndices,
			uint32 vertexCount, 
			const MashArray<uint32> &indices,
			f32 smoothNormalMergeTolerance,
			MashVertexMeshConversion::sMashVertexMeshConversion *mergedVerticesOut,
			mash::MashVector4 *mergedBoneWeightsOut,
			mash::MashVector4 *mergedBoneIndicesOut,
			uint32 &mergedVertexCountOut,
			MashArray<uint32> &mergedIndicesOut)const
	{
		/*
			TODO : This function still has alot of collisions in the hash table.
			Try to improve on it.
		*/

		const uint32 hashTableSize = vertexCount * 3;

		bool copyBones = false;
		if (mergedBoneWeightsOut && mergedBoneIndicesOut)
			copyBones = true;

		CMashIndexingHashTable<MashVertexMeshConversion::sMashVertexMeshConversion, sMeshVertCmp> indexingHashTable(vertexCount, MeshConvHashingFunction, sMeshVertCmp());
		mergedIndicesOut.Resize(vertexCount);

		mergedVertexCountOut = 0;

		for(uint32 i = 0; i < vertexCount; ++i)
		{
			uint32 createdIndexValue = 0;
			uint32 foundIndexValue = indexingHashTable.Add(vertices[i], mergedVerticesOut, createdIndexValue);
			if (foundIndexValue == mash::math::MaxUInt32())
			{
				mergedVerticesOut[mergedVertexCountOut] = vertices[i];

				if (copyBones)
				{
					mergedBoneIndicesOut[mergedVertexCountOut] = boneIndices[i];
					mergedBoneWeightsOut[mergedVertexCountOut] = boneWeights[i];
				}

				++mergedVertexCountOut;
			}

			mergedIndicesOut[i] = createdIndexValue;
		}
	}

	uint32 CMashMeshBuilder::PositionHashingFunction(const mash::MashVector3 &item)
	{
		//take the floating point into account for numbers less than 1
		return ((uint32)(item.x * 98625731) ^ (uint32)(item.y * 10313717) ^ (uint32)(item.z * 77606537));
	}

	void CMashMeshBuilder::ExtractUniquePositions(const mash::MashGenericArray &vertices,
			uint32 vertexCount, 
			const MashArray<uint32> &indices,
			const MashVertex *vertex,
			MashArray<mash::MashVector3> &positionsOut,
			MashArray<uint32> &indicesOut)const
	{
		uint32 geometryStream = 0;
		uint32 positionLocation = 0;
		uint32 positionSize = 0;
		const sMashVertexElement *vertexElements = vertex->GetVertexElements();
		for(uint32 i = 0; i < vertex->GetVertexElementCount(); ++i)
		{
			if (vertexElements[i].stream != geometryStream)
				continue;

			if (vertexElements[i].usage == aDECLUSAGE_POSITION)
			{
				//find the position location within the vertex array
				positionLocation = vertexElements[i].stride;
				//make sure we dont access bad memory
				positionSize = math::Min<uint32>(sizeof(mash::MashVector3), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
			}
		}

		if (positionSize == 0)
			return;

		CMashIndexingHashTable<mash::MashVector3> indexingHashTable(vertexCount, PositionHashingFunction);

		const uint8 *vertexData = (const uint8*)vertices.Pointer();
		const uint32 indexCount = indices.Size();

		indicesOut.Resize(indexCount);
		uint32 vertexSize = vertex->GetStreamSizeInBytes(0);
		mash::MashVector3 position;

		for(uint32 i = 0; i < indexCount; ++i)
		{
			uint32 currentIndex = indices[i];

			memcpy(position.v, &vertexData[(currentIndex * vertexSize) + positionLocation], positionSize);

			uint32 createdIndexValue = 0;
			uint32 foundIndexValue = indexingHashTable.Add(position, positionsOut.Pointer(), createdIndexValue);
			if (foundIndexValue == mash::math::MaxUInt32())
				positionsOut.PushBack(position);

			indicesOut[i] = createdIndexValue;
		}

	}

	bool CMashMeshBuilder::GetVertexElementData(const mash::MashVertex *vertex, eVERTEX_DECLUSAGE elm, uint32 maxSize, uint32 &location, uint32 &size)const
	{
		const uint32 geometryStream = 0;
		const sMashVertexElement *vertexElements = vertex->GetVertexElements();
		for(uint32 i = 0; i < vertex->GetVertexElementCount(); ++i)
		{
			if (vertexElements[i].stream != geometryStream)
				continue;

			if (vertexElements[i].usage == elm)
			{
				//find the position location within the vertex array
				location = vertexElements[i].stride;
				//make sure we dont access bad memory
				size = math::Min<uint32>(maxSize, mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
				return true;
			}
		}

		return false;
	}

	void CMashMeshBuilder::GenerateIndicesIfNeeded(const uint8 *indices, 
			uint32 indexCount,
			eFORMAT indexFormat,
			uint32 vertexCount,
			MashArray<uint32> &indicesOut)const
	{
		if (indexCount && indices)
		{
			uint16 indexElementSize = 2;
			switch(indexFormat)
			{
			case aFORMAT_R16_UINT:
				indexElementSize = 2;
				break;
			case aFORMAT_R32_UINT:
				indexElementSize = 4;
				break;
			default:
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Invalid index format",
						"CMashMeshBuilder::GenerateIndicesIfNeeded");

					return;	
				}
			};

			indicesOut.Resize(indexCount);

			for(uint32 i = 0; i < indexCount; ++i)
			{
				uint32 index = 0;
				memcpy(&index, &indices[i * indexElementSize], indexElementSize);

				indicesOut[i] = index;
			}
		}
		else
		{
			if (vertexCount > 0)
			{
				indicesOut.Resize(vertexCount);

				for(uint32 i = 0; i < vertexCount; ++i)
					indicesOut[i] = i;
			}
		}
	}

	eMASH_STATUS CMashMeshBuilder::UpdateMesh(MashMesh *mesh, uint32 flags, MashVertex *toVertex, f32 weldVertexNormalTolerance)const
	{
		sMesh newMesh;
		newMesh.vertices = (uint8*)mesh->GetRawVertices().Pointer();
		newMesh.vertexCount = mesh->GetVertexCount();
		newMesh.indices = (uint8*)mesh->GetRawIndices().Pointer();
		newMesh.indexCount = mesh->GetIndexCount();
		newMesh.indexFormat = mesh->GetIndexFormat();
		newMesh.boneWeightArray = mesh->GetBoneWeights();
		newMesh.boneIndexArray = mesh->GetBoneIndices();
		newMesh.currentVertexElements = mesh->GetVertexDeclaration()->GetVertexElements();
		newMesh.currentVertexElementCount = mesh->GetVertexDeclaration()->GetVertexElementCount();
		newMesh.primitiveType = mesh->GetPrimitiveType();

		if (!toVertex)
			toVertex = mesh->GetVertexDeclaration();

		//clear the empty mesh flag if it has been set
		flags &= ~aMESH_UPDATE_FILL_MESH;

		return UpdateMeshEx(mesh, &newMesh, toVertex, flags, weldVertexNormalTolerance);
	}

	eMASH_STATUS CMashMeshBuilder::UpdateMeshEx(MashMesh *destinationMesh, sMesh *mesh, MashVertex *toVertex, uint32 flags, f32 weldVertexNormalTolerance)const
	{
		/*
			Early outs
		*/
		if (!mesh->vertices || (mesh->vertexCount == 0))
			return aMASH_OK;

		if (flags == 0)
			return aMASH_OK;

		if (!toVertex)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"A destination vertex type must be stated.",
				"CMashMeshBuilder::UpdateMesh");

			return aMASH_FAILED;
		}

		//tests to see if the user only wants to change the vertex format. Early outs might exist.
		uint32 flagsWithoutChangeVertexFormat = flags & ~(1UL << aMESH_UPDATE_CHANGE_VERTEX_FORMAT);
		if ((flags & aMESH_UPDATE_CHANGE_VERTEX_FORMAT) && (flagsWithoutChangeVertexFormat == 0))
		{
			/*
				Compare vertex types for early out.
				First compare the whole vertex declaration.
			*/
			if (toVertex->IsEqual(mesh->currentVertexElements, mesh->currentVertexElementCount))
				return aMASH_OK;

			/*
				At this point the vertex decls are not exactly the same. However, the geometry stream
				only maybe the same.
			*/
			if (toVertex->IsEqual(mesh->currentVertexElements, mesh->currentVertexElementCount, 0))
			{
				destinationMesh->_SetVertexDeclaration(toVertex);
				return aMASH_OK;
			}
		}

		MashArray<uint32> generatedIndices;
		uint8 *generatedVertices = 0;
		bool vertexDataGenerated = false;
		uint32 generatedVertexCount = 0;
		eFORMAT generatedIndexFormat = aFORMAT_R32_UINT;
		const sMashVertexElement *generatedVertexElements = mesh->currentVertexElements;
		uint32 generatedVertexElementCount = mesh->currentVertexElementCount;

		mash::MashVector4 *generatedBoneWeights = 0;
		mash::MashVector4 *generatedBoneIndices = 0;
		bool boneDataGenerated = false;

		GenerateIndicesIfNeeded(mesh->indices,
			mesh->indexCount,
			mesh->indexFormat,
			mesh->vertexCount,
			generatedIndices);

		if (generatedIndices.Empty())
		{
			return aMASH_FAILED;
		}

		bool updateNeeded = false;

		uint32 oldMeshVertexStride = 0;
		const uint32 geometryStream = 0;
		for(uint32 i = 0; i < mesh->currentVertexElementCount; ++i)
		{
			if (mesh->currentVertexElements[i].stream != geometryStream)
				continue;

			oldMeshVertexStride += mash::helpers::GetVertexTypeSize(mesh->currentVertexElements[i].type);
		}

		//created so we have valid memory to the internal element array
		MashVertexMeshConversion meshConversionType;

		if (flags & aMESH_UPDATE_WELD)
		{
			MashArray<uint32> weldedIndices;

			MashVertexMeshConversion::sMashVertexMeshConversion *weldVertices = MASH_ALLOC_T_COMMON(MashVertexMeshConversion::sMashVertexMeshConversion, mesh->vertexCount);
			
			struct sVertexDataLocation
			{
				uint32 location;
				uint32 sizeInBytes;

				sVertexDataLocation():location(0), sizeInBytes(0){}
			};

			sVertexDataLocation vertexDataLocations[aVERTEX_DECLUSAGE_COUNT];

			const sMashVertexElement *vertexElements = mesh->currentVertexElements;
			for(uint32 i = 0; i < mesh->currentVertexElementCount; ++i)
			{
				if (vertexElements[i].stream != geometryStream)
					continue;

				if (vertexElements[i].usage == aDECLUSAGE_POSITION)
				{
					//find the position location within the vertex array
					vertexDataLocations[aDECLUSAGE_POSITION].location = vertexElements[i].stride;
					//make sure we dont access bad memory
					vertexDataLocations[aDECLUSAGE_POSITION].sizeInBytes = math::Min<uint32>(sizeof(mash::MashVector3), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
				}
				else if (vertexElements[i].usage == aDECLUSAGE_NORMAL)
				{
					vertexDataLocations[aDECLUSAGE_NORMAL].location = vertexElements[i].stride;
					vertexDataLocations[aDECLUSAGE_NORMAL].sizeInBytes = math::Min<uint32>(sizeof(mash::MashVector3), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
				}
				else if (vertexElements[i].usage == aDECLUSAGE_TANGENT)
				{
					vertexDataLocations[aDECLUSAGE_TANGENT].location = vertexElements[i].stride;
					vertexDataLocations[aDECLUSAGE_TANGENT].sizeInBytes = math::Min<uint32>(sizeof(mash::MashVector3), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
				}
				else if (vertexElements[i].usage == aDECLUSAGE_TEXCOORD)
				{
					vertexDataLocations[aDECLUSAGE_TEXCOORD].location = vertexElements[i].stride;
					vertexDataLocations[aDECLUSAGE_TEXCOORD].sizeInBytes = math::Min<uint32>(sizeof(mash::MashVector2), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
				}
				else if (vertexElements[i].usage == aDECLUSAGE_COLOUR)
				{
					vertexDataLocations[aDECLUSAGE_COLOUR].location = vertexElements[i].stride;
					vertexDataLocations[aDECLUSAGE_COLOUR].sizeInBytes = math::Min<uint32>(sizeof(mash::sMashColour), mash::helpers::GetVertexDeclTypeSize(vertexElements[i].type));
				}
			}

			for(uint32 i = 0; i < mesh->vertexCount; ++i)
			{
				weldVertices[i].position.Zero();
				weldVertices[i].normal.Zero();
				weldVertices[i].tangent.Zero();
				weldVertices[i].texCoord.Zero();
				weldVertices[i].colour = 0;

				if (vertexDataLocations[aDECLUSAGE_POSITION].sizeInBytes > 0)
					memcpy(weldVertices[i].position.v, &mesh->vertices[(i * oldMeshVertexStride) + vertexDataLocations[aDECLUSAGE_POSITION].location], vertexDataLocations[aDECLUSAGE_POSITION].sizeInBytes);
				if (vertexDataLocations[aDECLUSAGE_NORMAL].sizeInBytes > 0)
					memcpy(weldVertices[i].normal.v, &mesh->vertices[(i * oldMeshVertexStride) + vertexDataLocations[aDECLUSAGE_NORMAL].location], vertexDataLocations[aDECLUSAGE_NORMAL].sizeInBytes);
				if (vertexDataLocations[aDECLUSAGE_TANGENT].sizeInBytes > 0)
					memcpy(weldVertices[i].tangent.v, &mesh->vertices[(i * oldMeshVertexStride) + vertexDataLocations[aDECLUSAGE_TANGENT].location], vertexDataLocations[aDECLUSAGE_TANGENT].sizeInBytes);
				if (vertexDataLocations[aDECLUSAGE_TEXCOORD].sizeInBytes > 0)
					memcpy(weldVertices[i].texCoord.v, &mesh->vertices[(i * oldMeshVertexStride) + vertexDataLocations[aDECLUSAGE_TEXCOORD].location], vertexDataLocations[aDECLUSAGE_TEXCOORD].sizeInBytes);
				if (vertexDataLocations[aDECLUSAGE_COLOUR].sizeInBytes > 0)
					memcpy(&weldVertices[i].colour.colour, &mesh->vertices[(i * oldMeshVertexStride) + vertexDataLocations[aDECLUSAGE_COLOUR].location], vertexDataLocations[aDECLUSAGE_COLOUR].sizeInBytes);
			}

			if (mesh->boneIndexArray && mesh->boneWeightArray)
			{
				generatedBoneWeights = MASH_ALLOC_T_COMMON(mash::MashVector4, mesh->vertexCount);
				generatedBoneIndices = MASH_ALLOC_T_COMMON(mash::MashVector4, mesh->vertexCount);
				boneDataGenerated = true;
			}

			generatedVertices = (uint8*)MASH_ALLOC_COMMON(mesh->vertexCount * sizeof(MashVertexMeshConversion::sMashVertexMeshConversion));
			vertexDataGenerated = true;

			//updates both the vertices and indices
			WeldVertices(weldVertices,
				mesh->boneWeightArray,
				mesh->boneIndexArray,
				mesh->vertexCount,
				generatedIndices,
				weldVertexNormalTolerance,
				(MashVertexMeshConversion::sMashVertexMeshConversion*)generatedVertices,
				generatedBoneWeights,
				generatedBoneIndices,
				generatedVertexCount,
				weldedIndices);

			//indices have been updated
			generatedIndices = weldedIndices;

			generatedVertexElements = meshConversionType.m_elements;
			generatedVertexElementCount = sizeof(meshConversionType.m_elements) / sizeof(sMashVertexElement);
			
			if (weldVertices)
				MASH_FREE(weldVertices);

			//we use the change format function to change the vertex array back into the correct order
			flags |= aMESH_UPDATE_CHANGE_VERTEX_FORMAT;

			updateNeeded = true;
		}
		else
		{
			//copy over the vertex data
			generatedVertices = (uint8*)mesh->vertices;
			generatedVertexCount = mesh->vertexCount;

			generatedBoneWeights = (mash::MashVector4*)mesh->boneWeightArray;
			generatedBoneIndices = (mash::MashVector4*)mesh->boneIndexArray;
		}

		if (flags & aMESH_UPDATE_CHANGE_VERTEX_FORMAT)
		{
			uint8 *newVertices = 0;
			//This function can add to the flags param
			if (ChangeVertexFormat(generatedVertices, 
				generatedVertexCount, 
				generatedBoneWeights,
				generatedBoneIndices,
				generatedVertexElements,
				generatedVertexElementCount,
				toVertex, flags, &newVertices) == aMASH_FAILED)
			{
				return aMASH_FAILED;
			}

			if (vertexDataGenerated)
				MASH_FREE(generatedVertices);

			generatedVertices = newVertices;
			vertexDataGenerated = true;
			
			updateNeeded = true;
		}

		if (flags & aMESH_UPDATE_NORMALS)
		{
			UpdateNormals(generatedVertices, generatedVertexCount, generatedIndices, toVertex);
			updateNeeded = true;
		}

		if (flags & aMESH_UPDATE_TANGENTS)
		{
			UpdateTangents(generatedVertices, generatedVertexCount, generatedIndices, toVertex);
			updateNeeded = true;
		}

		/*
			We convert the indices to 16bit if the flag is set, or if
			the mesh was originally set to 16bit
		*/
		bool updateIndicesTo16Bit = (flags & aMESH_UPDATE_16BIT_INDICES) || (mesh->indexFormat == aFORMAT_R16_UINT);

		if ((flags & aMESH_UPDATE_FILL_MESH) || updateNeeded || updateIndicesTo16Bit)
		{
			void *indices = 0;
			uint16 *shortIndices = 0;
			if (updateIndicesTo16Bit)
			{
				uint32 indexCount = generatedIndices.Size();

				if (indexCount > 65535)
				{
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
							"Failed to convert a mesh into 16bit indices. The mesh contains more than 65535 indices.",
							"CMashMeshBuilder::UpdateMesh");

					indices = &generatedIndices[0];
				}
				else
				{
					shortIndices = MASH_ALLOC_T_COMMON(uint16, indexCount);
					for(uint32 i = 0; i < indexCount; ++i)
						shortIndices[i] = generatedIndices[i];

					indices = shortIndices;

					generatedIndexFormat = aFORMAT_R16_UINT;
				}
			}
			else
			{
				indices = &generatedIndices[0];
			}

			uint8 *newSafeVertices = generatedVertices;
			bool safeVerticesGenerated = false;
			
			/*
				If new memory hasnt been allocated for the vertices then create another memory slot
				for the vertex data. This will prevent bad memory reads from occuring if the user
				has passed in a pointer to the vertex data within the mesh, this memory will become
				corrupt when we set new geometry.

				We assume if the destination mesh has no vertices set, then its already safe.
			*/
			if (!vertexDataGenerated && !destinationMesh->GetRawVertices().Empty())
			{
				newSafeVertices = (uint8*)MASH_ALLOC_COMMON(toVertex->GetStreamSizeInBytes(0) * generatedVertexCount);
				memcpy(newSafeVertices, generatedVertices, toVertex->GetStreamSizeInBytes(0) * generatedVertexCount);
				safeVerticesGenerated = true;
			}

			if (destinationMesh->SetGeometry(newSafeVertices, generatedVertexCount, toVertex, indices, generatedIndices.Size(), 
					generatedIndexFormat, mesh->primitiveType, mash::helpers::GetPrimitiveCount(mesh->primitiveType, generatedIndices.Size()), false) == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
							"Failed to set geometry",
							"CMashMeshBuilder::UpdateMesh");

				return aMASH_FAILED;
			}

			/*
				Only delete the vertex data if it was generated during this update. Else it's
				user data and they are responsible for deleting it.
			*/
			if (vertexDataGenerated || safeVerticesGenerated)
				MASH_FREE(newSafeVertices);

			if (shortIndices)
				MASH_FREE(shortIndices);

			if (boneDataGenerated)
			{
				destinationMesh->SetBoneIndices(generatedBoneIndices, generatedVertexCount);
				MASH_FREE(generatedBoneIndices);

				destinationMesh->SetBoneWeights(generatedBoneWeights, generatedVertexCount);
				MASH_FREE(generatedBoneWeights);
			}
			else if (flags & aMESH_UPDATE_FILL_MESH)
			{
				if (mesh->boneIndexArray && mesh->boneWeightArray)
				{
					/*
						If new memory hasnt been allocated for the vertices then create another memory slot
						for the vertex data. This will prevent bad memory reads from occuring if the user
						has passed in a pointer to the vertex data within the mesh, this memory will become
						corrupt when we set new geometry.
					*/
					mash::MashVector4 *safeBoneBuffer = 0;

					//Only alloc if needed. Else use the user provided array.
					if (destinationMesh->GetBoneIndices() || destinationMesh->GetBoneWeights())
						safeBoneBuffer = (mash::MashVector4*)MASH_ALLOC_COMMON(sizeof(mash::MashVector4) * mesh->vertexCount);

					if (destinationMesh->GetBoneIndices() == 0)
						destinationMesh->SetBoneIndices(mesh->boneIndexArray, mesh->vertexCount);
					else
					{
						memcpy(safeBoneBuffer, mesh->boneIndexArray, sizeof(mash::MashVector4) * mesh->vertexCount);
						destinationMesh->SetBoneIndices(safeBoneBuffer, mesh->vertexCount);
					}

					if (destinationMesh->GetBoneWeights() == 0)
						destinationMesh->SetBoneWeights(mesh->boneWeightArray, mesh->vertexCount);
					else
					{
						memcpy(safeBoneBuffer, mesh->boneWeightArray, sizeof(mash::MashVector4) * mesh->vertexCount);
						destinationMesh->SetBoneWeights(safeBoneBuffer, mesh->vertexCount);
					}

					if (safeBoneBuffer)
						MASH_FREE(safeBoneBuffer);
				}
			}
		}

		return aMASH_OK;
	}

	eMASH_STATUS CMashMeshBuilder::ChangeVertexFormat(const uint8 *vertices, 
			uint32 vertexCount,
			const mash::MashVector4 *boneWeights,
			const mash::MashVector4 *boneIndices,
			const sMashVertexElement *currentVertexElements,
			uint32 currentVertexElementCount,
			const MashVertex *pToVertx,
			uint32 &flagsOut,
			uint8 **generatedVertices)const
	{
		/*
			It is assumed early outs have been explored before entering this function.
		*/

		const uint32 geometryStream = 0;

		if (currentVertexElementCount == 0)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Vertex element pointer contains 0 elements.",
						"CMashMeshBuilder::ChangeVertexFormat");

			return aMASH_FAILED;
		}

		if (!currentVertexElements)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Vertex element pointer is invalid.",
						"CMashMeshBuilder::ChangeVertexFormat");

			return aMASH_FAILED;
		}

		if (!pToVertx)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Invalid vertex pointer.",
						"CMashMeshBuilder::ChangeVertexFormat");

			return aMASH_FAILED;
		}

		const sMashVertexElement *pToVertexDecl = pToVertx->GetVertexElements();
		uint32 iToElementCount = pToVertx->GetVertexElementCount();

		if (vertexCount == 0 || !vertices)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
						"Change vertex format failed. The mesh does not contain any raw vertex data.\
												  This may have been deleted after MainLoop::Inititalise. If you really want to \
												  change the vertex format during normal application updates then set \
												  Mesh::SetSaveInitialiseDataFlags before MainLoop::Inititalise",
						"CMashMeshBuilder::ChangeVertexFormat");

			return aMASH_FAILED;
		}

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
						"Mesh vertex format conversion started",
						"CMashMeshBuilder::ChangeVertexFormat");

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
						"Mesh vertex format changing to support new vertex format. Performance will be affected. This should be done offline where possible",
						"CMashMeshBuilder::ChangeVertexFormat");

		//get vertex sizes
		uint32 iOrigVertexSizeInBytes = 0;
		for(uint32 i = 0; i < currentVertexElementCount; ++i)
		{
			if (currentVertexElements[i].stream != geometryStream)
				continue;

			iOrigVertexSizeInBytes += mash::helpers::GetVertexDeclTypeSize(currentVertexElements[i].type);
		}

		uint32 iDestVertexSizeInBytes = 0;
		for(uint32 i = 0; i < iToElementCount; ++i)
		{
			if (pToVertexDecl[i].stream != geometryStream)
				continue;

			iDestVertexSizeInBytes += mash::helpers::GetVertexDeclTypeSize(pToVertexDecl[i].type);
		}

		uint32 newVertexBufferSizeInBytes = iDestVertexSizeInBytes * vertexCount;
		*generatedVertices = (uint8*)MASH_ALLOC_COMMON(newVertexBufferSizeInBytes);
		uint8 *newVertices = *generatedVertices;

		for(uint32 iVertex = 0; iVertex < vertexCount; ++iVertex)
		{
			for(uint32 iToVertexElm = 0; iToVertexElm < iToElementCount; ++iToVertexElm)
			{
				//make sure we are only dealing with the geometry stream
				if (pToVertexDecl[iToVertexElm].stream != geometryStream)
					continue;

				/*
					First check if we are adding bone data.
					This can simply be copied directly from the mesh.
				*/
				if (pToVertexDecl[iToVertexElm].usage == aDECLUSAGE_BLENDWEIGHT)
				{
					if (boneWeights)
					{
						memcpy(&newVertices[(iVertex * iDestVertexSizeInBytes) + pToVertexDecl[iToVertexElm].stride],
							boneWeights[iVertex].v, sizeof(mash::MashVector4));
					}
				}
				else if (pToVertexDecl[iToVertexElm].usage == aDECLUSAGE_BLENDINDICES)
				{
					if (boneIndices)
					{
						memcpy(&newVertices[(iVertex * iDestVertexSizeInBytes) + pToVertexDecl[iToVertexElm].stride],
							boneIndices[iVertex].v, sizeof(mash::MashVector4));
					}
				}
				else
				{
					/*
						Something other than bone data is being copied over. So see if it exisits
						in the old data and copy it over if it does.
					*/
					for(uint32 iCurrentVertexElm = 0; iCurrentVertexElm < currentVertexElementCount; ++iCurrentVertexElm)
					{
						//make sure we are only dealing with the geometry stream
						if (currentVertexElements[iCurrentVertexElm].stream != geometryStream)
							continue;

						if ((currentVertexElements[iCurrentVertexElm].stream == pToVertexDecl[iToVertexElm].stream) &&
							(currentVertexElements[iCurrentVertexElm].usage == pToVertexDecl[iToVertexElm].usage) &&
							(currentVertexElements[iCurrentVertexElm].usageIndex == pToVertexDecl[iToVertexElm].usageIndex))
						{
							f32 minCopySize = math::Min<uint32>(mash::helpers::GetVertexTypeSize(pToVertexDecl[iToVertexElm].type), mash::helpers::GetVertexTypeSize(currentVertexElements[iCurrentVertexElm].type));
							memcpy(&newVertices[(iVertex * iDestVertexSizeInBytes) + pToVertexDecl[iToVertexElm].stride],
									&vertices[(iVertex * iOrigVertexSizeInBytes) + currentVertexElements[iCurrentVertexElm].stride],
									minCopySize);
						}
					}
				}
			}
		}

		bool nowHasTangents = false;
		bool nowHasNormals = false;
		for(uint32 i = 0; i < iToElementCount; ++i)
		{
			if (pToVertexDecl[i].stream != geometryStream)
				continue;

			if (pToVertexDecl[i].usage == aDECLUSAGE_TANGENT)
			{
				nowHasTangents = true;
			}
			else if (pToVertexDecl[i].usage == aDECLUSAGE_NORMAL)
			{
				nowHasNormals = true;
			}
		}

		bool hadTangents = false;
		bool hadNormals = false;
		for(uint32 i = 0; i < currentVertexElementCount; ++i)
		{
			if (currentVertexElements[i].stream != geometryStream)
				continue;

			if (currentVertexElements[i].usage == aDECLUSAGE_TANGENT)
				hadTangents = true;
			else if (currentVertexElements[i].usage == aDECLUSAGE_NORMAL)
				hadNormals = true;
		}

		if (nowHasNormals && !hadNormals)
			flagsOut |= aMESH_UPDATE_NORMALS;
		if (nowHasTangents && !hadTangents)
			flagsOut |= aMESH_UPDATE_TANGENTS;

		MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
						"Vertex format conversion succeeded",
						"CMashMeshBuilder::ChangeVertexFormat");

		return aMASH_OK;
	}
}