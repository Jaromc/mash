//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_D3D10_MESH_BUFFER_H_
#define _C_MASH_D3D10_MESH_BUFFER_H_

#include "MashDataTypes.h"

#ifdef MASH_WINDOWS
#include "MashMeshBufferIntermediate.h"

namespace mash
{
	class CMashD3D10MeshBuffer : public MashMeshBufferIntermediate
	{
	public:
		CMashD3D10MeshBuffer(MashVideo *renderer,
			MashArray<MashVertexBuffer*> &vertexBuffers, 
			MashIndexBuffer *indexBuffer,
			MashVertex *vertexDeclaration):MashMeshBufferIntermediate(renderer,
			vertexBuffers, indexBuffer, vertexDeclaration){}

		CMashD3D10MeshBuffer(MashVideo *renderer):MashMeshBufferIntermediate(renderer){}

		MashMeshBuffer* Clone();

		~CMashD3D10MeshBuffer(){}
	};
}

#endif

#endif