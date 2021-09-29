//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashCompileSettings.h"

#ifdef MASH_WINDOWS
#include "CMashD3D10EffectProgram.h"
#include "MashHelper.h"
#include "MashFileStream.h"
#include "MashMaterialManager.h"
#include "MashLog.h"

#include "windows.h"
#include <D3DX10Async.h>

namespace mash
{
	CMashD3D10EffectParamHandle::CMashD3D10EffectParamHandle(const int8 *name, 
		eMASH_D3D10_EFFECT_PARAM_TYPE type,
		uint32 offset, 
		uint32 size, 
		ePROGRAM_TYPE ownerType,
		uint8 *parentBuffer):MashEffectParamHandle(), m_name(name), m_type(type),
		m_startOffset(offset), m_size(size), m_parentBuffer(parentBuffer), m_ownerType(ownerType),
		m_samplerSlotIndex(0)
	{
		
	}

	void CMashD3D10EffectParamHandle::OnSet(const void *data, uint32 size)
	{
		/*
			make sure the data size is no greater than the param size.
			This allows for part of an array to be set.
		*/
		if (size > m_size)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, 
				"CMashD3D10Effect::OnSet",
				"Data sent to shader constant buffer (%d) is greater than the variable size (%d).", size, m_size);

			size = m_size;
		}

		memcpy(&m_parentBuffer[m_startOffset], data, size);
	}

	void CMashD3D10EffectParamHandle::OnSetScalarArray(const void *data, uint32 elementSize, uint32 elementCount)
	{
		/*
			make sure the data size is no greater than the param size.
			This allows for part of an array to be set.
		*/
		if ((elementSize * elementCount) > m_size)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, 
				"CMashD3D10Effect::OnSetScalarArray",
				"Data sent to shader constant buffer (%d) is greater than the variable size (%d).", (elementSize * elementCount), m_size);

			return;
		}

		if ((elementCount > 1) && (elementSize != 16))
		{
			//each element in an array will be padded to the 16byte boundary
			for(uint32 i = 0; i < elementCount; ++i)
			{
				memcpy(&m_parentBuffer[m_startOffset + (i * 16)], &((const int8*)data)[i * elementSize], elementSize);
			}
		}
		else
		{
			memcpy(&m_parentBuffer[m_startOffset], data, elementSize * elementCount);
		}
	}

	CMashD3D10EffectProgram::MashD3D10IncludeImpl::MashD3D10IncludeImpl(MashFileManager *fileManager):m_fileManager(fileManager)
	{
	}

	HRESULT CMashD3D10EffectProgram::MashD3D10IncludeImpl::Open(D3D10_INCLUDE_TYPE includeType,
				LPCSTR fileName,
				LPCVOID parentData,
				LPCVOID *data,
				UINT *bytes)
	{
		MashFileStream *pFileStream = m_fileManager->CreateFileStream();
		if (pFileStream->LoadFile(fileName, aFILE_IO_TEXT) == aMASH_FAILED)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to include shader data.", "CMashD3D10Effect::MashD3D10IncludeImpl::Open");
			pFileStream->Destroy();
			return S_FALSE;
		}

		const uint32 fileSize = pFileStream->GetDataSizeInBytes();
		if (fileSize > 0)
		{
			/*
				Remove the null terminator otherwise this will
				end the whole shader file
			*/
			const int8 *text = (const int8*)pFileStream->GetData();
			uint32 textLen = strlen(text);
			*data = (int8*)MASH_ALLOC_COMMON(textLen);
			memcpy((int8*)*data, text, textLen);
			*bytes = textLen;
		}
		

		pFileStream->Destroy();

		return S_OK;
	}

	HRESULT CMashD3D10EffectProgram::MashD3D10IncludeImpl::Close(LPCVOID data)
	{
		uint8 *dataCast = (uint8*)data;
		if (dataCast)
			MASH_FREE(dataCast);

		return S_OK;
	}

	CMashD3D10EffectProgram::CMashD3D10EffectProgram(ID3D10Device *pDevice,
			ePROGRAM_TYPE programType,
			const int8 *compiledCode,
			const int8 *sFileName, 
			const int8 *sEntry, 
			eSHADER_PROFILE profile):MashEffectProgramIntermediate(programType, compiledCode, sFileName, sEntry, profile),
			m_pD3D10VertexShader(0), m_pD3D10Device(pDevice),
			m_pShaderBlob(0)
	{
		if (m_pD3D10Device)
		{
			m_pD3D10Device->AddRef();
		}
	}

	CMashD3D10EffectProgram::~CMashD3D10EffectProgram()
	{
		ClearOldData();

		if (m_pD3D10Device)
		{
			m_pD3D10Device->Release();
			m_pD3D10Device = 0;
		}
	}

	void CMashD3D10EffectProgram::ClearOldData()
	{
		MashArray<sAutoParameter>::Iterator paramIter = m_autoParameters.Begin();
		MashArray<sAutoParameter>::Iterator paramIterEnd = m_autoParameters.End();
		for(; paramIter != paramIterEnd; ++paramIter)
		{
			if (paramIter->pParameter)
			{
				paramIter->pParameter->Drop();
				paramIter->pParameter = 0;
			}
		}

		m_autoParameters.Clear();

		MashArray<sConstantBuffer>::Iterator cbIter = m_constantBuffers.Begin();
		MashArray<sConstantBuffer>::Iterator cbIterEnd = m_constantBuffers.End();
		for(; cbIter != cbIterEnd; ++cbIter)
		{
			if (cbIter->buffer)
			{
				MASH_FREE(cbIter->buffer);
				cbIter->buffer = 0;
				cbIter->bufferSize = 0;
				cbIter->bufferSlot = 0;
			}

			if (cbIter->d3DBuffer)
			{
				cbIter->d3DBuffer->Release();
				cbIter->d3DBuffer = 0;
			}
		}

		m_constantBuffers.Clear();

		std::map<MashStringc, CMashD3D10EffectParamHandle*>::iterator cbvIter = m_constantBufferVariables.begin();
		std::map<MashStringc, CMashD3D10EffectParamHandle*>::iterator cbvIterEnd = m_constantBufferVariables.end();
		for(; cbvIter != cbvIterEnd; ++cbvIter)
		{
			if (cbvIter->second)
				cbvIter->second->Drop();
		}

		m_constantBufferVariables.clear();

		if (m_pD3D10VertexShader)
		{
			m_pD3D10VertexShader->Release();
			m_pD3D10VertexShader = 0;
		}

		if (m_pD3D10PixelShader)
		{
			m_pD3D10PixelShader->Release();
			m_pD3D10PixelShader = 0;
		}

		if (m_pShaderBlob)
		{
			m_pShaderBlob->Release();
			m_pShaderBlob = 0;
		}
	}

	eMASH_STATUS CMashD3D10EffectProgram::_Compile(MashFileManager *pFileManager, 
			const MashMaterialManager *pSkinManager, 
			const sEffectMacro *customMacros, 
			uint32 customMacroCount,
			bool &isValid)
	{
		isValid = false;

		if (m_programType ==  aPROGRAM_UNKNOWN)
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_WARNING, "Effect program type is unknown.", "CMashD3D10Effect::_Compile");
			return aMASH_OK;
		}

		if (!pSkinManager->IsProfileSupported(m_profile))
			return aMASH_OK; //no error

		/*
			Clear any old loaded data
		*/
		ClearOldData();

		/*
			If no shader source has been set then its assumed the data in the file
			is in native format
		*/
		bool isNative = false;
		if (m_compiledShader.Empty() && !m_fileName.Empty())
		{
			isNative = true;

			MashFileStream *pFileStream = pFileManager->CreateFileStream();
			if (pFileStream->LoadFile(m_fileName.GetCString(), aFILE_IO_TEXT) == aMASH_FAILED)
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to read program source", "CMashD3D10EffectProgram::_Compile");
				pFileStream->Destroy();
				return aMASH_FAILED;
			}

			if (pFileStream->GetDataSizeInBytes() > 0)
				m_compiledShader = (const int8*)pFileStream->GetData();

			m_compiledEntry = m_entry;
			m_compiledFileName = m_fileName;

			pFileStream->Destroy();
			pFileStream = 0;
		}

		if (m_compiledShader.Empty())
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "No program source set.", "CMashD3D10EffectProgram::_Compile");
			return aMASH_FAILED;
		}

		DWORD flags = 0;
