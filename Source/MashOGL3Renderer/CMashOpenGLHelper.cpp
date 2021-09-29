//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLHelper.h"

namespace mash
{
	GLenum MashToOpenGLCubeFace(eCUBEMAP_FACE face)
	{
		switch(face)
		{
		case aCUBEMAP_FACE_POS_X:
			return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		case aCUBEMAP_FACE_NEG_X:
			return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
		case aCUBEMAP_FACE_POS_Y:
			return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
		case aCUBEMAP_FACE_NEG_Y:
			return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
		case aCUBEMAP_FACE_POS_Z:
			return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
		case aCUBEMAP_FACE_NEG_Z:
			return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
		default:
			return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
		};
	}

	GLenum MashToOpenGLDepthFunc(eDEPTH_COMPARISON cmp)
	{
		switch(cmp)
		{
		case aZCMP_NEVER:
			return GL_NEVER;
		case aZCMP_LESS:
			return GL_LESS;
		case aZCMP_EQUAL:
			return GL_EQUAL;
		case aZCMP_LESS_EQUAL:
			return GL_LEQUAL;
		case aZCMP_GREATER:
			return GL_GREATER;
		case aZCMP_NOT_EQUAL:
			return GL_NOTEQUAL;
		case aZCMP_GREATER_EQUAL:
			return GL_GEQUAL;
		case aZCMP_ALWAYS:
			return GL_ALWAYS;
		default:
			return GL_LESS;
		};
	}

	GLenum MashToOpenGLCullMode(eCULL_MODE cullMode)
	{
		switch(cullMode)
		{
		case aCULL_CCW:
			return GL_BACK;
		case aCULL_NONE:
		case aCULL_CW:
			return GL_FRONT;
		default:
			return GL_FRONT;
		};
	}

	GLenum MashToOpenGLFillMode(eFILL_MODE fillMode)
	{
		switch(fillMode)
		{
		case aFILL_WIRE_FRAME:
			return GL_LINE;
		case aFILL_SOLID:
			return GL_FILL;
		default:
			return GL_FILL;
		};
	}

	GLenum MashToOpenGLBlendState(eBLEND blend)
	{
		switch(blend)
		{
		case aBLEND_SRC_ALPHA:
			return GL_SRC_ALPHA;
		case aBLEND_INV_SRC_ALPHA:
			return GL_ONE_MINUS_SRC_ALPHA;
		case aBLEND_DEST_ALPHA:
			return GL_DST_ALPHA;
		case aBLEND_DEST_COLOR:
			return GL_DST_COLOR;
		case aBLEND_INV_DEST_ALPHA:
			return GL_ONE_MINUS_DST_ALPHA;
		case aBLEND_INV_SRC_COLOR:
			return GL_ONE_MINUS_SRC_COLOR;
		case aBLEND_ONE:
			return GL_ONE;
		case aBLEND_SRC_ALPHA_SAT:
			return GL_SRC_ALPHA_SATURATE;
		case aBLEND_SRC_COLOR:
			return GL_SRC_COLOR;
		case aBLEND_ZERO:
			return GL_ZERO;
		case aBLEND_INV_DEST_COLOR:
			return GL_ONE_MINUS_DST_COLOR;
		default:
			return GL_ONE;
		};
	}

	GLenum MashToOpenGLBlendOp(eBLENDOP blendOp)
	{
		switch(blendOp)
		{
		case aBLENDOP_ADD:
			return GL_FUNC_ADD;
		case aBLENDOP_MAX:
			return GL_MAX;
		case aBLENDOP_MIN:
			return GL_MIN;
		case aBLENDOP_SUBTRACT:
			return GL_FUNC_SUBTRACT;
		case aBLENDOP_REV_SUBTRACT:
			return GL_FUNC_REVERSE_SUBTRACT;
		default:
			return GL_FUNC_ADD;
		};
	}

