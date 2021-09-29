//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OPENGL_HELPER_H_
#define _C_MASH_OPENGL_HELPER_H_

#include "MashEnum.h"
#include "CMashOpenGLHeader.h"

namespace mash
{
	GLenum MashToOpenGLCubeFace(eCUBEMAP_FACE face);
	GLenum MashToOpenGLDepthFunc(eDEPTH_COMPARISON cmp);
	GLenum MashToOpenGLCullMode(eCULL_MODE cullMode);
	GLenum MashToOpenGLFillMode(eFILL_MODE fillMode);
	GLenum MashToOpenGLBlendState(eBLEND blend);
	GLenum MashToOpenGLBlendOp(eBLENDOP blendOp);
	GLenum MashToOpenGLFormat(eFORMAT format);
	GLenum MashToOpenGLIndexFormat(eFORMAT format);
	uint32 MashToOpenGLClearFlags(uint32 flags);
	GLenum MashToOpenGLVertexDeclType(eVERTEX_DECLTYPE type);
	GLenum MashToOpenGLPrimitiveType(ePRIMITIVE_TYPE type);
	GLenum MashToOpenGLProgramType(ePROGRAM_TYPE type);
	void MashToOpenGLColourWriteMask(uint32 colourWriteMask, GLboolean &r, GLboolean &g, GLboolean &b, GLboolean &a);

	void MashToOpenGLTexFormat(eFORMAT inFormat,
		GLenum &internalFormat,
		GLenum &format,
		GLenum &type);

	void MashToOpenGLSamplerFilter(eFILTER filter, 
		GLenum &min, 
		GLenum &mag);

	GLenum MashToOpenGLSamplerAddress(eTEXTURE_ADDRESS mode);
}

#endif