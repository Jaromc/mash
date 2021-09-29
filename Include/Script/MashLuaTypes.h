//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_LUA_TYPES_H_
#define _MASH_LUA_TYPES_H_

namespace mash
{
	class MashLuaScript;

	/*!
		Creates a custom function for use in lua and sets the function pointer
		that will be called.
	*/
	struct sMashLuaUserFunction
	{
		//! Name of this function from within a lua script.
		char *callingString;
		
		/*!
			This function must return the number of objects pushed to the stack
			using a function such as MashLuaScript::Pushxxx().
		*/
		int (*functPtr)(MashLuaScript *voidScript);
	};
}

#endif