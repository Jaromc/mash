//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_FUNCTOR_H_
#define _MASH_FUNCTOR_H_

#include "MashReferenceCounter.h"

namespace mash
{
	//! Used by MashEventFunctor.
	template<typename TData>
	class MashFunctorBase : public MashReferenceCounter
	{
	public:
		MashFunctorBase():MashReferenceCounter(){}
		virtual void Call(TData &inputEvent) = 0;
	};

	template<typename T, typename TData>
	class MashFunctorImp : public MashFunctorBase<TData>
	{
	public:
		typedef void(T::*functPtr)(TData &inputEvent);
	private:
		functPtr m_ptr;
		T *m_obj;
	public:
		MashFunctorImp():m_ptr(0), m_obj(0){}
		MashFunctorImp(functPtr ptr, T *obj):m_ptr(ptr), m_obj(obj){}
		MashFunctorImp(const T &copy):m_ptr(copy.m_ptr), m_obj(copy.m_obj){}

		void Call(TData &inputEvent)
		{
			(*m_obj.*m_ptr)(inputEvent);
		}
	};

	template<typename TData>
	class MashFunctor
	{
	private:
		MashFunctorBase<TData> *m_funcImp;
	public:
		MashFunctor():m_funcImp(0){}

		~MashFunctor()
		{
			Destroy();
		}

		template<typename T>
		MashFunctor(void (T::*functPtr)(TData &), T *obj):m_funcImp(0)
		{
			if (functPtr && obj)
				m_funcImp = MASH_NEW_COMMON MashFunctorImp<T, TData>(functPtr, obj);
		}

		MashFunctor(const MashFunctor &copy):m_funcImp(0)
		{
			if (copy.m_funcImp)
			{
				copy.m_funcImp->Grab();
				m_funcImp = copy.m_funcImp;
			}
		}

		MashFunctor& operator=(const MashFunctor &copy)
		{
			if (copy.m_funcImp)
			{
				copy.m_funcImp->Grab();

				if (m_funcImp)
					m_funcImp->Drop();

				m_funcImp = copy.m_funcImp;
			}
			return *this;
		}

		void Destroy()
		{
			if (m_funcImp)
				m_funcImp->Drop();

			m_funcImp = 0;
		}

		void Call(TData &inputEvent)
		{
			if (m_funcImp)
				m_funcImp->Call(inputEvent);
		}

		bool IsValid()const
		{
			return (m_funcImp)?true:false;
		}
	};
}

#endif