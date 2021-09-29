//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OPENGL_EFFECT_H_
#define _C_MASH_OPENGL_EFFECT_H_

#include "MashEffect.h"
#include "CMashOpenGLEffectProgram.h"
#include "CMashOpenGLHelper.h"
#include <map>
#include "MashArray.h"
namespace mash
{
	class CMashOpenGLRenderer;
    class CMashOpenGLSkinManager;
    
	class CMashOglSharedUniformBuffer : public MashReferenceCounter
	{
	public:
		CMashOglSharedUniformBuffer(CMashOpenGLSkinManager *_manager, const MashStringc &_bufferName, GLuint _uboIndex, GLuint _bindingIndex, GLuint _bufferSize);
		~CMashOglSharedUniformBuffer();

        CMashOpenGLSkinManager *manager;
        MashStringc bufferName;
		GLuint uboIndex;
		GLuint bindingIndex;
		GLuint bufferSize;
	};

	class CMashOpenGLEffectParamHandle : public MashEffectParamHandle
	{
    public:
        enum eAUTO_PARAM_BASIC_TYPE
        {
            aAUTO_PARAM_BASIC_FLOAT,
            aAUTO_PARAM_BASIC_INT,
            aAUTO_PARAM_BASIC_BOOL,
            aAUTO_PARAM_BASIC_VEC2,
            aAUTO_PARAM_BASIC_VEC3,
            aAUTO_PARAM_BASIC_VEC4,
            aAUTO_PARAM_BASIC_MAT4X4,
            aAUTO_PARAM_BASIC_NONE
        };
        
        eAUTO_PARAM_BASIC_TYPE m_structParamBaseType;
	private:
		GLuint m_openGLLocation;
		GLenum m_type;
		GLint m_size;
		CMashOglSharedUniformBuffer *m_ubo;
        
		//textures only
		GLenum m_oglTextureSlot;//ie, GL_TEXTURE0
		GLuint m_effectTextureIndex;//ie, the number from _autoSampler0
	public:
		CMashOpenGLEffectParamHandle(GLuint location, GLenum type, GLint size, CMashOglSharedUniformBuffer *ubo = 0):m_openGLLocation(location),
			m_type(type), m_size(size), m_ubo(ubo)
		{
			if (m_ubo)
				m_ubo->Grab();
		}
		~CMashOpenGLEffectParamHandle()
		{
			/*
				Note, to destroy a ubo then have to recreate it again on recompile
				seems pretty horrid. Instead, ubos are keeped alive until the user
				decides its time to start fresh.

				The skin manager always has a copy of a ubo, and a param dropping
				a ubo pointer will bring the ref count down to no less than 1.
			*/
			if (m_ubo)
				m_ubo->Drop();
		}

		GLuint GetOpenGLLocation()const{return m_openGLLocation;}
		GLenum GetOpenGLType()const{return m_type;}
		GLint GetOpenGLSize()const{return m_size;}
		CMashOglSharedUniformBuffer* GetOpenGLUBO()const{return m_ubo;}

		//textures only
		GLenum GetOGLTextureSlot()const{return m_oglTextureSlot;}
		GLuint GetEffectTextureIndex()const{return m_effectTextureIndex;}
		void SetOGLTextureSlot(GLenum s){m_oglTextureSlot = s;}
		void SetEffectTextureIndex(GLuint i){m_effectTextureIndex = i;}
	};
	
	struct sMashOpenGLVertexAttribute
	{
		eVERTEX_DECLUSAGE type;
		GLint location;
		uint32 usageIndex;

		sMashOpenGLVertexAttribute(GLint _location, eVERTEX_DECLUSAGE _type, uint32 _usageIndex):
		location(_location), type(_type), usageIndex(_usageIndex){}
		sMashOpenGLVertexAttribute():location(0), type(aDECLUSAGE_POSITION), usageIndex(0){}
	};

	class CMashOpenGLEffect : public MashEffect
	{
	private:
		struct sAutoParameter
		{
			MashEffectParamHandle *pParameter;
			//MashStringc sName;
			uint32 type;
			uint32 iIndex;
            
