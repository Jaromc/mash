//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_MEMORY_TYPES_H_
#define _MASH_MEMORY_TYPES_H_

#include "MashCompileSettings.h"

namespace mash
{
    /*!
        Common memory byte alignment.
    */
	const size_t _g_globalMemoryAlignment = 4;

	/*!
		These categories can be used to determine how memory might be used.
	*/
	enum eMEMORY_CATEGORY
	{
		/*!
			Most objects fall under this category. Objects defined as common 
			are usually average sized in memory and are alive for many frames.
			Example, scene nodes.
		*/
		aMEMORY_CATEGORY_COMMON,
		/*!
			This category is used for objects that will be alive for a long
			time (and are usually large).
			Example, textures, vertex and index buffers.
		*/
		aMEMORY_CATEGORY_LONG_TERM,
		/*!
			Any objects using this category are alive for only a frame, or
			within a functions scope.
		*/
		aMEMORY_CATEGORY_SHORT_TERM,
		/*!
			Objects here are likely to be alive for the entire
			application lifetime.
		*/
		aMEMORY_CATEGORY_SYSTEM
	};
}

#endif