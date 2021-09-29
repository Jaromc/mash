//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GENERIC_SCRIPT_READER_H_
#define _MASH_GENERIC_SCRIPT_READER_H_

#include "MashString.h"
#include "MashEnum.h"
#include <cctype>

namespace mash
{
    /*!
        Contains some methods for file or script reading.
    */
	namespace scriptreader
	{
        //! Compares two strings, doesn't consider case.
        /*!
			Input maybe NULL, and the result will be false.

            \param a First string.
            \param b Second string.
            \return True if they are equal, regardless of case.
        */
        bool CompareStrings(const int8 *a, const int8 *b);
        
        //! Returns the byte length of the next '{' '}' block of text.
        /*!
            The byte length starts from the first '{' up to and including its closing '}'.
         
            \param fileData Char array to read from.
            \param location Position in the array to start searching. This param will be counted forward and returned.
            \param end This is the max amount of chars to read.
            \param startOut Position of the first '{'. This index is from location.
            \param endOut Position of the closing '}'. This index is from location.
            \return Ok on success. Failed if there were any errors, such as mismatched brackets.
        */
		eMASH_STATUS ReadNextBlockLimits(const int8 *fileData, uint32 &location, uint32 end, 
                                        uint32 &startOut, uint32 &endOut);
        
        //! Reads the next int8.
        /*!
            Removes any c style commenting.
         
            \param fileData Char array to read from.
            \param location Position in the array to start searching. This param will be counted forward and returned.
            \param end This is the max amount of chars to read.
            \param out Next non space int8.
			\return 1 if a valid char was read. 0 otherwise.
        */
		uint32 ReadNextChar(const int8 *fileData, uint32 &location, uint32 end, int8 &out);
        
        //! Removes c style block comments. /*  */
        /*!
            \param fileData Char array to read from.
            \param location Position in the array to start searching. This param will be counted forward and returned.
            \param end This is the max amount of chars to read.
        */
		void EatBlockComment(const int8 *fileData, uint32 &location, uint32 end);
        
        //! Removes c style comments. '//'
        /*!
            \param fileData Char array to read from.
            \param location Position in the array to start searching. This param will be counted forward and returned.
            \param end This is the max amount of chars to read.
         */
		void EatComment(const int8 *fileData, uint32 &location, uint32 end);

		//! Reads the next line up to the new line char.
        /*!
            \param fileData Char array to read from.
            \param location Position in the array to start searching. This param will be counted forward and returned.
            \param out line read. 
            \return The line length.
        */
		template<class T, class TAlloc>
		uint32 ReadLine(const int8 *fileData, uint32 &location, uint32 end, MashBaseString<T, TAlloc> &out)
		{
			out.Clear();

			while(location < end)
			{
				int8 c = fileData[location++];
				if (c == '\n' || c == '\0')
					break;

				out += c;
			}

			return out.Size();
		}

		//! Reads the next string.
        /*!
            A valid string is alphanumeric or contains '_'. This function returns when a non valid
            int8 is found such as a space. This function removes any c style commenting.
         
            \param fileData Char array to read from.
            \param location Position in the array to start searching. This param will be counted forward and returned.
            \param end This is the max amount of chars to read.
            \param out String read.
            \return String length.
        */
		template<class T, class TAlloc>
		uint32 ReadNextString(const int8 *fileData, uint32 &location, uint32 end, MashBaseString<T, TAlloc> &out)
		{
			out.Clear();

			while(location < end)
			{
				int8 c = fileData[location];
				bool isValidStringChar = isalnum(c) || (c == '_');

				//have we found our string?
				if (!out.Empty() && !isValidStringChar)
					break;
				
				++location;

				if (c == '/')
				{
					if (location < end)
					{
						if (fileData[location] == '/')
							EatComment(fileData, location, end);
						else if (fileData[location] == '*')
							EatBlockComment(fileData, location, end);
					}
				}
				else if (isValidStringChar)
					out += c;
			}

			return out.Size();
		}

		//! Reads the next "string".
        /*!
            This function expects the next item to be a string literal. Removes any c style commenting.
         
            \param fileData Char array to read from.
            \param location Position in the array to start searching. This param will be counted forward and returned.
            \param end This is the max amount of chars to read.
            \param out String read.
            \return String length. 0 if the next item was not a string literal.
        */
		template<class T, class TAlloc>
		uint32 ReadNextStringLiteral(const int8 *fileData, uint32 &location, uint32 end, MashBaseString<T, TAlloc> &out)
		{
			out.Clear();

			while(location < end)
			{
				//reads upto the first "
				int8 c = fileData[location++];
				if (c == '"')
				{
					break;
				}
				else if (c == '/')
				{
					if (location < end)
					{
						if (fileData[location] == '/')
							EatComment(fileData, location, end);
						else if (fileData[location] == '*')
							EatBlockComment(fileData, location, end);
					}
				}
				else if (!isspace(c))
				{
					//the next object was not a string literal
					return 0;
				}
			}

			//reads the string after the first "
			while(location < end)
			{
				int8 c = fileData[location++];
				if (c == '"')
				{
					break;
				}
				else if (c == '/')
				{
					if (location < end)
					{
						if (fileData[location] == '/')
							EatComment(fileData, location, end);
						else if (fileData[location] == '*')
							EatBlockComment(fileData, location, end);
					}
				}
				else
				{
					out.PushBack(c);
				}
			}

			return out.Size();
		}
	}
}

#endif