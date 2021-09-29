//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_SCENE_NODE_SCRIPT_HANDLER_H_
#define _MASH_SCENE_NODE_SCRIPT_HANDLER_H_

#include "MashSceneNodeCallback.h"

namespace mash
{
    /*!
        Allows scene nodes to be manipulated from a script.
     
        This class will call spacific lua function on scene node events.
    */
	class MashSceneNodeScriptHandler : public MashSceneNodeCallback
	{
	public:
		MashSceneNodeScriptHandler():MashSceneNodeCallback(){}
		virtual ~MashSceneNodeScriptHandler(){}
	};
}

#endif