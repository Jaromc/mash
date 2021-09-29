//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OPENGL_VERTEX_H_
#define _C_MASH_OPENGL_VERTEX_H_

#include "MashVertexIntermediate.h"
#include "CMashOpenGLHeader.h"
#include "MashMaterialDependentResource.h"
namespace mash
{
	struct sMashOpenGLVertexData
	{
		uint32 stream;
		GLuint attribute;
		GLint size;
		GLenum type;
		GLboolean normalized;
		GLsizei stride;
		uint32 stepRate;
		const GLvoid *pointer;
	};

	class CMashOpenGLVertex : public MashVertexIntermediate, public MashMaterialDependentResource<CMashOpenGLVertex>
	{
	private:
		MashArray<sMashOpenGLVertexData> m_openGLElements;
		bool m_isCompiled;
	public:
		CMashOpenGLVertex(const sMashVertexElement *pMashVertexDecl,
			uint32 iElementCount,
			uint32 iVertexSizeInBytes);
		~CMashOpenGLVertex();

		bool IsValid()const;
		void OnDependencyCompiled(MashVideo *renderer, MashMaterialDependentResourceBase *dependency);

		const MashArray<sMashOpenGLVertexData>& GetOpenGLVertexElements()const{return m_openGLElements;}
	};

	
	inline bool CMashOpenGLVertex::IsValid()const
	{
		return m_isCompiled;
	}
}

#endif