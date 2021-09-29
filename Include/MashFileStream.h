//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_FILE_STREAM_H_
#define _MASH_FILE_STREAM_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"

namespace mash
{    
    /*!
        A file stream simplifes data reading and writing from the VFS and API FS. This
        can be created from MashFileManager::CreateFileStream().
     
        This is basically just a helper class that utilises the functions found in the file manager.
     
        Destroy() must be called when you are done with a stream.
    */
	class MashFileStream : public MashReferenceCounter
	{
	public:
		MashFileStream():MashReferenceCounter(){}
		virtual ~MashFileStream(){}

        //! Load a file.
        /*!
            The loaded data can be grabbed from GetData() and GetDataSizeInBytes().
         
            \param fileName File to load.
            \param mode Mode.
            \return Status of the function.
        */
		virtual eMASH_STATUS LoadFile(const int8 *fileName, eFILE_IO_MODE mode) = 0;
        
        //! Saves/Flushes the data held in this stream to a file in the API FS.
        /*!
            This function will not clear the stream.
         
            After calling this, ClearStream() can be called to start appending
            new data for saving.
         
            \param fileName File directory and name to save to.
            \param mode Mode.
            \return Status of the function.
         */
		virtual eMASH_STATUS SaveFile(const int8 *fileName, eFILE_IO_MODE mode) = 0;
        
        //! Destroys this stream.
        /*!
            This should be called when you are done with the stream.
         */
		virtual void Destroy() = 0;

        //! Appends a null terminated string to this stream.
        /*!
			This will write all the characters upto but not including the null terminator.

            \param str String.
        */
		virtual void AppendStringToStream(const int8 *str) = 0;
        
        //! Appends data to this stream.
        /*!
            This can be any data, not necessarily null terminated.

            \param data Data to append.
            \param sizeInBytes Data size in bytes to append.
        */
		virtual void AppendToStream(const void *data, uint32 sizeInBytes) = 0;
        
        //! File stream data.
        /*!
            \return Current file data.
        */
		virtual const void* GetData()const = 0;
        
        //! File stream data size.
        /*!
            \return Current file data size in bytes.
        */
		virtual uint32 GetDataSizeInBytes()const = 0;
        
        //! Clears the current stream data.
		virtual void ClearStream() = 0;
	};
}

#endif
