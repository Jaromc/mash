//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OPENGL_TEXTURE_STATE_H_
#define _C_MASH_OPENGL_TEXTURE_STATE_H_

#include "MashTexture.h"
#include "CMashOpenGLHeader.h"
namespace mash
{
	struct sMashOpenGLTextureStates
	{
		GLenum minFilter;
		GLenum magFilter;
		GLenum texWrapS;
		GLenum texWrapT;
		float maxAnistropy;
	};

	class CMashOpenGLTextureState : public MashTextureState
	{
	private:
		sMashOpenGLTextureStates m_openGLStateData;
	public:
		CMashOpenGLTextureState(const sSamplerState &_stateData, const sMashOpenGLTextureStates &openStateData):
		  MashTextureState(_stateData), m_openGLStateData(openStateData){}
		~CMashOpenGLTextureState(){}

		const sMashOpenGLTextureStates* GetOpenGLStateData()const{return &m_openGLStateData;}
	};
}

#endif