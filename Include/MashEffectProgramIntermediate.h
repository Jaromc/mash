//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_EFFECT_PROGRAM_INTERMEDIATE_H_
#define _MASH_EFFECT_PROGRAM_INTERMEDIATE_H_

#include "MashEffectProgram.h"

namespace mash
{
    /*!
        Impliments common functions for API dependent effects.
        This class should not be accessed directly, use MashEffectProgram instead.
    */
	class MashEffectProgramIntermediate : public MashEffectProgram
	{
	protected:
		ePROGRAM_TYPE m_programType;
		MashStringc m_fileName;
		MashStringc m_entry;
		MashStringc m_compiledShader;
		MashStringc m_compiledEntry;
        MashStringc m_compiledFileName;
		eSHADER_PROFILE m_profile;
		MashArray<sEffectMacro> m_compileArgs;
	public:
		MashEffectProgramIntermediate(ePROGRAM_TYPE programType,
			const int8 *compiledCode,
			const int8 *fileName, 
			const int8 *entry, 
			eSHADER_PROFILE profile);

		virtual ~MashEffectProgramIntermediate();

		const MashStringc& GetFileName()const;
		eSHADER_PROFILE GetProfile()const;
		const MashStringc& GetEntry()const;
		ePROGRAM_TYPE GetProgramType()const;
		void SetCompileArguments(const MashArray<sEffectMacro> &compileArgs);
		void AddCompileArgument(const sEffectMacro &compileArg);
		const MashArray<sEffectMacro>& GetCompileArguments()const;
		const MashStringc& GetHighLevelSource()const;
		const MashStringc& GetHighLevelSourceEntry()const;
		void SetHighLevelSource(const MashStringc &source, const MashStringc &entry, const MashStringc &fileName);
	};

	inline const MashStringc& MashEffectProgramIntermediate::GetHighLevelSourceEntry()const
	{
		return m_compiledEntry;
	}

	inline const MashStringc& MashEffectProgramIntermediate::GetHighLevelSource()const
	{
		return m_compiledShader;
	}

	inline const MashArray<sEffectMacro>& MashEffectProgramIntermediate::GetCompileArguments()const
	{
		return m_compileArgs;
	}

	inline ePROGRAM_TYPE MashEffectProgramIntermediate::GetProgramType()const
	{
		return m_programType;
	}

	inline const MashStringc& MashEffectProgramIntermediate::GetFileName()const
	{
		return m_fileName;
	}

	inline eSHADER_PROFILE MashEffectProgramIntermediate::GetProfile()const
	{
		return m_profile;
	}

	inline const MashStringc& MashEffectProgramIntermediate::GetEntry()const
	{
		return m_entry;
	}
}

#endif