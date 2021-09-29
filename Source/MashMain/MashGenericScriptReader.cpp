//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashGenericScriptReader.h"
#include <cstring>
#include <cctype>
namespace mash
{
	namespace scriptreader
	{
		bool CompareStrings(const int8 *sA, const int8 *sB)
		{
			if (!sA || !sB)
				return false;

			uint32 i = 0;
			while((sA[i] != '\0') && (sB[i] != '\0'))
			{
				if (toupper(sA[i]) != toupper(sB[i]))
					return false;

				++i;
			}

			if ((sA[i] == '\0') && (sB[i] == '\0'))
				return true;

			return false;
		}

		void EatComment(const int8 *fileData, uint32 &location, uint32 end)
		{
			while((location < end) && (fileData[location] != '\n' && fileData[location] != '\0'))
				++location;
		}

		void EatBlockComment(const int8 *fileData, uint32 &location, uint32 end)
		{
			bool partFound = false;
			while (location < end)
			{
				int8 c = fileData[location++];

				if (c == '*')
				{
					partFound = true;
				}
				else if (partFound)
				{
					if ((c == '/'))
					{
						break;
					}
					else
					{
						partFound = false;
					}
				}
			}
		}

		uint32 ReadNextChar(const int8 *fileData, uint32 &location, uint32 end, int8 &out)
		{
			uint32 returnVal = 0;
			while(!returnVal && (location < end))
			{
				out = fileData[location++];

				if (out == '/')
				{
					if (location < end)
					{
						if (fileData[location] == '/')
							EatComment(fileData, location, end);
						else if (fileData[location] == '*')
							EatBlockComment(fileData, location, end);
						else
							returnVal = 1;
					}
				}
				else
					returnVal = 1;
			}

			return returnVal;
		}

		eMASH_STATUS ReadNextBlockLimits(const int8 *fileData, uint32 &location, uint32 end, 
			uint32 &startOut, uint32 &endOut)
		{
			int32 depth = 0;
			startOut = location;
			int8 c;
			while(location < end)
			{
				if (ReadNextChar(fileData, location, end, c))
				{
					if (c == '}')
					{
						if (depth == 0)
						{
							//error
							break;
						}
						else if (depth == 1)
						{
							endOut = location-1;
							return aMASH_OK;
						}
						else
						{
							--depth;
						}
					}
					else if (c == '{')
					{
						if (depth == 0)
							startOut = location;

						++depth;
					}
				}
			}

			//error, bracket mismatch
			return aMASH_FAILED;
		}
	}
}