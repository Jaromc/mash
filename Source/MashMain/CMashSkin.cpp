//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashSkin.h"
#include "MashSceneManager.h"
#include "MashDevice.h"
#include "MashTimer.h"
#include "MashBone.h"
#include "MashLog.h"
namespace mash
{
	MashBone* CMashSkin::FindNewInstance(const MashBone *originalBone, MashSceneNode *root)
	{
		if ((root != originalBone) && 
			(root->GetNodeType() == aNODETYPE_BONE) &&
			(((MashBone*)root)->GetSourceBoneSceneId() == ((MashBone*)originalBone)->GetBoneSceneId()))
		{
			return (MashBone*)root;
		}

		MashList<MashSceneNode*>::ConstIterator iter = root->GetChildren().Begin();
		MashList<MashSceneNode*>::ConstIterator end = root->GetChildren().End();
		for(; iter != end; ++iter)
		{
			MashBone *newBoneInstance = FindNewInstance(originalBone, *iter);
			if (newBoneInstance)
				return newBoneInstance;
		}

		return 0;
	}

	CMashSkin::CMashSkin(MashVideo *renderer):MashSkin(), m_renderer(renderer), m_bonePaletteSize(0),
		m_lastRenderFrame(mash::math::MaxUInt32())
	{
		
	}

	CMashSkin::~CMashSkin()
	{
		MashArray<sBone>::Iterator boneIter = m_skinningBones.Begin();
		MashArray<sBone>::Iterator boneIterEnd = m_skinningBones.End();
		for(; boneIter != boneIterEnd; ++boneIter)
		{
			if (boneIter->node)
			{
				boneIter->node->Drop();
				boneIter->node = 0;
			}
		}
	}

	MashSkin* CMashSkin::CreateInstance(MashSceneNode *root)
	{
		MashSkin *newSkinInstance = MashDevice::StaticDevice->GetSceneManager()->CreateSkin();

        newSkinInstance->FillFrom(this, root);

		return newSkinInstance;
	}
    
    void CMashSkin::FillFrom(MashSkin *original, MashSceneNode *newInstanceRoot)
    {
        if (!original || !newInstanceRoot)
            return;
        
        m_skinningBones.Clear();
        m_bonePaletteSize = 0;
        
        MashArray<sBone>::ConstIterator boneIter = original->GetBones().Begin();
		MashArray<sBone>::ConstIterator boneIterEnd = original->GetBones().End();
		for(; boneIter != boneIterEnd; ++boneIter)
		{
			MashBone *newBoneInstance = FindNewInstance(boneIter->node, newInstanceRoot);
			if (newBoneInstance)
			{
				AddBone(newBoneInstance, boneIter->id);
			}
			else
			{
                MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, "CMashSkin::FillFrom",
                                    "Failed to find new instance of bone '%s' in root node given.", 
                                    boneIter->node->GetNodeName().GetCString());
			}
		}
    }

	void CMashSkin::AddBone(MashBone *bone, uint32 boneId)
	{
		if (!bone)
			return;

		if (boneId > 256)
		{
            MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, "CMashSkin::AddBone",
                                "Bone '%s' has an id greater than 256 and the current bone limit is 256. Reduce the number of bones and make sure their ids increment from 0 -> boneCount -1",
                                boneIter->node->GetNodeName().GetCString());
		}

		uint32 tempBonePaletteSize = boneId+1;

		if (tempBonePaletteSize > m_bonePaletteSize)
			m_bonePaletteSize = tempBonePaletteSize;

		m_renderer->GetRenderInfo()->SetBonePaletteMinimumSize(m_bonePaletteSize);
		
		sBone newSkinningBone;
		newSkinningBone.node = bone;
		newSkinningBone.id = boneId;
		m_skinningBones.PushBack(newSkinningBone);

		bone->Grab();
	}

	void CMashSkin::OnRender()
	{
		/*
			This assumes the bone palette has already been initialised to at least
			the length of the bone count.
		*/
		const uint32 currentFrameTime = MashDevice::StaticDevice->GetTimer()->GetFrameCount();
		/*
			This maybe processed twice in the case of shadow + scene rendering. Or many times
			if this skin is being reused for some special reason. This frame check makes sure we 
			aren't doing things twice per frame.
		*/
		if (m_lastRenderFrame != currentFrameTime)
		{
            mash::MashMatrix4 *bonePalette = m_renderer->GetRenderInfo()->GetBonePalette();
            m_renderer->GetRenderInfo()->SetCurrentBonePaletteSize(m_bonePaletteSize);
            m_renderer->GetRenderInfo()->SetSkin(this);
            uint32 boneCount = m_skinningBones.Size();
            for(uint32 i = 0; i < boneCount; ++i)
            {
                m_skinningBones[i].node->GetWorldSkinningOffset(bonePalette[m_skinningBones[i].id]);
            }

			m_lastRenderFrame = currentFrameTime;
		}
	}
}	