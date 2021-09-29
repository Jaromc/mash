//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashOpenGLEffect.h"
#include "CMashOpenGLRenderer.h"
#include "CMashOpenGLSkinManager.h"
#include "CMashOpenGLTexture.h"
#include "CMashOpenGLTextureState.h"
#include "MashMaterialManager.h"
#include "MashHelper.h"
#include "MashTypes.h"

#include "MashMatrix4.h"
#include "MashVector2.h"
#include "MashVector3.h"
#include "MashVector4.h"
#include "MashTexture.h"
#include "MashFileManager.h"
#include "MashEffectProgram.h"

#include "MashLog.h"

namespace mash
{
	CMashOglSharedUniformBuffer::CMashOglSharedUniformBuffer(CMashOpenGLSkinManager *_manager, const MashStringc &_bufferName, GLuint _uboIndex, GLuint _bindingIndex, GLuint _bufferSize):uboIndex(_uboIndex), 
		bindingIndex(_bindingIndex), bufferSize(_bufferSize), bufferName(_bufferName), manager(_manager)
	{
	}

    CMashOglSharedUniformBuffer::~CMashOglSharedUniformBuffer()
    {
		/*
			bad hack. If the material manager has any ubos alive when it's being destroyed then the
			ubos will have their destructors called too.
			But in that case we don't want to call the manager here. So we do a simple ref check as the ref class
			should still be valid.
		*/
		if (manager->GetReferenceCount() > 0)
			manager->_OnSharedUniformBufferDelete(this);

        glDeleteBuffersPtr(1, &uboIndex);
    }
    
	CMashOpenGLEffect::CMashOpenGLEffect(CMashOpenGLRenderer *renderer):MashEffect(),m_pRenderer(renderer),
		m_bIsCompiled(false), m_bIsValid(false), m_openGLProgamID(0), m_isAPIEffect(false)
	{
		for(uint32 i = 0; i < aPROGRAM_UNKNOWN; ++i)
			m_programs[i] = 0;
	}

	CMashOpenGLEffect::~CMashOpenGLEffect()
	{   
        if (m_openGLProgamID != 0)
        {
            //automatically detaches any attached shaders
            glDeleteProgramPtr(m_openGLProgamID);
            m_openGLProgamID = 0;
        }
        
        for(uint32 i = 0; i < aPROGRAM_UNKNOWN; ++i)
        {
            if (m_programs[i])
                m_programs[i]->Drop();
            
            m_programs[i] = 0;
        }
        
        std::map<MashStringc, CMashOpenGLEffectParamHandle*>::iterator iterParam = m_parameters.begin();
        std::map<MashStringc, CMashOpenGLEffectParamHandle*>::iterator iterParamEnd = m_parameters.end();
        for(; iterParam != iterParamEnd; ++iterParam)
        {
            MASH_DELETE iterParam->second;
        }
        
        m_parameters.clear();

	}

	MashEffect* CMashOpenGLEffect::CreateIndependentCopy()
	{
		CMashOpenGLEffect *newEffect = (CMashOpenGLEffect*)m_pRenderer->GetMaterialManager()->CreateEffect();
		newEffect->m_isAPIEffect = m_isAPIEffect;

		for(uint32 i = 0; i < aPROGRAM_UNKNOWN; ++i)
		{
			if (m_programs[i])
			{
				/*
					If copying from a runtime effect then we dont want to pass in the compiled code, else
					it will be treated as an api effect, and wont recompile with new lighting changes
				*/
				MashEffectProgram *program = 0;
				if (m_isAPIEffect)
					program = newEffect->AddProgramCompiled(m_programs[i]->GetHighLevelSource().GetCString(), m_programs[i]->GetEntry().GetCString(), m_programs[i]->GetProfile(), m_programs[i]->GetFileName().GetCString());
				else
					program = newEffect->AddProgram(m_programs[i]->GetFileName().GetCString(), m_programs[i]->GetEntry().GetCString(), m_programs[i]->GetProfile());

				program->SetCompileArguments(m_programs[i]->GetCompileArguments());
			}
		}

		return newEffect;
	}

