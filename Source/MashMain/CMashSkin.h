//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_SKIN_H_
#define _C_MASH_SKIN_H_

#include "MashSkin.h"
#include "MashVideo.h"
namespace mash
{
	class CMashSkin : public MashSkin
	{
	public:
		MashVideo *m_renderer;
		MashArray<sBone> m_skinningBones;
		uint32 m_bonePaletteSize;
		uint32 m_lastRenderFrame;

		MashBone* FindNewInstance(const MashBone *originalBone, MashSceneNode *root);
	public:
		CMashSkin(MashVideo *renderer);
		~CMashSkin();

        /*
         This function will search the root node to find bones of the
         same name within this skin. Bones found will be used in the
         new instance.
         
         If multiple nodes share the same name from this root node then result will most
         likely not be correct. Also, if the same bone pointers are found in the 
         root node then they will not be added to the new instance.
         
         Note, depending on the number of bones and the size of the root hierarchy, this 
         function may be slow. If this function is to be used alot after intialization and
         you have prior knowledge of the scene then it may be better to create an
         instance manually.
         */
		MashSkin* CreateInstance(MashSceneNode *root);
        void FillFrom(MashSkin *original, MashSceneNode *newInstanceRoot);

		uint32 GetBonePaletteLength()const;
		const MashArray<sBone>& GetBones()const;
		void AddBone(MashBone *bone, uint32 boneId);
		void OnRender();
	};

	inline uint32 CMashSkin::GetBonePaletteLength()const
	{
		return m_bonePaletteSize;
	}

	inline const MashArray<MashSkin::sBone>& CMashSkin::GetBones()const
	{
		return m_skinningBones;
	}
}

#endif