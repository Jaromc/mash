//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_DUMMY_H_
#define _MASH_DUMMY_H_

#include "MashSceneNode.h"

namespace mash
{
    /*!
        Dummy nodes are simple scene nodes commonly used as attachment points
        or markers.
    */
	class MashDummy : public MashSceneNode
	{
	private:

	public:
		MashDummy(MashSceneNode *parent,
			MashSceneManager *manager,
			const MashStringc &name):MashSceneNode(parent, manager, name){}

		virtual ~MashDummy(){}

        //! Sets the local bounding box for this node.
		/*!
            \param bounds Local AABB.
         */
		virtual void SetBoundingBox(const MashAABB &bounds) = 0;
	};
}

#endif