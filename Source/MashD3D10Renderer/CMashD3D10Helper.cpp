//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashD3D10Helper.h"

namespace mash
{
	D3D10_COMPARISON_FUNC MashToD3D10DepthFunc(eDEPTH_COMPARISON cmp)
	{
		switch(cmp)
		{
		case aZCMP_NEVER:
			return D3D10_COMPARISON_NEVER;
		case aZCMP_LESS:
			return D3D10_COMPARISON_LESS;
		case aZCMP_EQUAL:
			return D3D10_COMPARISON_EQUAL;
		case aZCMP_LESS_EQUAL:
			return D3D10_COMPARISON_LESS_EQUAL;
		case aZCMP_GREATER:
			return D3D10_COMPARISON_GREATER;
		case aZCMP_NOT_EQUAL:
			return D3D10_COMPARISON_NOT_EQUAL;
		case aZCMP_GREATER_EQUAL:
			return D3D10_COMPARISON_GREATER_EQUAL;
		case aZCMP_ALWAYS:
			return D3D10_COMPARISON_ALWAYS;
		default:
			return D3D10_COMPARISON_LESS;
		};
	}

	D3D10_CULL_MODE MashToD3D10CullMode(eCULL_MODE cullMode)
	{
		switch(cullMode)
		{
		case aCULL_NONE:
			return D3D10_CULL_NONE;
		case aCULL_CW:
			return D3D10_CULL_FRONT;
		case aCULL_CCW:
			return D3D10_CULL_BACK;
		default:
			return D3D10_CULL_NONE;
		};
	}

	D3D10_FILL_MODE MashToD3D10FillMode(eFILL_MODE fillMode)
	{
		switch(fillMode)
		{
		case aFILL_WIRE_FRAME:
			return D3D10_FILL_WIREFRAME;
		case aFILL_SOLID:
			return D3D10_FILL_SOLID;
		default:
			return D3D10_FILL_SOLID;
		};
	}

	D3D10_BLEND MashToD3D10BlendState(eBLEND blend)
	{
		switch(blend)
		{
		case aBLEND_SRC_ALPHA:
			return D3D10_BLEND_SRC_ALPHA;
		case aBLEND_INV_SRC_ALPHA:
			return D3D10_BLEND_INV_SRC_ALPHA;
		case aBLEND_DEST_ALPHA:
			return D3D10_BLEND_DEST_ALPHA;
		case aBLEND_DEST_COLOR:
			return D3D10_BLEND_DEST_COLOR;
		case aBLEND_INV_DEST_ALPHA:
			return D3D10_BLEND_INV_DEST_ALPHA;
		case aBLEND_INV_SRC_COLOR:
			return D3D10_BLEND_INV_SRC_COLOR;
		case aBLEND_ONE:
			return D3D10_BLEND_ONE;
		case aBLEND_SRC_ALPHA_SAT:
			return D3D10_BLEND_SRC_ALPHA_SAT;
		case aBLEND_SRC_COLOR:
			return D3D10_BLEND_SRC_COLOR;
		case aBLEND_ZERO:
			return D3D10_BLEND_ZERO;
		case aBLEND_INV_DEST_COLOR:
			return D3D10_BLEND_INV_DEST_COLOR;
		default:
			return D3D10_BLEND_ONE;
		};
	}

	D3D10_BLEND_OP MashToD3D10BlendOp(eBLENDOP blendOp)
	{
		switch(blendOp)
		{
		case aBLENDOP_ADD:
			return D3D10_BLEND_OP_ADD;
		case aBLENDOP_MAX:
			return D3D10_BLEND_OP_MAX;
		case aBLENDOP_MIN:
			return D3D10_BLEND_OP_MIN;
		case aBLENDOP_SUBTRACT:
			return D3D10_BLEND_OP_SUBTRACT;
		case aBLENDOP_REV_SUBTRACT:
			return D3D10_BLEND_OP_REV_SUBTRACT;
		default:
			return D3D10_BLEND_OP_ADD;
		};
	}

