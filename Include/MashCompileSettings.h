//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_DEVICE_COMPILE_CONFIG_H_
#define _MASH_DEVICE_COMPILE_CONFIG_H_

#include <assert.h>

#if defined (WIN32) || defined (WIN64)
#define MASH_WINDOWS
#elif defined (__APPLE__)
#define MASH_APPLE
#else
#define MASH_LINUX
#include <cstddef>
#endif

/*
	Debug values for memory, bounds, etc... checking.
*/
#if defined (MASH_DEBUG) || defined (_DEBUG) || defined (debug)

#ifndef MASH_DEBUG
#define MASH_DEBUG
#endif

#define MASH_LOG_ENABLED
#define MASH_MEMORY_TRACKING_ENABLED

#define MASH_ASSERT(val) assert(val)
#define MASH_DEBUG_IF(condtion, body) if (condtion){body}
#else
#define MASH_ASSERT(val)
#define MASH_DEBUG_IF(condtion, body)
#endif

#ifdef MASH_DLL
	#ifdef MASH_EXPORTS
	#define _MASH_EXPORT __declspec(dllexport)
	#else
	#define _MASH_EXPORT __declspec(dllimport)
	#endif
#else
	#define _MASH_EXPORT
#endif

#if defined(MASH_DEBUG) && defined (MASH_WINDOWS)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

#define _MASH_ABGR_COLOUR_FORMAT_



#endif
