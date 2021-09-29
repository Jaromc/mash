//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_EFFECT_PROGRAM_H_
#define _MASH_EFFECT_PROGRAM_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"
#include "MashTypes.h"
#include "MashArray.h"
#include "MashString.h"

namespace mash
{
    /*!
        Parameter handles represent a parameter within an effect program. 
    */
	class MashEffectParamHandle : public MashReferenceCounter
	{
	public:
		MashEffectParamHandle():MashReferenceCounter(){}
		virtual ~MashEffectParamHandle(){}
	};

    /*!
        Effect programs are vertex, pixel, geometry, etc programs
        for the GPU. They can be created from MashEffect::AddProgram.
    */
	class MashEffectProgram : public MashReferenceCounter
	{
	public:
		MashEffectProgram():MashReferenceCounter(){}
		virtual ~MashEffectProgram(){}

        //! File name.
        /*!
            \return Program file name.
        */
		virtual const MashStringc& GetFileName()const = 0;
        
        //! Program profile.
        /*!
            \return Program profile.
        */
		virtual eSHADER_PROFILE GetProfile()const = 0;
        
        //! Entry point.
        /*!
            \return Entry point.
        */
		virtual const MashStringc& GetEntry()const = 0;
        
        //! Program type.
        /*!
            \return Program type.
        */
		virtual ePROGRAM_TYPE GetProgramType()const = 0;
        
        //! Compile arguments.
        /*!
            This will reset any compile arguments already set.
         
            \param compileArgs Arguments to be used at compile time.
        */
		virtual void SetCompileArguments(const MashArray<sEffectMacro> &compileArgs) = 0;
        
        //! Adds compile arguments to the argument list.
        /*!
            \param compileArg Compile argument to add.
        */
		virtual void AddCompileArgument(const sEffectMacro &compileArg) = 0;

		//! Compile argument list.
		/*!
			This list should not be modified. Instead use one of the accessor functions.

			\return Compile argument list.
		*/
		virtual const MashArray<sEffectMacro>& GetCompileArguments()const = 0;

		//! Native high level program.
		/*!
			Returns the code used for compiling this program. 
			If this program is created at runtime then the final generated shader is returned.

			After this effect has been compiled this string will be destroyed.

			\return High level shader code.
		*/
		virtual const MashStringc& GetHighLevelSource()const = 0;

		//! Entry point to the high level source program.
		/*!
			Note this can be different from GetEntry().
			If this program was generated at runtime then this will be the
			generated entry point set via SetHighLevelSource().

			\return High level entry point.
		*/
		virtual const MashStringc& GetHighLevelSourceEntry()const = 0;

		//! Sets the native high level source.
		/*!
			Sets the code used to compile this program. It is assumed this code
			is in native high level format.
			
			If this is set then it will override loading from file (if set).
			If set to "" then this program will load from file (if set).
			This program must be recompiled before the new settings take affect.

			After this effect has been compiled this string will be destroyed.

			\param source High level native source.
			\param entry Entry point function name. 
            \param fileName Used only for debug logging.
		*/
		virtual void SetHighLevelSource(const MashStringc &source, const MashStringc &entry, const MashStringc &fileName) = 0;
	};
}

#endif