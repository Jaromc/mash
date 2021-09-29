//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_STATIC_MESH_H_
#define _MASH_STATIC_MESH_H_

#include "MashMesh.h"

namespace mash
{
    /*!
        Base class for static meshes. This can be created from MashSceneManager::CreateStaticMesh().
    */
	class MashStaticMesh : public MashMesh
	{
	public:
		MashStaticMesh():MashMesh(){}
		virtual ~MashStaticMesh(){}
	};
}

#endif