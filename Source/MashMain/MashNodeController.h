//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _MASH_NODE_CONTROLLER_H_
#define _MASH_NODE_CONTROLLER_H_

#include "MashMemoryObject.h"
#include "MashSceneNode.h"

namespace mash
{
	class MashNodeControler : public MashMemoryObject
	{
	public:
		MashNodeControler(){}
		virtual ~MashNodeControler(){}

		/*
			Node world transformation is calculated AFTER this is called.
		*/
		virtual void Run(mash::MashSceneNode *pNode, f32 dt) = 0;
	};
}

#endif