#ifdef MASH_DEBUG
		flags |= D3D10_SHADER_DEBUG;
#endif

		flags |= D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY;

		D3D10_SHADER_MACRO *macros = 0;

		/*
			Only native shaders have macros set here. Effects have theres added to the
			shader source when built.
		*/
		if (isNative)
		{
			//build macro list
			const uint32 builtInMacroCount = m_compileArgs.Size();
			const uint32 totalMacroCount = customMacroCount + builtInMacroCount;

			
			if (totalMacroCount > 0)
			{
				macros = MASH_ALLOC_T_COMMON(D3D10_SHADER_MACRO, totalMacroCount + 1);
				uint32 currentMacro = 0;
				for(uint32 i = 0; i < builtInMacroCount; ++i)
				{
					macros[currentMacro].Name = m_compileArgs[i].name.GetCString();
					if (!m_compileArgs[i].definition.Empty())
						macros[currentMacro].Definition = m_compileArgs[i].definition.GetCString();
					else
						macros[currentMacro].Definition = 0;
					++currentMacro;
				}

				for(uint32 i = 0; i < customMacroCount; ++i)
				{
					macros[currentMacro].Name = customMacros[i].name.GetCString();
					if (!customMacros[i].definition.Empty())
						macros[currentMacro].Definition = customMacros[i].definition.GetCString();
					else
						macros[currentMacro].Definition = 0;

					++currentMacro;
				}

				//NULL terminate
				macros[totalMacroCount].Name = 0;
				macros[totalMacroCount].Definition = 0;
			}
		}

		ID3D10Blob *pErrorMsgs = 0;

		HRESULT hr =  D3DX10CompileFromMemory(m_compiledShader.GetCString(),
			m_compiledShader.Size(),
			m_fileName.GetCString(),
			macros,
			&CMashD3D10EffectProgram::MashD3D10IncludeImpl(pFileManager),
			m_compiledEntry.GetCString(),
			helpers::GetShaderProfileString(m_profile),
			flags,
			0,
			0,
			&m_pShaderBlob,
			&pErrorMsgs,
			0);

		if (FAILED(hr))
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR,
					"CMashD3D10EffectProgram::_Compile",
					"Failed to compile effect: '%s', generated effect: '%s'.", m_fileName.GetCString(), m_compiledFileName.GetCString());

			if (pErrorMsgs && pErrorMsgs->GetBufferPointer())
			{
				MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, (const int8*)pErrorMsgs->GetBufferPointer(), "CMashD3D10EffectProgram::_Compile");
				pErrorMsgs->Release();
			}

            MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR,
                             m_compiledShader.GetCString(),
                            "CMashD3D10EffectProgram::_Compile");

            m_compiledShader.FreeMemory();
            m_compiledEntry.FreeMemory();

			return aMASH_FAILED;
		}

        m_compiledShader.FreeMemory();
        m_compiledEntry.FreeMemory();

		if (macros)
			MASH_FREE(macros);

		HRESULT result = S_OK;

		switch(m_programType)
		{
		case aPROGRAM_VERTEX:
			{
				result = m_pD3D10Device->CreateVertexShader((DWORD*)m_pShaderBlob->GetBufferPointer(),
					m_pShaderBlob->GetBufferSize(),
					&m_pD3D10VertexShader);

				break;
			}
		case aPROGRAM_PIXEL:
			{
				result = m_pD3D10Device->CreatePixelShader((DWORD*)m_pShaderBlob->GetBufferPointer(),
					m_pShaderBlob->GetBufferSize(),
					&m_pD3D10PixelShader);

				break;
			}
		};

		if (FAILED(result))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to create shader object from D3D.", "CMashD3D10EffectProgram::_Compile");
			return aMASH_FAILED;
		}

		ID3D10ShaderReflection *reflectShader = 0;
		if (FAILED(D3D10ReflectShader(m_pShaderBlob->GetBufferPointer(), m_pShaderBlob->GetBufferSize(), &reflectShader)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to get shader constants from D3D.", "CMashD3D10EffectProgram::_Compile");
			return aMASH_FAILED;
		}

		D3D10_SHADER_DESC shaderDesc;
		if (FAILED(reflectShader->GetDesc(&shaderDesc)))
		{
			MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, "Failed to get shader description from D3D.", "CMashD3D10EffectProgram::_Compile");
			return aMASH_FAILED;
		}

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
				"CMashD3D10EffectProgram::_Compile",
				"Adding auto parameters to effect program '%s'...", GetFileName().GetCString());

		uint32 semanticTypes = 0;
		uint32 iSemanticIndex = 0;

		D3D10_SHADER_BUFFER_DESC constDesc;
		D3D10_SHADER_INPUT_BIND_DESC shaderInputBindDesc;
		D3D10_SHADER_VARIABLE_DESC varDesc;
		D3D10_SHADER_TYPE_DESC varTypeDesc;
		int8 newParamName[256];
		std::map<MashStringc, uint32> samplerMap;
		for(uint32 res = 0; res < shaderDesc.BoundResources; ++res)
		{
			reflectShader->GetResourceBindingDesc(res, &shaderInputBindDesc);

			if (shaderInputBindDesc.Type == D3D10_SIT_CBUFFER || 
				shaderInputBindDesc.Type == D3D10_SIT_TBUFFER)
			{
				for(uint32 bufferIndex = 0; bufferIndex < shaderDesc.ConstantBuffers; ++bufferIndex)
				{
					ID3D10ShaderReflectionConstantBuffer *shaderConstant = reflectShader->GetConstantBufferByIndex(bufferIndex);
				
					shaderConstant->GetDesc(&constDesc);

					if (!(constDesc.Size == 0) || (constDesc.Variables == 0))
					{
						//scalar buffer
						if (constDesc.Type == D3D10_CT_CBUFFER)
						{
							sConstantBuffer newBuffer;
							
							D3D10_BUFFER_DESC cbDesc;
							cbDesc.ByteWidth = constDesc.Size;
							cbDesc.Usage = D3D10_USAGE_DYNAMIC;
							cbDesc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
							cbDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
							cbDesc.MiscFlags = 0;
							if (SUCCEEDED(m_pD3D10Device->CreateBuffer(&cbDesc, 0, &newBuffer.d3DBuffer)))
							{		
								newBuffer.buffer = (uint8*)MASH_ALLOC_COMMON(constDesc.Size);
								newBuffer.bufferSize = constDesc.Size;
								newBuffer.bufferSlot = bufferIndex;

								m_constantBuffers.PushBack(newBuffer);
							
								for(uint32 var = 0; var < constDesc.Variables; ++var)
								{
									ID3D10ShaderReflectionVariable *variable = shaderConstant->GetVariableByIndex(var);
									variable->GetDesc(&varDesc);

									ID3D10ShaderReflectionType *varType = variable->GetType();
									varType->GetDesc(&varTypeDesc);

									eMASH_D3D10_EFFECT_PARAM_TYPE engineParamType = aD3D10_EFFECT_PARAM_UNKNOWN;
									switch(varTypeDesc.Class)
									{
									case D3D10_SVC_SCALAR:
											engineParamType = aD3D10_EFFECT_PARAM_SCALAR;
											break;
									case D3D10_SVC_VECTOR:
											engineParamType = aD3D10_EFFECT_PARAM_VECTOR;
											break;
									case D3D10_SVC_MATRIX_ROWS:
									case D3D10_SVC_MATRIX_COLUMNS:
										engineParamType = aD3D10_EFFECT_PARAM_MATRIX;
										break;
									case D3D10_SVC_STRUCT:
										engineParamType = aD3D10_EFFECT_PARAM_STRUCT;
										break;
									default:
										engineParamType = aD3D10_EFFECT_PARAM_UNKNOWN;
										break;
									};

									if (engineParamType != aD3D10_EFFECT_PARAM_UNKNOWN)
									{
										CMashD3D10EffectParamHandle *newParam = MASH_NEW_COMMON CMashD3D10EffectParamHandle(varDesc.Name, engineParamType,
											varDesc.StartOffset, varDesc.Size, m_programType, newBuffer.buffer);

										m_constantBufferVariables.insert(std::make_pair(varDesc.Name, newParam));

										bool isPartOfStruct = false;
										helpers::GetAutoEffectParameterName(varDesc.Name, newParamName, iSemanticIndex, isPartOfStruct);
										if (pSkinManager->IsAutoParameter(newParamName, semanticTypes) != aMASH_FAILED)
										{
											MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
												"CMashD3D10EffectProgram::_Compile",
												"Adding auto parameter '%s' size '%d'.", varDesc.Name, varDesc.Size);

											m_autoParameters.PushBack(sAutoParameter(newParam, semanticTypes, iSemanticIndex));
											newParam->Grab();
										}
									}
									else
									{
										if (varDesc.Name)
										{
											MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
												"CMashD3D10EffectProgram::_Compile",
												"Shader constant '%s' has invalid type .", varDesc.Name);
										}
										else
										{
											MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
												"Invalid shader constant type.", 
												"CMashD3D10EffectProgram::_Compile");
										}
									}
								}
							}
						}
						else
						{
							//texture buffer
							eMASH_D3D10_EFFECT_PARAM_TYPE engineParamType = aD3D10_EFFECT_PARAM_TEXTURE;
							for(uint32 var = 0; var < constDesc.Variables; ++var)
							{
								ID3D10ShaderReflectionVariable *variable = shaderConstant->GetVariableByIndex(var);
								variable->GetDesc(&varDesc);

								ID3D10ShaderReflectionType *varType = variable->GetType();
								varType->GetDesc(&varTypeDesc);

								CMashD3D10EffectParamHandle *newParam = MASH_NEW_COMMON CMashD3D10EffectParamHandle(varDesc.Name, aD3D10_EFFECT_PARAM_TEXTURE,
										varDesc.StartOffset, varDesc.Size, m_programType, 0);

								m_constantBufferVariables.insert(std::make_pair(varDesc.Name, newParam));

								//if (IsAutoEffectParameter(varDesc.Name, semanticTypes, iSemanticIndex))
								bool isPartOfStruct = false;
								helpers::GetAutoEffectParameterName(varDesc.Name, newParamName, iSemanticIndex, isPartOfStruct);
								if (pSkinManager->IsAutoParameter(newParamName, semanticTypes) != aMASH_FAILED)
								{
									MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
												"CMashD3D10EffectProgram::_Compile",
												"Adding auto parameter texture '%s'.", varDesc.Name);

									m_autoParameters.PushBack(sAutoParameter(newParam, semanticTypes, iSemanticIndex));
									newParam->Grab();
								}
							}
						}
					}
				}
			}
			else if (shaderInputBindDesc.Type == D3D10_SIT_TEXTURE)
			{
				CMashD3D10EffectParamHandle *newParam = MASH_NEW_COMMON CMashD3D10EffectParamHandle(shaderInputBindDesc.Name, aD3D10_EFFECT_PARAM_TEXTURE,
					shaderInputBindDesc.BindPoint, shaderInputBindDesc.BindCount, m_programType, 0);
				
				std::map<MashStringc, uint32>::iterator samplerIter = samplerMap.find(shaderInputBindDesc.Name);
				if (samplerIter != samplerMap.end())
				{
					newParam->m_samplerSlotIndex = samplerIter->second;
				}
				else
				{
					//should not happen
					newParam->m_samplerSlotIndex = 0;
					
					if (varDesc.Name)
					{
						MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
							"CMashD3D10EffectProgram::_Compile",
							"Texture '%s' has invalid texture sampler slot.", shaderInputBindDesc.Name);
					}
					else
					{
						MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
								"Invalid texture sampler slot.", 
								"CMashD3D10EffectProgram::_Compile");
					}
				}

				m_constantBufferVariables.insert(std::make_pair(shaderInputBindDesc.Name, newParam));

				//if (IsAutoEffectParameter(shaderInputBindDesc.Name, semanticTypes, iSemanticIndex))
				bool isPartOfStruct = false;
				helpers::GetAutoEffectParameterName(shaderInputBindDesc.Name, newParamName, iSemanticIndex, isPartOfStruct);
				if (pSkinManager->IsAutoParameter(newParamName, semanticTypes) != aMASH_FAILED)
				{
					MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_INFORMATION, 
										"CMashD3D10EffectProgram::_Compile",
										"Adding auto parameter texture '%s'.", shaderInputBindDesc.Name);

					m_autoParameters.PushBack(sAutoParameter(newParam, semanticTypes, iSemanticIndex));
					newParam->Grab();
				}
			}
			else if (shaderInputBindDesc.Type == D3D10_SIT_SAMPLER)
			{
				/*
					note, samplers seem to load before textures. So we just generate a list of samplers
					and map them to the textures later
				*/
				samplerMap[shaderInputBindDesc.Name] = shaderInputBindDesc.BindPoint;
			}
			else
			{
				//MASH_WRITE_TO_LOG(MashLog::aERROR_LEVEL_ERROR, 
				//						"Invalid shader resource type.", 
				//						"CMashD3D9Effect::_CompileProgram");
			}
		}		

		if (reflectShader)
			reflectShader->Release();

		/*
			Only vertex shaders need to hold the shader blob so we
			can create vertex declarations.
		*/
		if ((m_programType != aPROGRAM_VERTEX) && m_pShaderBlob)
		{
			m_pShaderBlob->Release();
			m_pShaderBlob = 0;
		}

		isValid = true;

		return aMASH_OK;
	}

	MashEffectParamHandle* CMashD3D10EffectProgram::GetParameterByName(const int8 *sName)
	{
		std::map<MashStringc, CMashD3D10EffectParamHandle*>::iterator iter = m_constantBufferVariables.find(sName);
		if (iter != m_constantBufferVariables.end())
			return iter->second;

		MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, 
				"CMashD3D10EffectProgram::GetParameterByName",
				"Parameter '%s' does not exist in this effect.", sName);

		return 0;
	}

	void CMashD3D10EffectProgram::_OnLoad(MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo)
	{
		switch(m_programType)
		{
		case aPROGRAM_VERTEX:
			{
				m_pD3D10Device->VSSetShader(m_pD3D10VertexShader);
				break;
			}
		case aPROGRAM_PIXEL:
			{
				m_pD3D10Device->PSSetShader(m_pD3D10PixelShader);
				break;
			}
		};
	}

	void CMashD3D10EffectProgram::_OnUpdate(MashEffect *owner, MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo)
	{
		const uint32 iAutoParameterCount = m_autoParameters.Size();
		for(uint32 i = 0; i < iAutoParameterCount; ++i)
		{
			pSkinManager->_SetProgramAutoParameter(owner, m_autoParameters[i].pParameter, m_autoParameters[i].type, m_autoParameters[i].iIndex);
		}

		/*
			TODO : User callback?
		*/
		
		const uint32 bufferCount = m_constantBuffers.Size();
		void *data = 0;
		for(uint32 i = 0; i < bufferCount; ++i)
		{
			sConstantBuffer *buffer = &m_constantBuffers[i];
			buffer->d3DBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, &data);
			memcpy(data, buffer->buffer, buffer->bufferSize);
			buffer->d3DBuffer->Unmap();

			switch(m_programType)
			{
			case aPROGRAM_VERTEX:
				{
					m_pD3D10Device->VSSetConstantBuffers(buffer->bufferSlot, 1, &buffer->d3DBuffer);
					break;
				}
			case aPROGRAM_PIXEL:
				{
					m_pD3D10Device->PSSetConstantBuffers(buffer->bufferSlot, 1, &buffer->d3DBuffer);
					break;
				}
			};
		}
	}

	ID3D10Blob* CMashD3D10EffectProgram::GetProgramIASignature()
	{
		if (!m_pShaderBlob || (m_programType != aPROGRAM_VERTEX))
			return 0;

		return m_pShaderBlob;
	}
}

#endif
