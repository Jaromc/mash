//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_EVENT_CPP_FUNC_H_
#define _C_MASH_EVENT_CPP_FUNC_H_

#include "CMashEventHandler.h"

namespace mash
{
	template <class T>
	class CMashEventCPPFunc : public CMashEventHandler
	{
	private:
		T *m_pCallee;
		void (T::*m_funcPtr)(); 
	public:
		CMashEventCPPFunc(T *who, void (T::*funcPtr)()):CMashEventHandler(),
		  m_pCallee(who), m_funcPtr(funcPtr){}

		virtual ~CMashEventCPPFunc(){}

		virtual eMASH_STATUS RunCallback()
		{
			if (m_funcPtr && m_pCallee)
			{
				m_pCallee->*m_funcPtr();
				return aMASH_OK;
			}

			return aMASH_FAILED;
		};
	};
}

#endif