	GLenum MashToOpenGLFormat(eFORMAT format)
	{
		switch (format)
		{
		case aFORMAT_RGBA8_UINT:
			return GL_RGBA8UI;
		case aFORMAT_RGBA16_UINT:
			return GL_RGBA16UI;

		case aFORMAT_RGBA8_SINT:
			return GL_RGBA8I;
		case aFORMAT_RGBA16_SINT:
			return GL_RGBA16I;

		case aFORMAT_RGBA16_FLOAT:
			return GL_RGBA16F;
		case aFORMAT_RGBA32_FLOAT:
			return GL_RGBA32F;

		case aFORMAT_R8_UINT:
			return GL_R8UI;
		case aFORMAT_R16_UINT:
			return GL_R16UI;
		case aFORMAT_R32_UINT:
			return GL_R32UI;

		case aFORMAT_R16_FLOAT:
			return GL_R16F;
		case aFORMAT_R32_FLOAT:
			return GL_R32F;

		case aFORMAT_RG16_FLOAT:
			return GL_RG16F;
		case aFORMAT_RG32_FLOAT:
			return GL_RG32F;

		case aFORMAT_DEPTH32_FLOAT:
			return GL_DEPTH32F_STENCIL8;

		default:
			return GL_RGBA32F;
		};
	}

	GLenum MashToOpenGLIndexFormat(eFORMAT format)
	{
		switch (format)
		{
		case aFORMAT_R8_UINT:
			return GL_UNSIGNED_BYTE;
		case aFORMAT_R16_UINT:
			return GL_UNSIGNED_SHORT;
		case aFORMAT_R32_UINT:
			return GL_UNSIGNED_INT;
		default:
			return GL_UNSIGNED_INT;
		};
	}

	uint32 MashToOpenGLClearFlags(uint32 flags)
	{
		uint32 openGLFlags = 0;

		if (flags & aCLEAR_DEPTH)
			openGLFlags |= GL_DEPTH_BUFFER_BIT;
		if (flags & aCLEAR_STENCIL)
			openGLFlags |= GL_STENCIL_BUFFER_BIT;
		if (flags & aCLEAR_TARGET)
			openGLFlags |= GL_COLOR_BUFFER_BIT;

		return openGLFlags;
	}

	GLenum MashToOpenGLVertexDeclType(eVERTEX_DECLTYPE type)
	{
		switch(type)
		{
		case aDECLTYPE_R32_FLOAT:
			return GL_FLOAT;
		case aDECLTYPE_R32G32_FLOAT:
			return GL_FLOAT;
		case aDECLTYPE_R32G32B32_FLOAT:
			return GL_FLOAT;
		case aDECLTYPE_R32G32B32A32_FLOAT:
			return GL_FLOAT;
		case aDECLTYPE_R8G8B8A8_UNORM:
			return GL_UNSIGNED_BYTE;
		case aDECLTYPE_R8G8B8A8_UINT:
			return GL_UNSIGNED_INT;
		case aDECLTYPE_R16G16_SINT:
			return GL_SHORT;
		case aDECLTYPE_R16G16B16A16_SINT:
			return GL_SHORT;
		default: 
			return GL_FLOAT;
		};
	}

	GLenum MashToOpenGLPrimitiveType(ePRIMITIVE_TYPE type)
	{
		switch(type)
		{
		case aPRIMITIVE_LINE_STRIP:
			return GL_LINE_STRIP;
		case aPRIMITIVE_LINE_LIST:
			return GL_LINES;
		case aPRIMITIVE_TRIANGLE_STRIP:
			return GL_TRIANGLE_STRIP;
		case aPRIMITIVE_TRIANGLE_LIST:
			return GL_TRIANGLES;
		case aPRIMITIVE_POINT_LIST:
			return GL_POINTS;
		default:
			return GL_TRIANGLES;
		};
	}

	GLenum MashToOpenGLProgramType(ePROGRAM_TYPE type)
	{
		switch(type)
		{
		case aPROGRAM_VERTEX:
			return GL_VERTEX_SHADER;
		case aPROGRAM_PIXEL:
			return GL_FRAGMENT_SHADER;
		case aPROGRAM_GEOMETRY:
		default:
			return 0;
		};
	}

	void MashToOpenGLColourWriteMask(uint32 colourWriteMask, GLboolean &r, GLboolean &g, GLboolean &b, GLboolean &a)
	{
        r = GL_FALSE;
        g = GL_FALSE;
        b = GL_FALSE;
        a = GL_FALSE;
        
        if (colourWriteMask & aCOLOUR_WRITE_RED)
        {
            r = GL_TRUE;
        }
        if (colourWriteMask & aCOLOUR_WRITE_GREEN)
        {
            g = GL_TRUE;
        }
        if (colourWriteMask & aCOLOUR_WRITE_BLUE)
        {
            b = GL_TRUE;
        }
        if (colourWriteMask & aCOLOUR_WRITE_ALPHA)
        {
            a = GL_TRUE;
        }
	}

