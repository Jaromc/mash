//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashD3D10MeshBuffer.h"

namespace mash
{
	MashMeshBuffer* CMashD3D10MeshBuffer::Clone()
	{
		return MashMeshBufferIntermediate::CloneMembers(this);
	}
}