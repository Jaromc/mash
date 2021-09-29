//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef MASH_STRING_HELPER_H_
#define MASH_STRING_HELPER_H_

#include "MashString.h"
#include "MashMathHelper.h"
#include <cctype>
namespace mash
{
    //! Removes the final name and '.' from the string.
    /*!
        \param fileName File name in the format of someFile.ext.
        \param out File extenstion.
     */
    template<class T, class TAlloc>
    void GetFileExtention(const int8 *fileName, MashBaseString<T, TAlloc> &out)
    {
        //get extension
		out = "";
        
        if (!fileName)
            return;
        
		const int32 filePathLen = math::Max<int32>(0, ((int32)strlen(fileName)) - 1);
		for(int32 i = filePathLen; i >= 0; --i)
		{
			if (fileName[i] == '.')
				break;
			else if (!isspace(fileName[i]))
				out.Insert(out.Begin(), fileName[i]);
		}
    }
    
    //! Gets the file name only from a path.
    /*!
        /Path1/Path2/Filename.ext would return Filename

        \param fileName File path.
        \param fileNameOnlyOut Returned file name only.
     */
    template<class T, class TAlloc>
    void GetFileName(const int8 *fileName, MashBaseString<T, TAlloc> &fileNameOnlyOut)
    {
        fileNameOnlyOut = "";
        
        if (!fileName)
            return;
        
		const int32 filePathLen = math::Max<int32>(0, ((int32)strlen(fileName)) - 1);
		bool fileNameFound = false;
		//start from the extention and work our way back
		for(int32 i = filePathLen; i >= 0; --i)
		{
			if (!fileNameFound && (fileName[i] == '.'))
				fileNameFound = true;
			else if (fileNameFound && ((fileName[i] == '\\') || (fileName[i] == '/')))
				break;
			else if (fileNameFound && (!isspace(fileName[i])))
				fileNameOnlyOut.Insert(fileNameOnlyOut.Begin(), fileName[i]);
		}
    }
    
    //! Gets the file name and extention from a path.
    /*!
        /Path1/Path2/Filename.ext would return Filename.ext

        \param fileName File path.
        \param fileNameOnlyOut Returned file name only.
     */
    template<class T, class TAlloc>
    void GetFileNameAndExtention(const int8 *fileName, MashBaseString<T, TAlloc> &out)
    {
        //get extension
		out = "";
        
        if (!fileName)
            return;
        
		const int32 filePathLen = math::Max<int32>(0, ((int32)strlen(fileName)) - 1);
		for(int32 i = filePathLen; i >= 0; --i)
		{
			if (fileName[i] == '/' || fileName[i] == '\\')
				break;
			else if (!isspace(fileName[i]))
				out.Insert(out.Begin(), fileName[i]);
		}
    }
    
    //! Gets the file path from a file directory.
    /*!
        /Path1/Path2/Filename.ext would return /Path1/Path2

        \param fileName File name.
        \param out Files path only.
     */
    template<class T, class TAlloc>
    void GetFilePath(const int8 *fileName, MashBaseString<T, TAlloc> &out)
    {
        //get extension
		out = "";
        
        if (!fileName)
            return;
        
		const int32 filePathLen = math::Max<int32>(0, ((int32)strlen(fileName)) - 1);
		int32 i = filePathLen;
		bool extFound = false;
		for(; i >= 0; --i)
		{
			if (fileName[i] == '.')
				extFound = true;
			else if (extFound && (fileName[i] == '/' || fileName[i] == '\\'))
				break;
		}
        
		out = fileName;

		/*
			If there was an extention then we remove it. Otherwise it is assumed
			the string passed in was a path already.
		*/
		if (extFound)
			out.Erase(out.Begin() + i, out.End());
    }
    
    //! Concatenates two paths.
    /*!
        Handy for concatenating a path and file name.
        \param a First path.
        \param b Second path.
        \param out Concatentating path.
     */
    template<class T, class TAlloc>
    void ConcatenatePaths(const int8 *a, const int8 *b, MashBaseString<T, TAlloc> &out)
    {
        if (!a || (a[0] == '\0'))
		{
			if (b)
				out = b;
			else
				out = "";
            
			return;
		}
        
		if (!b || (b[0] == '\0'))
		{
			if (a)
				out = a;
			else
				out = "";
            
			return;
		}
        
		MashBaseString<T, TAlloc> temp = a;
        
		//add a dir if not already added
        int8 lastChar = temp.Back();
        if ((lastChar != '/') && (lastChar != '\\'))
			temp += "/";
        
		//if a dir exists on the second str then remove it
		const int8 *tb = b;
		if ((tb[0] == '/') || (tb[0] == '\\'))
			++tb;
        
        temp += tb;
        
		out = temp;
    }
}

#endif