	void MashToOpenGLTexFormat(eFORMAT inFormat,
		GLenum &internalFormat,
		GLenum &format,
		GLenum &type)
	{
		switch (inFormat)
		{
		case aFORMAT_RGBA8_UINT:
			{
				internalFormat = GL_RGBA8;
				format = GL_RGBA;
				type = GL_UNSIGNED_BYTE;
				break;
			}
		case aFORMAT_RGBA16_UINT:
			{
				internalFormat = GL_RGBA16;
				format = GL_RGBA;
				type = GL_UNSIGNED_SHORT;
				break;
			}
		case aFORMAT_RGBA8_SINT:
			{
				internalFormat = GL_RGBA8;
				format = GL_RGBA;
				type = GL_BYTE;
				break;
			}
		case aFORMAT_RGBA16_SINT:
			{
				internalFormat = GL_RGBA16;
				format = GL_RGBA;
				type = GL_SHORT;
				break;
			}
		case aFORMAT_RGBA16_FLOAT:
			{
				internalFormat = GL_RGBA16F;
				format = GL_RGBA;
				type = GL_HALF_FLOAT;
				break;
			}
		case aFORMAT_RGBA32_FLOAT:
			{
				internalFormat = GL_RGBA32F;
				format = GL_RGBA;
				type = GL_FLOAT;
				break;
			}
		case aFORMAT_R8_UINT:
			{
				internalFormat = GL_R8UI;
				format = GL_RED;
				type = GL_UNSIGNED_BYTE;
				break;
			}
		case aFORMAT_R16_UINT:
			{
				internalFormat = GL_R16UI;
				format = GL_RED;
				type = GL_UNSIGNED_SHORT;
				break;
			}
		case aFORMAT_R32_UINT:
			{
				internalFormat = GL_R32UI;
				format = GL_RED;
				type = GL_UNSIGNED_INT;
				break;
			}
		case aFORMAT_R16_FLOAT:
			{
				internalFormat = GL_R16F;
				format = GL_RED;
				type = GL_HALF_FLOAT;
				break;
			}
		case aFORMAT_R32_FLOAT:
			{
				internalFormat = GL_R32F;
				format = GL_RED;
				type = GL_FLOAT;
				break;
			}
		case aFORMAT_DEPTH32_FLOAT:
			{
				internalFormat = GL_DEPTH_COMPONENT32F;
				format = GL_RED;
				type = GL_FLOAT;
				break;
			}
		default:
			{
				internalFormat = GL_RGBA;
				format = GL_RGBA;
				type = GL_FLOAT;
			}
		};
	}

	void MashToOpenGLSamplerFilter(eFILTER filter, 
		GLenum &min, 
		GLenum &mag)
	{
		switch(filter)
		{
		case aFILTER_MIN_MAG_MIP_POINT:
			{
				min = GL_NEAREST_MIPMAP_NEAREST;
				mag = GL_NEAREST;
				break;
			}
		case aFILTER_MIN_MAG_MIP_LINEAR:
			{
				min = GL_LINEAR_MIPMAP_LINEAR;
				mag = GL_LINEAR;
				break;
			}
		case aFILTER_MIN_MAG_POINT:
			{
				min = GL_NEAREST;
				mag = GL_NEAREST;
				break;
			}
		case aFILTER_MIN_MAG_LINEAR:
			{
				min = GL_LINEAR;
				mag = GL_LINEAR;
				break;
			}
		case aFILTER_MIN_MAG_MIP_ANISOTROPIC:
			{
				min = GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT;
				mag = GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT;
				break;
			}
		default:
			{
				min = GL_LINEAR;
				mag = GL_LINEAR_MIPMAP_LINEAR;
				break;
			}
		};
	}

	GLenum MashToOpenGLSamplerAddress(eTEXTURE_ADDRESS mode)
	{
		switch(mode)
		{
		case aTEXTURE_ADDRESS_WRAP:
			return GL_REPEAT;
		case aTEXTURE_ADDRESS_CLAMP:
			return GL_CLAMP_TO_EDGE;
		default:
			return GL_REPEAT;
		};
	}
}
