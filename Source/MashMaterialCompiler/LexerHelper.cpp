//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "LexerHelper.h"
#include "MashGenericScriptReader.h"
#include <string.h>
#include <cstdlib>
#include "MashMemoryPoolLexer.h"
#include "Material.h"
namespace mash
{
	extern sShaderCompilerData *g_shaderCompilerData;

	extern int g_lineno;

	float StringToNumber(char *text)
	{
		return atof(text);
	}
	   

	void EatBlockComment(int (*pFuncPointer)(void))  
	{
		char c;

		bool bPartFound = false;
	   while ((c = pFuncPointer()) != 0)
	   {
			if (c == '\n')
			{
				g_lineno++;
			}
			else if (c == '*')
			{
				bPartFound = true;
			}
			else if (bPartFound)
			{
				if ((c == '/'))
				{
					break;
				}
				else
				{
					bPartFound = false;
				}
			}
	   }
		
	}

	// The comment-skipping function: skip to end-of-line
	void EatComment(int (*pFuncPointer)(void))  {
		char c;

	   while ((c = pFuncPointer()) != '\n' && c != 0);
		g_lineno++;
	}

	char* StringConstant(char *text)  {
	   int l = strlen(text)-2;
	   char *str = (char*)g_shaderCompilerData->g_materialLexerMemoryPool->GetMemory(l+1);
	   strncpy (str, &text[1], l); str[l] = 0;
	   return str;
	   }

	mash::eLIGHTTYPE GetLightTypeFromString(char *text)
	{
		if (mash::scriptreader::CompareStrings(text, "directional"))
			return mash::aLIGHT_DIRECTIONAL;
		if (mash::scriptreader::CompareStrings(text, "spot"))
			return mash::aLIGHT_SPOT;
		if (mash::scriptreader::CompareStrings(text, "point"))
			return mash::aLIGHT_POINT;

		return mash::aLIGHT_TYPE_COUNT;
	}
}