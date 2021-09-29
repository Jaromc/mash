//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _LEXER_HELPER_H_
#define _LEXER_HELPER_H_

#include "MashEnum.h"
#include "MashGenericArray.h"

namespace mash
{
	float StringToNumber(char *text);
	void EatBlockComment(int (*pFuncPointer)(void));
	void EatComment(int (*pFuncPointer)(void));
	char* StringConstant(char *text);
	mash::eLIGHTTYPE GetLightTypeFromString(char *text);
}

#endif