	DXGI_FORMAT MashToD3D10VertexDeclType(eVERTEX_DECLTYPE type)
	{
		switch(type)
		{
		case aDECLTYPE_R32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;
		case aDECLTYPE_R32G32_FLOAT:
			return DXGI_FORMAT_R32G32_FLOAT;
		case aDECLTYPE_R32G32B32_FLOAT:
			return DXGI_FORMAT_R32G32B32_FLOAT;
		case aDECLTYPE_R32G32B32A32_FLOAT:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		case aDECLTYPE_R8G8B8A8_UNORM:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case aDECLTYPE_R8G8B8A8_UINT:
			return DXGI_FORMAT_R8G8B8A8_UINT;
		case aDECLTYPE_R16G16_SINT:
			return DXGI_FORMAT_R16G16_SINT;
		case aDECLTYPE_R16G16B16A16_SINT:
			return DXGI_FORMAT_R16G16B16A16_SINT;
		default: 
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		};
	}

	const int8* MashVertexUsageToString(eVERTEX_DECLUSAGE usage)
	{
		static const int8 *sPosition = "POSITION";
		static const int8 *sBlendWeight = "BLENDWEIGHT";
		static const int8 *sBlendIndices = "BLENDINDICES";
		static const int8 *sNormal = "NORMAL";
		static const int8 *sTexCoord = "TEXCOORD";
		static const int8 *sTangent = "TANGENT";
		static const int8 *sColour = "COLOR";
		static const int8 *sCustom = "CUSTOM";

		switch(usage)
		{
		case aDECLUSAGE_POSITION:
			return sPosition;
		case aDECLUSAGE_BLENDWEIGHT:
			return sBlendWeight;
		case aDECLUSAGE_BLENDINDICES:
			return sBlendIndices;
		case aDECLUSAGE_NORMAL:
			return sNormal;
		case aDECLUSAGE_TEXCOORD:
			return sTexCoord;
		case aDECLUSAGE_CUSTOM:
			return sCustom;
		case aDECLUSAGE_TANGENT:
			return sTangent;
		case aDECLUSAGE_COLOUR:
			return sColour;
		default: 
			return sTexCoord;
		};
	}
	
	DXGI_FORMAT MashToD3D10Format(eFORMAT format)
	{
		switch (format)
		{
		case aFORMAT_RGBA8_UINT:
			return DXGI_FORMAT_R8G8B8A8_UINT;
		case aFORMAT_RGBA16_UINT:
			return DXGI_FORMAT_R16G16B16A16_UINT;

		case aFORMAT_RGBA8_SINT:
			return DXGI_FORMAT_R8G8B8A8_SINT;
		case aFORMAT_RGBA16_SINT:
			return DXGI_FORMAT_R16G16B16A16_SINT;

		case aFORMAT_RGBA16_FLOAT:
			return DXGI_FORMAT_R16G16B16A16_FLOAT;
		case aFORMAT_RGBA32_FLOAT:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;

		case aFORMAT_R8_UINT:
			return DXGI_FORMAT_R8_UINT;
		case aFORMAT_R16_UINT:
			return DXGI_FORMAT_R16_UINT;
		case aFORMAT_R32_UINT:
			return DXGI_FORMAT_R32_UINT;

		case aFORMAT_R16_FLOAT:
			return DXGI_FORMAT_R16_FLOAT;
		case aFORMAT_R32_FLOAT:
			return DXGI_FORMAT_R32_FLOAT;

		case aFORMAT_RG16_FLOAT:
			return DXGI_FORMAT_R16G16_FLOAT;
		case aFORMAT_RG32_FLOAT:
			return DXGI_FORMAT_R32G32_FLOAT;

		case aFORMAT_DEPTH32_FLOAT:
			return DXGI_FORMAT_D32_FLOAT;

		default:
			return DXGI_FORMAT_R32G32B32A32_FLOAT;
		};
	}

	D3D10_USAGE MashToD3D10Usage(eUSAGE usage)
	{
		switch(usage)
		{
		case aUSAGE_STATIC:
			return D3D10_USAGE_DEFAULT;
		case aUSAGE_DYNAMIC:
			return D3D10_USAGE_DYNAMIC;
		default:
			return D3D10_USAGE_DEFAULT;
		};
	}