            int32 isStructParam;
            int32 structParamBaseType;
            int32 structParamOffset;

			sAutoParameter():pParameter(0), type(0), iIndex(0), isStructParam(false), structParamBaseType(0), structParamOffset(0){}
			sAutoParameter(MashEffectParamHandle *_pParameter, uint32 _type, uint32 _iIndex):pParameter(_pParameter), type(_type), iIndex(_iIndex), isStructParam(false), structParamBaseType(0), structParamOffset(0){}
		};

		struct sProgram
		{
			MashEffectProgram *program;
		};

		CMashOpenGLRenderer *m_pRenderer;
		MashArray<sMashOpenGLVertexAttribute> m_vertexInputAttributes;
		MashArray<sAutoParameter> m_autoParameters;
		std::map<MashStringc, CMashOpenGLEffectParamHandle*> m_parameters;
		CMashOpenGLEffectProgram *m_programs[aPROGRAM_UNKNOWN];
		bool m_bIsValid;
		bool m_bIsCompiled;
		GLuint m_openGLProgamID;
		bool m_isAPIEffect;
		GLenum GetOGLTextureSlotEnum(uint32 index);
	public:
		CMashOpenGLEffect(CMashOpenGLRenderer *renderer);

		~CMashOpenGLEffect();

		MashEffect* CreateIndependentCopy();

		void SetMatrix(MashEffectParamHandle *pHandle, const mash::MashMatrix4 *data, uint32 count = 1);
		void SetInt(MashEffectParamHandle *pHandle, const int32 *data, uint32 count = 1);
		void SetBool(MashEffectParamHandle *pHandle, const bool *data, uint32 count = 1);
		void SetFloat(MashEffectParamHandle *pHandle, const f32 *data, uint32 count = 1);
		void SetVector2(MashEffectParamHandle *pHandle, const mash::MashVector2 *data, uint32 count = 1);
		void SetVector3(MashEffectParamHandle *pHandle, const mash::MashVector3 *data, uint32 count = 1);

		//provides the fastest way to set shader variables
		void SetVector4(MashEffectParamHandle *pHandle, const mash::MashVector4 *data, uint32 count = 1);
		void SetValue(MashEffectParamHandle *pHandle, const void *pData, uint32 iSizeInBytes);

		//doesnt pay any attention to program type
		MashEffectParamHandle* GetParameterByName(const int8 *sName, ePROGRAM_TYPE type);

		bool IsValid()const;
		bool IsCompiled()const;
		eMASH_STATUS _Compile(MashFileManager *fileManager, const MashMaterialManager *skinManager, const sEffectCompileArgs &compileArgs);
		void _OnLoad(MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo);
		void _OnUpdate(MashMaterialManager *pSkinManager, const MashRenderInfo *pRenderInfo);
		void _OnUnload(MashMaterialManager *skinManager, const MashRenderInfo *renderInfo);

		MashEffectProgram* AddProgramCompiled(const int8 *highLevelCode, const int8 *entry, eSHADER_PROFILE profile, const int8 *fileName = 0);
		MashEffectProgram* AddProgram(/*const int8 *highLevelCode, */const int8 *sFileName, const int8 *sntry, eSHADER_PROFILE profile);
		MashEffectProgram* GetProgramByType(ePROGRAM_TYPE type)const;

		eMASH_STATUS SetTexture(MashEffectParamHandle *pHandle, MashTexture *pTexture, const MashTextureState *pTextureState);

		const MashArray<sMashOpenGLVertexAttribute>& GetOpenGLVertexAttributes()const;
	};

	inline const MashArray<sMashOpenGLVertexAttribute>& CMashOpenGLEffect::GetOpenGLVertexAttributes()const
	{
		return m_vertexInputAttributes;
	}

	inline MashEffectProgram* CMashOpenGLEffect::GetProgramByType(ePROGRAM_TYPE type)const
	{
		return m_programs[type];
	}

	inline bool CMashOpenGLEffect::IsValid()const
	{
		return m_bIsValid;
	}

	inline bool CMashOpenGLEffect::IsCompiled()const
	{
		return m_bIsCompiled;
	}
}

#endif