//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_LUA_TYPES_C_H_
#define _MASH_LUA_TYPES_C_H_

/*!
	See MashScriptManager file header for usage.
*/
struct sMashLuaKeyValue
{
	//! String version of eINPUT_KEY_MAP for use in lua
	char *keyString;
	//! Input action < aINPUTMAP_MAX
	int value;
};

#endif