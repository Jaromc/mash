//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_D3D10_EFFECT_PROGRAM_H_
#define _C_MASH_D3D10_EFFECT_PROGRAM_H_

#include "MashDataTypes.h"

#ifdef MASH_WINDOWS
#include "MashEffectProgramIntermediate.h"
#include "MashFileManager.h"
#include "MashMaterialManager.h"
#include "windows.h"
#include <map>
#include <d3d10_1.h>
#include <d3d10shader.h>
namespace mash
{
	class MashEffect;
	class MashMaterialManager;
	class MashRenderInfo;

	enum eMASH_D3D10_EFFECT_PARAM_TYPE
	{
		aD3D10_EFFECT_PARAM_SCALAR,
		aD3D10_EFFECT_PARAM_VECTOR,
		aD3D10_EFFECT_PARAM_MATRIX,
		aD3D10_EFFECT_PARAM_STRUCT,
		aD3D10_EFFECT_PARAM_TEXTURE,
		aD3D10_EFFECT_PARAM_UNKNOWN
	};

	class CMashD3D10EffectParamHandle : public MashEffectParamHandle
	{
	public:
		MashStringc m_name;
		eMASH_D3D10_EFFECT_PARAM_TYPE m_type;
		uint32 m_startOffset;
		uint32 m_size;
		uint8 *m_parentBuffer;
		ePROGRAM_TYPE m_ownerType;
		uint32 m_samplerSlotIndex;
	public:
		CMashD3D10EffectParamHandle(const int8 *name, 
			eMASH_D3D10_EFFECT_PARAM_TYPE type,
			uint32 offset, 
			uint32 size,
			ePROGRAM_TYPE ownerType,
			uint8 *parentBuffer);

		~CMashD3D10EffectParamHandle(){}

		void OnSet(const void *data, uint32 size);
		
		/*
			This makes sure each element in an array is aligned to the 16byte boundary.
			Otherwise the data will not be accessed correctly.
			Padding will be added to vec2 and vec3s.
		*/
		void OnSetScalarArray(const void *data, uint32 elementSize, uint32 elementCount);
		eMASH_D3D10_EFFECT_PARAM_TYPE GetType()const{return m_type;}
		ePROGRAM_TYPE GetOwnerType()const{return m_ownerType;}
		//uint32 GetOffset()const{return m_startOffset;}
	};

	class CMashD3D10EffectProgram : public MashEffectProgramIntermediate
	{
	private:
		class MashD3D10IncludeImpl : public ID3D10Include
		{
		private:
			MashFileManager *m_fileManager;
		public:
			MashD3D10IncludeImpl(MashFileManager *fileManager);
			~MashD3D10IncludeImpl(){}

			STDMETHOD(Open)(D3D10_INCLUDE_TYPE includeType,
				LPCSTR fileName,
				LPCVOID parentData,
				LPCVOID *data,
				UINT *bytes);

			STDMETHOD(Close)(LPCVOID data);


		};
	private:
		struct sAutoParameter
		{
			MashEffectParamHandle *pParameter;
			//MashStringc sName;
			uint32 type;
			uint32 iIndex;

			sAutoParameter():pParameter(0), type(0), iIndex(0){}
			sAutoParameter(MashEffectParamHandle *_pParameter, uint32 _type, uint32 _iIndex):pParameter(_pParameter), type(_type)/*, sName(_sName)*/, iIndex(_iIndex){}
		};

		union
		{
			ID3D10VertexShader *m_pD3D10VertexShader;
			ID3D10PixelShader *m_pD3D10PixelShader;
		};

		struct sConstantBuffer
		{
			uint8 *buffer;
			ID3D10Buffer *d3DBuffer;
			uint32 bufferSize;
			uint32 bufferSlot;

			sConstantBuffer():buffer(0), d3DBuffer(0), bufferSize(0), bufferSlot(0){}
			sConstantBuffer(const sConstantBuffer &c):buffer(c.buffer), d3DBuffer(c.d3DBuffer), bufferSize(c.bufferSize), bufferSlot(c.bufferSlot){}
		};

		MashArray<sConstantBuffer> m_constantBuffers;
		std::map<MashStringc, CMashD3D10EffectParamHandle*> m_constantBufferVariables;

		ID3D10Blob *m_pShaderBlob;
		ID3D10Device *m_pD3D10Device;
		MashArray<sAutoParameter> m_autoParameters;

		void ClearOldData();
	public:
		CMashD3D10EffectProgram(ID3D10Device *pDevice,
			ePROGRAM_TYPE programType,
			const int8 *compiledCode,
			const int8 *sFileName,
			const int8 *sEntry, 
			eSHADER_PROFILE profile);

		~CMashD3D10EffectProgram();

		MashEffectParamHandle* GetParameterByName(const int8 *sName);

		eMASH_STATUS _Compile(MashFileManager *pFileManager, const MashMaterialManager *pSkinManagers, const sEffectMacro *macros, 
			uint32 macroCount, bool &isValid);

		void _OnLoad(MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo);
		void _OnUpdate(MashEffect *owner, MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo);

		ID3D10Blob* GetProgramIASignature();
	};
}

#endif

#endif