	uint32 MashToD3D10ClearFlags(uint32 flags)
	{
		uint32 iD3D10Value = 0;

		if (flags & aCLEAR_DEPTH)
			iD3D10Value |= D3D10_CLEAR_DEPTH;
		if (flags & aCLEAR_STENCIL)
			iD3D10Value |= D3D10_CLEAR_STENCIL;

		return iD3D10Value;
	}

	D3D10_PRIMITIVE_TOPOLOGY MashToD3D10Primitive(ePRIMITIVE_TYPE type)
	{
		switch(type)
		{
		case aPRIMITIVE_TRIANGLE_STRIP:
			return D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		case aPRIMITIVE_TRIANGLE_LIST:
			return D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case aPRIMITIVE_POINT_LIST:
			return D3D10_PRIMITIVE_TOPOLOGY_POINTLIST;
		case aPRIMITIVE_LINE_STRIP:
			return D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case aPRIMITIVE_LINE_LIST:
			return D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
		default:
			return D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		};
	}

	D3D10_MAP MashToD3D10Lock(eBUFFER_LOCK lockType)
	{
		switch(lockType)
		{
		case aLOCK_WRITE:
			return D3D10_MAP_WRITE;
		case aLOCK_WRITE_DISCARD:
			return D3D10_MAP_WRITE_DISCARD;
		case aLOCK_WRITE_NOOVERWRITE:
			return D3D10_MAP_WRITE_NO_OVERWRITE;
		default:
			return D3D10_MAP_WRITE;
		};	
	}

	D3D10_COLOR_WRITE_ENABLE MashToD3D10ColourWriteMask(eCOLOUR_WRITE colourWriteMask)
	{
		switch(colourWriteMask)
		{
		case aCOLOUR_WRITE_RED:
			return D3D10_COLOR_WRITE_ENABLE_RED;
		case aCOLOUR_WRITE_GREEN:
			return D3D10_COLOR_WRITE_ENABLE_GREEN;
		case aCOLOUR_WRITE_BLUE:
			return D3D10_COLOR_WRITE_ENABLE_BLUE;
		case aCOLOUR_WRITE_ALPHA:
			return D3D10_COLOR_WRITE_ENABLE_ALPHA;
		default:
			return D3D10_COLOR_WRITE_ENABLE_ALL;
		};
	}

	D3D10_INPUT_CLASSIFICATION MashToD3D10Classification(eVERTEX_CLASSIFICATION classification)
	{
		switch(classification)
		{
		case aCLASSIFICATION_INSTANCE_DATA:
			return D3D10_INPUT_PER_INSTANCE_DATA;
		default:
			return D3D10_INPUT_PER_VERTEX_DATA;
		};
	}

	D3D10_FILTER MashToD3D10SamplerFilter(eFILTER filter)
	{
		switch(filter)
		{
		case aFILTER_MIN_MAG_MIP_POINT:
			return D3D10_FILTER_MIN_MAG_MIP_POINT;
		case aFILTER_MIN_MAG_MIP_LINEAR:
			return D3D10_FILTER_MIN_MAG_MIP_LINEAR;
		case aFILTER_MIN_MAG_POINT:
			return D3D10_FILTER_MIN_MAG_MIP_POINT;
		case aFILTER_MIN_MAG_LINEAR:
			return D3D10_FILTER_MIN_MAG_MIP_LINEAR;
		case aFILTER_MIN_MAG_MIP_ANISOTROPIC:
			return D3D10_FILTER_ANISOTROPIC;
		default:
			return D3D10_FILTER_MIN_MAG_MIP_LINEAR;
		};
	}

	D3D10_TEXTURE_ADDRESS_MODE MashToD3D10SamplerAddress(eTEXTURE_ADDRESS mode)
	{
		switch(mode)
		{
		case aTEXTURE_ADDRESS_WRAP:
			return D3D10_TEXTURE_ADDRESS_WRAP;
		case aTEXTURE_ADDRESS_CLAMP:
			return D3D10_TEXTURE_ADDRESS_CLAMP;
		default:
			return D3D10_TEXTURE_ADDRESS_CLAMP;
		};
	}
}