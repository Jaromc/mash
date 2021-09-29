//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OPENGL_EFFECT_PROGRAM_H_
#define _C_MASH_OPENGL_EFFECT_PROGRAM_H_

#include "MashEffectProgramIntermediate.h"
#include "CMashOpenGLHeader.h"
#include "MashFileManager.h"

namespace mash
{
	class MashMaterialManager;

	class CMashOpenGLEffectProgram : public MashEffectProgramIntermediate
	{
	private:
		GLuint m_openGLID;
		GLenum m_shaderType;
	public:
		CMashOpenGLEffectProgram(ePROGRAM_TYPE programType,
			const int8 *compiledCode,
			const int8 *fileName, 
			const int8 *entry, 
			eSHADER_PROFILE profile);

		~CMashOpenGLEffectProgram();

		eMASH_STATUS _Compile(MashFileManager *pFileManager, 
			const MashMaterialManager *pSkinManager, 
			const sEffectMacro *customMacros, 
			uint32 customMacroCount,
			bool &isValid);
		GLuint GetOpenGLID()const;
	};

	inline GLuint CMashOpenGLEffectProgram::GetOpenGLID()const
	{
		return m_openGLID;
	}
}

#endif