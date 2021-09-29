//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _MASH_LUA_LIB_USER_H_
#define _MASH_LUA_LIB_USER_H_

#include "MashLuaTypesC.h"

 #ifdef __cplusplus
 extern "C" {
 #endif

 struct sMashLuaUserFunctionImpl
 {
	char *callingString;
	int functionId;
 };

extern struct sMashLuaUserFunctionImpl *g_aeroLuaUserFunctionList;
extern unsigned int g_aeroLuaUserFunctionCount;

#ifdef __cplusplus
 }
 #endif

#endif