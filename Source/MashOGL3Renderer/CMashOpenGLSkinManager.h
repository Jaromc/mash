//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_OPENGL3_SKIN_MANAGER_H_
#define _C_MASH_OPENGL3_SKIN_MANAGER_H_

#include "MashMaterialManagerIntermediate.h"
#include "MashFileManager.h"
#include <map>
#include <stack>
namespace mash
{
	class CMashOpenGLRenderer;
	class CMashOglSharedUniformBuffer;

	class CMashOpenGLSkinManager : public MashMaterialManagerIntermediate
	{
	private:
		CMashOpenGLRenderer *m_oglRenderer;
		std::map<MashStringc, CMashOglSharedUniformBuffer*> m_sharedUniformBuffers;
		std::stack<uint32> m_freedUniformBufferBindings;
		int32 m_nextUboBindingIndex;
	public:
		CMashOpenGLSkinManager(mash::CMashOpenGLRenderer *pRenderer);
		~CMashOpenGLSkinManager();

		MashEffect* CreateEffect();

		bool IsProfileSupported(eSHADER_PROFILE profile)const;

		eSHADER_PROFILE GetLatestVertexProfile()const;
		eSHADER_PROFILE GetLatestFragmentProfile()const;
		eSHADER_PROFILE GetLatestGeometryProfile()const;

		/*
			The returned buffer MUST be dropped when done
		*/
		CMashOglSharedUniformBuffer* GetSharedUniformBuffer(const int8 *name, uint32 bufferSize);
        void _OnSharedUniformBufferDelete(CMashOglSharedUniformBuffer *buffer);

		const int8* _GetAPIShaderHeader();
	};
}

#endif