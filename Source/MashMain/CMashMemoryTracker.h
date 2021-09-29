//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_MEMORY_TRACKER_H_
#define _C_MASH_MEMORY_TRACKER_H_

#include <map>
#include <set>
#include <algorithm>
#include <string>
#include "MashDataTypes.h"

namespace mash
{
	class CMashMemoryTracker
	{
	private:
		struct sAllocation
		{
			/*
				Must use stl here otherwise we end up in an infinate loop
				when mash containers allocate memory
			*/
			void *iPointer;
			int32 iSize;
			std::string sFile;
			int32 iLineNumber;
			std::string sFunc;
		};
	private:
		std::map<void*, sAllocation> m_allocations;

		bool m_trackingEnabled;
		static CMashMemoryTracker *m_pInstance;
		
	protected:
		CMashMemoryTracker();
		virtual ~CMashMemoryTracker();

	public:
		static CMashMemoryTracker* Instance();
		static void DestroyInstance();

		void LogAllocation(void *p, int32 iSize, const int8 *sFile, int32 iLine, const int8 *sFunc);
		void LogDeallocation(void *p);
		void OutputMemoryLog();

		void EnableTracking(bool state);
	};

	inline void CMashMemoryTracker::EnableTracking(bool state)
	{
		m_trackingEnabled = state;
	}

}

#endif