	MashEffectProgram* CMashOpenGLEffect::AddProgramCompiled(const int8 *highLevelCode, const int8 *entry, eSHADER_PROFILE profile, const int8 *fileName)
	{
		if (!highLevelCode)
			return 0;

		ePROGRAM_TYPE programType = mash::helpers::GetEffectProgramTypeFromProfile(profile);

		if (programType == aPROGRAM_UNKNOWN)
		{
            if (!fileName)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Unknown profile type used. The program was not added.", 
                                 "CMashOpenGLEffect::AddProgramCompiled");
            }
            else
            {
            
                MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
                                "CMashOpenGLEffect::AddProgramCompiled",
                                "Unknown profile type used for effect '%s'. The program was not added.",
                                fileName);
            }

			return 0;
		}

		m_isAPIEffect =  true;

		CMashOpenGLEffectProgram *program = MASH_NEW_COMMON CMashOpenGLEffectProgram(programType,
			highLevelCode,
			fileName,
			entry,
			profile);

		if (m_programs[programType])
			MASH_DELETE m_programs[programType];

		m_programs[programType] = program;

		return program;
	}

	MashEffectProgram* CMashOpenGLEffect::AddProgram(const int8 *fileName, const int8 *entry, eSHADER_PROFILE profile)
	{
		if (!fileName)
			return 0;

		ePROGRAM_TYPE programType = mash::helpers::GetEffectProgramTypeFromProfile(profile);

		if (programType == aPROGRAM_UNKNOWN)
		{
			if (!fileName)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Unknown profile type used. The program was not added.", 
                                 "CMashOpenGLEffect::AddProgram");
            }
            else
            {
                
                MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
                                    "CMashOpenGLEffect::AddProgram",
                                    "Unknown profile type used for effect '%s'. The program was not added.",
                                    fileName);
            }

			return 0;
		}

		m_isAPIEffect = mash::helpers::IsFileANativeEffectProgram(fileName);

		CMashOpenGLEffectProgram *program = MASH_NEW_COMMON CMashOpenGLEffectProgram(programType,
			0,
			fileName,
			entry,
			profile);

		if (m_programs[programType])
			MASH_DELETE m_programs[programType];

		m_programs[programType] = program;

		return program;
	}

	GLenum CMashOpenGLEffect::GetOGLTextureSlotEnum(uint32 index)
	{
		switch(index)
		{
		case 0:
			return GL_TEXTURE0;
		case 1:
			return GL_TEXTURE1;
		case 2:
			return GL_TEXTURE2;
		case 3:
			return GL_TEXTURE3;
		case 4:
			return GL_TEXTURE4;
		case 5:
			return GL_TEXTURE5;
		case 6:
			return GL_TEXTURE6;
		case 7:
			return GL_TEXTURE7;
		case 8:
			return GL_TEXTURE8;
		case 9:
			return GL_TEXTURE9;
		case 10:
			return GL_TEXTURE10;
		case 11:
			return GL_TEXTURE11;
		case 12:
			return GL_TEXTURE12;
		case 13:
			return GL_TEXTURE13;
		case 14:
			return GL_TEXTURE14;
		case 15:
			return GL_TEXTURE15;
		case 16:
			return GL_TEXTURE16;
		case 17:
			return GL_TEXTURE17;
		case 18:
			return GL_TEXTURE18;
		case 19:
			return GL_TEXTURE19;
		case 20:
			return GL_TEXTURE20;
		default:
			return GL_TEXTURE0;
		};
	}

	eMASH_STATUS CMashOpenGLEffect::_Compile(MashFileManager *fileManager, 
		const MashMaterialManager *skinManager, 
		const sEffectCompileArgs &compileArgs)
	{
		m_bIsCompiled = true;
		m_bIsValid = true;

		if (!m_isAPIEffect)
		{
			if (((MashMaterialManager*)skinManager)->BuildRunTimeEffect(this, compileArgs) == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to compile effect into native format.", 
					"CMashOpenGLEffect::_CompileProgram");

				return aMASH_FAILED;
			}
		}

		m_autoParameters.Clear();

		std::map<MashStringc, CMashOpenGLEffectParamHandle*>::iterator iter = m_parameters.begin();
		for(; iter != m_parameters.end(); ++iter)
		{
			iter->second->Drop();
		}
		m_parameters.clear();

		for(uint32 i = 0; i < aPROGRAM_UNKNOWN; ++i)
		{
			if (m_programs[i])
			{
				//delete old data if any
				if (m_openGLProgamID != 0 && m_programs[i]->GetOpenGLID() != 0)
				{
                    glDetachShaderPtr(m_openGLProgamID, m_programs[i]->GetOpenGLID());
				}

				bool isValid = false;
				if (m_programs[i]->_Compile(fileManager, skinManager, compileArgs.macros, compileArgs.macroCount, isValid) == aMASH_FAILED)
				{
					m_bIsCompiled = false;
					m_bIsValid = false;
				}

				if (!isValid)
					m_bIsValid = false;
			}
		}

		if (!m_bIsCompiled)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Failed to compile effect.", 
				"CMashOpenGLEffect::_CompileProgram");

			return aMASH_FAILED;
		}

		if (!m_bIsValid)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
				"Effect is not valid on the current system.", 
				"CMashOpenGLEffect::_CompileProgram");

			return aMASH_OK;
		}

		//reset the flag for the next stage
		m_bIsValid = false;
        
        if (m_openGLProgamID != 0)
        {
            glDeleteProgramPtr(m_openGLProgamID);
            m_openGLProgamID = 0;
        }

		/*
			Create the gl program if it has not already
			been created
		*/
		if (m_openGLProgamID == 0)
		{
			m_openGLProgamID = glCreateProgramPtr();

			if (m_openGLProgamID == 0)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to create openGL program.", 
					"CMashOpenGLEffect::_CompileProgram");

				return aMASH_FAILED;
			}

			for(uint32 i = 0; i < aPROGRAM_UNKNOWN; ++i)
			{
				if (m_programs[i])
					glAttachShaderPtr(m_openGLProgamID, m_programs[i]->GetOpenGLID());
			}
            
			/*
				TODO : A hack and should be temporary.
			*/
            glBindFragDataLocationPtr(m_openGLProgamID, 0, "_FragDataOut0");
            glBindFragDataLocationPtr(m_openGLProgamID, 1, "_FragDataOut1");
            glBindFragDataLocationPtr(m_openGLProgamID, 2, "_FragDataOut2");
            glBindFragDataLocationPtr(m_openGLProgamID, 3, "_FragDataOut3");
 
			int32 linkStatus = 0;
			glLinkProgramPtr(m_openGLProgamID);
			
			glGetProgramivPtr(m_openGLProgamID, GL_LINK_STATUS, &linkStatus);
			if (linkStatus != GL_TRUE)
			{
				const GLsizei bsize = 10000;
				GLchar str[bsize];
				GLsizei len;
				glGetProgramInfoLogPtr(m_openGLProgamID, bsize, &len, str);
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to link openGL programs.", 
					"CMashOpenGLEffect::_CompileProgram");
                
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                str, "CMashOpenGLEffect::_CompileProgram");

				return aMASH_FAILED;
			}
			
			int32 attribCount = 0;
			GLchar attribNameBuffer[256];
			GLsizei attribNameLen = 0;
			GLint attribSize = 0;
			GLenum attribType;
			glGetProgramivPtr(m_openGLProgamID, GL_ACTIVE_ATTRIBUTES, &attribCount);
			for(uint32 i = 0; i < attribCount; ++i)
			{
                glGetActiveAttribPtr(m_openGLProgamID, i, sizeof(attribNameBuffer), &attribNameLen,
					&attribSize, &attribType, attribNameBuffer);

                GLint attribLocation = glGetAttribLocationPtr(m_openGLProgamID, attribNameBuffer);

				eVERTEX_DECLUSAGE declarationType;
				uint32 usageIndex;
				if (mash::helpers::IsOpenGLInputAttribute(attribNameBuffer, declarationType, usageIndex))
				{
					m_vertexInputAttributes.PushBack(sMashOpenGLVertexAttribute(attribLocation, declarationType, usageIndex));
				}
				else
				{
					int8 buffer[256];
					mash::helpers::PrintToBuffer(buffer, sizeof(buffer), "Uknown vertex input '%s', found in effect program.", attribNameBuffer);
					MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING, buffer, "CMashOpenGLEffect::_CompilePrograml");
				}
			}
			
		}
		else
		{
			//just link the re-compiled shaders
			for(uint32 i = 0; i < aPROGRAM_UNKNOWN; ++i)
			{
				if (m_programs[i])
					glAttachShaderPtr(m_openGLProgamID, m_programs[i]->GetOpenGLID());
			}
            
			//hack, 4 output targets are supported
            glBindFragDataLocationPtr(m_openGLProgamID, 0, "_FragDataOut0");
            glBindFragDataLocationPtr(m_openGLProgamID, 1, "_FragDataOut1");
            glBindFragDataLocationPtr(m_openGLProgamID, 2, "_FragDataOut2");
            glBindFragDataLocationPtr(m_openGLProgamID, 3, "_FragDataOut3");

			int32 linkStatus = 0;
			glLinkProgramPtr(m_openGLProgamID);
			glGetProgramivPtr(m_openGLProgamID, GL_LINK_STATUS, &linkStatus);
			if (linkStatus != GL_TRUE)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
					"Failed to link openGL programs.", 
					"CMashOpenGLEffect::_CompileProgram");

				return aMASH_FAILED;
			}
		}

		m_bIsValid = true;

		if (m_programs[aPROGRAM_VERTEX] && m_programs[aPROGRAM_PIXEL])
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
					"CMashOpenGLEffect::_Compile",
					"Adding auto parameters to effect programs '%s' and '%s'...", m_programs[aPROGRAM_VERTEX]->GetFileName().GetCString(),
						m_programs[aPROGRAM_PIXEL]->GetFileName().GetCString());
		}
		else
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_INFORMATION, 
				"Adding auto parameters to effect programs...",
				"CMashOpenGLEffect::_Compile");
		}

		//get program parameters
		int32 uniformCount = 0;
		glGetProgramivPtr(m_openGLProgamID, GL_ACTIVE_UNIFORMS, &uniformCount);
		if (uniformCount > 0)
		{                                              
			GLenum paramType = GL_ZERO;
			int8 paramName[256];
			GLsizei paramNameLen = 0;
			GLint paramSize = 0;
			GLuint paramLocation = 0;
			uint32 semanticType = 0;
			uint32 semanticIndex = 0;
			int8 newParamName[256];
			uint32 textureCounter = 0;
            bool isStructParam = false;
            
			for(uint32 i = 0; i < uniformCount; ++i)
			{
				glGetActiveUniformPtr(m_openGLProgamID, i, sizeof(paramName)-1, &paramNameLen, &paramSize, &paramType, paramName);

				paramName[paramNameLen] = 0;

				if (paramName[0] != 0)
				{
					paramLocation = glGetUniformLocationPtr(m_openGLProgamID, paramName);
                    
					/*
						-1 will be returned if this parameter is part of a structure/uniform buffer.
						We need to handle these as special cases.
					*/
					if (paramLocation != -1)
					{
                        CMashOpenGLEffectParamHandle *newParam = MASH_NEW_COMMON CMashOpenGLEffectParamHandle(paramLocation, paramType, paramSize);
						m_parameters[paramName] = newParam;
                        
                        isStructParam = false;
						mash::helpers::GetAutoEffectParameterName(paramName, newParamName, semanticIndex, isStructParam);
						if (m_pRenderer->GetMaterialManager()->IsAutoParameter(newParamName, semanticType))
						{
							if (paramType == GL_SAMPLER_1D || paramType == GL_SAMPLER_2D ||
								paramType == GL_SAMPLER_3D || paramType == GL_SAMPLER_CUBE)
							{
								MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
												"CMashOpenGLEffect::_Compile",
												"Adding auto parameter texture '%s'.", paramName);

								newParam->SetEffectTextureIndex(/*semanticIndex*/textureCounter);
								newParam->SetOGLTextureSlot(GetOGLTextureSlotEnum(/*semanticIndex*/textureCounter));
								++textureCounter;
							}
							else
							{
								MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
												"CMashOpenGLEffect::_Compile",
												"Adding auto parameter '%s' size '%d'.", paramName, paramSize);
							}
                            
							m_autoParameters.PushBack(sAutoParameter(newParam, semanticType, semanticIndex));
						}
					}
					else
					{
						//parameter must be part of a constant uniform buffer
						int32 i = 0;
					}
				}
			}

			//handle uniform buffers
			int8 *uniformBufferName = 0;
			uint32 uniformBufferNameMemSize = 0;
			int32 numActiveUniformBlocks = 0;
			glGetProgramivPtr(m_openGLProgamID, GL_ACTIVE_UNIFORM_BLOCKS, &numActiveUniformBlocks);
			for(uint32 i = 0; i < numActiveUniformBlocks; ++i)
			{
				GLint nameLength = 0;
                glGetActiveUniformBlockivPtr(m_openGLProgamID, i, GL_UNIFORM_BLOCK_NAME_LENGTH, &nameLength);

				if (nameLength <= 0)
				{
					//no more buffers to search?
					continue;
				}
				else
				{
					if (uniformBufferNameMemSize < nameLength)
					{
						MASH_FREE(uniformBufferName);
						uniformBufferName = (int8*)MASH_ALLOC_COMMON(nameLength);
						uniformBufferNameMemSize = nameLength;
					}

                    glGetActiveUniformBlockNamePtr(m_openGLProgamID, i, 256, 0, uniformBufferName);

					/*
						Uniform buffers in ogl are shared between all programs that use it.
					*/
					GLsizei uniformBlockSize = 0;
                    glGetActiveUniformBlockivPtr(m_openGLProgamID, i, GL_UNIFORM_BLOCK_DATA_SIZE, &uniformBlockSize);

					CMashOpenGLSkinManager *skinManager = (CMashOpenGLSkinManager*)m_pRenderer->GetMaterialManager();
					CMashOglSharedUniformBuffer *sharedUbo = skinManager->GetSharedUniformBuffer(uniformBufferName, uniformBlockSize);

					if (sharedUbo)
					{
						CMashOpenGLEffectParamHandle *newParam = MASH_NEW_COMMON CMashOpenGLEffectParamHandle(i, GL_UNIFORM_BUFFER, uniformBlockSize, sharedUbo);
                        
                        sharedUbo->Drop();
						
                        glUniformBlockBindingPtr (m_openGLProgamID, i, sharedUbo->bindingIndex);

						m_parameters[uniformBufferName] = newParam;

						int8 newParamName[256];//make dynamic buffer
						newParamName[0] = 0;
						uint32 semanticType = 0;
						uint32 semanticIndex = 0;
						mash::helpers::GetAutoEffectParameterName(uniformBufferName, newParamName, semanticIndex, isStructParam);
						if (m_pRenderer->GetMaterialManager()->IsAutoParameter(newParamName, semanticType))
						{
							MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
												"CMashOpenGLEffect::_Compile",
												"Adding auto parameter ubo '%s' size '%d'.", uniformBufferName, uniformBlockSize);

							m_autoParameters.PushBack(sAutoParameter(newParam, semanticType, semanticIndex));
						}
					}
				}
			}

			if (uniformBufferName)
				MASH_FREE(uniformBufferName);
		}

		return aMASH_OK;
	}

	void CMashOpenGLEffect::SetMatrix(MashEffectParamHandle *pHandle, const mash::MashMatrix4 *data, uint32 count)
	{  
        glUniformMatrix4fvPtr(((CMashOpenGLEffectParamHandle*)pHandle)->GetOpenGLLocation(), count, false, (GLfloat*)data->m);
	}

	void CMashOpenGLEffect::SetInt(MashEffectParamHandle *pHandle, const int32 *data, uint32 count)
	{
		CMashOpenGLEffectParamHandle *openGLHandle = (CMashOpenGLEffectParamHandle*)pHandle;
		if (openGLHandle->GetOpenGLSize() > 1)
		{
            glUniform1ivPtr(openGLHandle->GetOpenGLLocation(), count, data);
		}
		else
		{
            glUniform1iPtr(openGLHandle->GetOpenGLLocation(), *data);
		}
	}

	void CMashOpenGLEffect::SetVector2(MashEffectParamHandle *pHandle, const mash::MashVector2 *data, uint32 count)
	{
		CMashOpenGLEffectParamHandle *openGLHandle = (CMashOpenGLEffectParamHandle*)pHandle;
		switch(openGLHandle->GetOpenGLType())
		{
		case GL_INT_VEC2:
			{
				if (count > 1)
				{
                    glUniform2ivPtr(openGLHandle->GetOpenGLLocation(), count * 2, (GLint*)data->v);
				}
				else
				{
                    glUniform2iPtr(openGLHandle->GetOpenGLLocation(), data->x, data->y);
				}
				break;
			}
		case GL_FLOAT_VEC2:
			{
				if (count > 1)
				{
                    glUniform2fvPtr(openGLHandle->GetOpenGLLocation(), count * 2, data->v);
				}
				else
				{
                    glUniform2fPtr(openGLHandle->GetOpenGLLocation(), data->x, data->y);
				}
				break;
			}
		default:
			return;
		}
	}

	void CMashOpenGLEffect::SetVector3(MashEffectParamHandle *pHandle, const mash::MashVector3 *data, uint32 count)
	{
		CMashOpenGLEffectParamHandle *openGLHandle = (CMashOpenGLEffectParamHandle*)pHandle;
		switch(openGLHandle->GetOpenGLType())
		{
		case GL_INT_VEC3:
			{
				if (count > 1)
				{
                    glUniform3ivPtr(openGLHandle->GetOpenGLLocation(), count * 3, (GLint*)data->v);
				}
				else
				{
                    glUniform3iPtr(openGLHandle->GetOpenGLLocation(), data->x, data->y, data->z);
				}
				break;
			}
		case GL_FLOAT_VEC3:
			{
				if (count > 1)
				{
                    glUniform3fvPtr(openGLHandle->GetOpenGLLocation(), count * 3, data->v);
				}
				else
				{
                    glUniform3fPtr(openGLHandle->GetOpenGLLocation(), data->x, data->y, data->z);
				}
				break;
			}
		default:
			return;
		}
	}

	void CMashOpenGLEffect::SetBool(MashEffectParamHandle *pHandle, const bool *data, uint32 count)
	{
		CMashOpenGLEffectParamHandle *openGLHandle = (CMashOpenGLEffectParamHandle*)pHandle;
		if (openGLHandle->GetOpenGLSize() > 1)
		{
            glUniform1ivPtr(openGLHandle->GetOpenGLLocation(), count, (GLint*)data);
		}
		else
		{
            glUniform1iPtr(openGLHandle->GetOpenGLLocation(), *data);
		}
	}

	void CMashOpenGLEffect::SetFloat(MashEffectParamHandle *pHandle, const f32 *data, uint32 count)
	{
		CMashOpenGLEffectParamHandle *openGLHandle = (CMashOpenGLEffectParamHandle*)pHandle;
		if (openGLHandle->GetOpenGLSize() > 1)
		{
            glUniform1fvPtr(openGLHandle->GetOpenGLLocation(), count, data);
		}
		else
		{
            glUniform1fPtr(openGLHandle->GetOpenGLLocation(), *data);
		}
	}

	void CMashOpenGLEffect::SetVector4(MashEffectParamHandle *pHandle, const mash::MashVector4 *data, uint32 count)
	{
		CMashOpenGLEffectParamHandle *openGLHandle = (CMashOpenGLEffectParamHandle*)pHandle;
		switch(openGLHandle->GetOpenGLType())
		{
		case GL_INT_VEC4:
			{
				if (count > 1)
				{
                    glUniform4ivPtr(openGLHandle->GetOpenGLLocation(), count * 4, (GLint*)data->v);
				}
				else
				{
                    glUniform4iPtr(openGLHandle->GetOpenGLLocation(), data->x, data->y, data->z, data->w);
				}
				break;
			}
		case GL_FLOAT_VEC4:
			{
				if (count > 1)
				{
                    glUniform4fvPtr(openGLHandle->GetOpenGLLocation(), count * 4, data->v);
				}
				else
				{
                    glUniform4fPtr(openGLHandle->GetOpenGLLocation(), data->x, data->y, data->z, data->w);
				}
				break;
			}
		default:
            return;
		}
	}

	void CMashOpenGLEffect::SetValue(MashEffectParamHandle *pHandle, const void *pData, uint32 iSizeInBytes)
	{
        
		if (!pData || (iSizeInBytes == 0))
			return;

		CMashOpenGLEffectParamHandle *openGLHandle = (CMashOpenGLEffectParamHandle*)pHandle;

        {
            const CMashOglSharedUniformBuffer *uboSharedData = openGLHandle->GetOpenGLUBO();
            if (!uboSharedData)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "Parameter is not a uniform buffer.", 
                                 "CMashOpenGLEffect::SetValue");
                
                return;
            }
            
            if (uboSharedData->bufferSize < iSizeInBytes)
            {
                MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
                                 "The data passed to a uniform buffer is greater than its defined size.", 
                                 "CMashOpenGLEffect::SetValue");
                
                return;
            }
            //GLenum e1 = glGetError();
            glBindBufferPtr(GL_UNIFORM_BUFFER, uboSharedData->uboIndex);
            
            glBufferSubDataPtr(GL_UNIFORM_BUFFER, 0, iSizeInBytes, pData);//This offers a big speed boost
            
            glBindBufferPtr(GL_UNIFORM_BUFFER, 0);
        }
	}

	
	eMASH_STATUS CMashOpenGLEffect::SetTexture(MashEffectParamHandle *pHandle, MashTexture *pTexture, const MashTextureState *pTextureState)
	{
		if (!pTexture)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING, 
				"No texture pointer given.", 
				"CMashOpenGLEffect::SetTexture");

			return aMASH_OK;
		}

		CMashOpenGLEffectParamHandle *oglHandle = (CMashOpenGLEffectParamHandle*)pHandle;
        glActiveTexturePtr(GL_TEXTURE0+oglHandle->GetEffectTextureIndex());

		GLenum texType = GL_TEXTURE_2D;
		const eRESOURCE_TYPE resType = pTexture->GetType();
		if (resType == aRESOURCE_TEXTURE)
			texType = GL_TEXTURE_2D;
		else if (resType == aRESOURCE_CUBE_TEXTURE)
			texType = GL_TEXTURE_CUBE_MAP;
		else
		{
			//should never happen
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				"Invalid texture resource.", 
				"CMashOpenGLEffect::SetTexture");

			return aMASH_FAILED;
		}

		glBindTexture(texType, ((CMashOpenGLTexture*)pTexture)->GetOpenGLIndex());
        glUniform1iPtr(oglHandle->GetOpenGLLocation(), oglHandle->GetEffectTextureIndex());

		if (pTextureState)
		{
			CMashOpenGLTextureState *oglState = (CMashOpenGLTextureState*)pTextureState;
			const sMashOpenGLTextureStates *oglStateOpts = oglState->GetOpenGLStateData();
			glTexParameteri(texType, GL_TEXTURE_WRAP_S, oglStateOpts->texWrapS);
			glTexParameteri(texType, GL_TEXTURE_WRAP_T, oglStateOpts->texWrapT);

			if (oglStateOpts->magFilter == GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT)
			{
				glTexParameterf(texType, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, oglStateOpts->maxAnistropy);
			}
			else
			{
				glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, oglStateOpts->magFilter);
				glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, oglStateOpts->minFilter);
			}
		}

		return aMASH_OK;
	}

	MashEffectParamHandle* CMashOpenGLEffect::GetParameterByName(const int8 *sName, ePROGRAM_TYPE type)
	{
		std::map<MashStringc, CMashOpenGLEffectParamHandle*>::iterator iter = m_parameters.find(sName);
		if (iter != m_parameters.end())
			return iter->second;

		return 0;
	}

	void CMashOpenGLEffect::_OnUnload(MashMaterialManager *skinManager, const MashRenderInfo *renderInfo)
	{
		return;
	}

	void CMashOpenGLEffect::_OnLoad(MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo)
	{
		if (!m_bIsValid)
			return;

		glUseProgramPtr(m_openGLProgamID);
	}

	void CMashOpenGLEffect::_OnUpdate(MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo)
	{
		const uint32 iAutoParameterCount = m_autoParameters.Size();
		for(uint32 i = 0; i < iAutoParameterCount; ++i)
		{
			pSkinManager->_SetProgramAutoParameter(this, m_autoParameters[i].pParameter, m_autoParameters[i].type, m_autoParameters[i].iIndex);
		}
	}	
}
