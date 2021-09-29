//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OPENGL_MESH_BUFFER_H_
#define _C_MASH_OPENGL_MESH_BUFFER_H_

#include "MashMeshBufferIntermediate.h"
#include "CMashOpenGLHeader.h"
#include "MashMaterialDependentResource.h"

namespace mash
{
	class CMashOpenGLMeshBuffer :  public MashMaterialDependentResource<CMashOpenGLMeshBuffer>, public MashMeshBufferIntermediate
	{
	private:
		GLuint m_openGLvao;
		bool m_isCompiled;
	public:
		CMashOpenGLMeshBuffer(MashVideo *renderer,
			MashArray<MashVertexBuffer*> &vertexBuffers, 
			MashIndexBuffer *indexBuffer,
			MashVertex *vertexDeclaration);
        
        CMashOpenGLMeshBuffer(MashVideo *renderer);

		~CMashOpenGLMeshBuffer();

		MashMeshBuffer* Clone();
		GLuint GetOpenGLIndex()const;
		
		void _SetVertexDeclaration(MashVertex *vertex);
		eMASH_STATUS ResizeVertexBuffers(uint32 bufferIndex, uint32 bufferSize, eUSAGE usage, bool saveData);
		eMASH_STATUS ResizeIndexBuffer(uint32 bufferSize, bool saveData = false);
		eMASH_STATUS _CommitBufferChange();

		bool IsValid()const;
		void OnDependencyCompiled(MashVideo *renderer, MashMaterialDependentResourceBase *dependency);
	};

	inline bool CMashOpenGLMeshBuffer::IsValid()const
	{
		return m_isCompiled;
	}

	inline GLuint CMashOpenGLMeshBuffer::GetOpenGLIndex()const
	{
		return m_openGLvao;
	}
}

#endif