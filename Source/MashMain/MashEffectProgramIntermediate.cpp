//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashEffectProgramIntermediate.h"
#include "MashDevice.h"
namespace mash
{
	MashEffectProgramIntermediate::MashEffectProgramIntermediate(ePROGRAM_TYPE programType,
			const int8 *compiledCode,
			const int8 *fileName, 
			const int8 *entry, 
			eSHADER_PROFILE profile):MashEffectProgram(), m_programType(programType), m_fileName(""),
			m_entry(entry), m_profile(profile)
	{
		if (fileName)
		{
			m_fileName = fileName;
		}

		if (compiledCode)
		{
			m_compiledShader = compiledCode;
			m_fileName = "";
		}
	}

	MashEffectProgramIntermediate::~MashEffectProgramIntermediate()
	{
	}

	void MashEffectProgramIntermediate::SetCompileArguments(const MashArray<sEffectMacro> &compileArgs)
	{
		m_compileArgs = compileArgs;
	}
    
    void MashEffectProgramIntermediate::SetHighLevelSource(const MashStringc &source, const MashStringc &entry, const MashStringc &fileName)
	{
		m_compiledShader = source;
		m_compiledEntry = entry;
        m_compiledFileName = fileName;
	}

	void MashEffectProgramIntermediate::AddCompileArgument(const sEffectMacro &compileArg)
	{
		if (compileArg.name.Empty())
			return;

		const uint32 count = m_compileArgs.Size();
		for(uint32 i = 0; i < count ; ++i)
		{
			if (m_compileArgs[i].name == compileArg.name)
				return;
		}

		m_compileArgs.PushBack(compileArg);
	}
}