//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_EVENT_C_FUNC_H_
#define _C_MASH_EVENT_C_FUNC_H_

#include "CMashEventHandler.h"

namespace mash
{
	class CMashEventCFunc : public CMashEventHandler
	{
	private:
		void (*m_functPtr)();
	public:
		CMashEventCFunc(void (*functPtr)()):CMashEventHandler(), m_functPtr(functPtr) {}

		virtual ~CMashEventCFunc(){}

		virtual eMASH_STATUS RunCallback()
		{
			if (m_functPtr)
			{
				m_functPtr();

				return aMASH_OK;
			}

			return aMASH_FAILED;
		};
	};
}	

#endif