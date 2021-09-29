//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLEffectProgram.h"
#include "CMashOpenGLHelper.h"
#include "MashMaterialManager.h"
#include "MashFileStream.h"
#include "MashHelper.h"
#include "MashLog.h"

namespace mash
{

	CMashOpenGLEffectProgram::CMashOpenGLEffectProgram(ePROGRAM_TYPE programType,
			const int8 *compiledCode,
			const int8 *fileName, 
			const int8 *entry, 
			eSHADER_PROFILE profile):MashEffectProgramIntermediate(programType, compiledCode, fileName, entry, profile),
			m_openGLID(0)
	{
	}

	CMashOpenGLEffectProgram::~CMashOpenGLEffectProgram()
	{
        if (m_openGLID != 0)
		{
			glDeleteShaderPtr(m_openGLID);
			m_openGLID = 0;
		}
	}

	eMASH_STATUS CMashOpenGLEffectProgram::_Compile(MashFileManager *pFileManager, 
		const MashMaterialManager *pSkinManager, 
		const sEffectMacro *customMacros, 
		uint32 customMacroCount,
		bool &isValid)
	{
		isValid = false;

		if (!pSkinManager->IsProfileSupported(m_profile))
		{            
            MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, 
                                "CMashOpenGLEffectProgram::_CompileProgram",
                                "Shader profile not supported on the current system for effect '%s'.",
                                m_fileName.GetCString());
            
			return aMASH_OK; //no error
		}

		if (m_openGLID != 0)
		{
			glDeleteShaderPtr(m_openGLID);
			m_openGLID = 0;
		}

		/*
			create gl shader if it has not been created
		*/
		if (m_openGLID == 0)
		{
			m_shaderType = MashToOpenGLProgramType(m_programType);

			if (m_shaderType != 0)
			{
				m_openGLID = glCreateShaderPtr(m_shaderType);
			}

			if (m_openGLID == 0)
			{                
                MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
                                    "CMashOpenGLEffectProgram::_CompileProgram",
                                    "Failed to create openGL shader for effect '%s'.",
                                    m_fileName.GetCString());

				return aMASH_FAILED;
			}
		}

		{
			/*
				TODO : Macros need to be added here too!
			*/

			if (m_compiledShader.Empty() && !m_fileName.Empty())
			{
				MashFileStream *pFileStream = pFileManager->CreateFileStream();
				if (pFileStream->LoadFile(m_fileName.GetCString(), aFILE_IO_TEXT) == aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
                                        "CMashOpenGLEffectProgram::_CompileProgram",
                                        "Failed to read program source '%s'.",
                                        m_fileName.GetCString());
					pFileStream->Destroy();
					return aMASH_FAILED;
				}

				if (pFileStream->GetDataSizeInBytes() > 0)
					m_compiledShader = (const int8*)pFileStream->GetData();

				pFileStream->Destroy();
                
                m_compiledFileName = m_fileName;
			}
		}

		if (m_compiledShader.Empty())
		{
            MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
                                "CMashOpenGLEffectProgram::_CompileProgram",
                                "No program source set for effect '%s'.",
                                m_fileName.GetCString());
            
			return aMASH_FAILED;
		}

		//finally add the shader
        
        GLchar *shaderSources[1] = {(GLchar*)m_compiledShader.GetCString()};
		glShaderSourcePtr(m_openGLID, 1, (const GLchar**)shaderSources, 0);
		glCompileShaderPtr(m_openGLID);

		int32 retParam;
		glGetShaderivPtr(m_openGLID, GL_COMPILE_STATUS, &retParam);

		if(retParam != GL_TRUE) 
		{
			int32 errorMsgLen = 0;
			glGetShaderivPtr(m_openGLID, GL_INFO_LOG_LENGTH, &errorMsgLen);
			if (errorMsgLen > 0)
			{
				int32 charsWritten = 0;
				int8 *errorMsg = (int8*)MASH_ALLOC_COMMON(errorMsgLen);
				glGetShaderInfoLogPtr(m_openGLID, errorMsgLen, &charsWritten, errorMsg);
                
                MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
                                    "CMashOpenGLEffectProgram::_CompileProgram",
                                    "Native high level shader error in effect: '%s', generated effect: '%s'.",
                                    m_fileName.GetCString(),
                                    m_compiledFileName.GetCString());
                
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 errorMsg, 
                                 "CMashOpenGLEffectProgram::_CompileProgram");

				MASH_FREE(errorMsg);
			}
			else
			{
                MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
                                    "CMashOpenGLEffectProgram::_CompileProgram",
                                    "Native high level shader error in effect: '%s', generated effect: '%s'. No API message was generated.",
                                    m_fileName.GetCString(),
                                    m_compiledFileName.GetCString());
			}

            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                             m_compiledShader.GetCString(),
                             "CMashOpenGLEffectProgram::_CompileProgram");

            m_compiledFileName.FreeMemory();
            m_compiledShader.FreeMemory();

			return aMASH_FAILED;
		}

        m_compiledFileName.FreeMemory();
        m_compiledShader.FreeMemory();

		isValid = true;
		return aMASH_OK;
	}
}
