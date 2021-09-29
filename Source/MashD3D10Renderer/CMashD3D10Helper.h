//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_D3D10_HELPER_H_
#define _C_MASH_D3D10_HELPER_H_

#include "MashDataTypes.h"

#ifdef MASH_WINDOWS
#include <d3d10_1.h>
#include "MashEnum.h"

namespace mash
{
	D3D10_COMPARISON_FUNC MashToD3D10DepthFunc(eDEPTH_COMPARISON cmp);
	D3D10_CULL_MODE MashToD3D10CullMode(eCULL_MODE cullMode);
	D3D10_FILL_MODE MashToD3D10FillMode(eFILL_MODE fillMode);
	D3D10_BLEND MashToD3D10BlendState(eBLEND blend);
	D3D10_BLEND_OP MashToD3D10BlendOp(eBLENDOP blendOp);
	DXGI_FORMAT MashToD3D10VertexDeclType(eVERTEX_DECLTYPE type);
	const int8* MashVertexUsageToString(eVERTEX_DECLUSAGE usage);
	DXGI_FORMAT MashToD3D10Format(eFORMAT format);
	D3D10_USAGE MashToD3D10Usage(eUSAGE usage);
	uint32 MashToD3D10ClearFlags(uint32 flags);
	D3D10_PRIMITIVE_TOPOLOGY MashToD3D10Primitive(ePRIMITIVE_TYPE type);
	D3D10_MAP MashToD3D10Lock(eBUFFER_LOCK lockType);
	D3D10_COLOR_WRITE_ENABLE MashToD3D10ColourWriteMask(eCOLOUR_WRITE colourWriteMask);
	D3D10_INPUT_CLASSIFICATION MashToD3D10Classification(eVERTEX_CLASSIFICATION classification);

	D3D10_FILTER MashToD3D10SamplerFilter(eFILTER filter);
	D3D10_TEXTURE_ADDRESS_MODE MashToD3D10SamplerAddress(eTEXTURE_ADDRESS mode);
}
#endif

#endif