//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _MASH_MEMORY_POOL_LEXER_H_
#define _MASH_MEMORY_POOL_LEXER_H_

#include "MashEnum.h"
#include "MashArray.h"

namespace mash
{
	/*
		Impliments a very simple memory pool to hold strings generated from the lexer.
	*/
	class MashMemoryPoolLexer
	{
	private:
		struct sPool
		{
			uint8 *memory;
			uint32 poolSize;
			uint32 nextAvaliableByte;
		};

		MashArray<sPool> m_memoryPools;
	public:
		MashMemoryPoolLexer(){}
		~MashMemoryPoolLexer();

		void* GetMemory(uint32 sizeInBytes);
	};
}

#endif