//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_DUMMY_H_
#define _C_MASH_DUMMY_H_

#include "MashDummy.h"

namespace mash
{
	class CMashDummy : public MashDummy
	{
	private:
		struct sSerialize
		{
			mash::MashAABB bounds;
		};
	private:
		mash::MashAABB m_boundingBox;
	public:
		CMashDummy(MashSceneNode *pParent,
			mash::MashSceneManager *pManager,
			const MashStringc &sName);

		virtual ~CMashDummy();

		void SetBoundingBox(const mash::MashAABB &bounds);

		uint32 GetNodeType()const;
		const mash::MashAABB& GetLocalBoundingBox()const;

		MashSceneNode* _CreateInstance(MashSceneNode *parent, const MashStringc &name);
	};

	inline const mash::MashAABB& CMashDummy::GetLocalBoundingBox()const
	{
		return m_boundingBox;
	}

	inline uint32 CMashDummy::GetNodeType()const
	{
		return aNODETYPE_DUMMY;
	}
}

#endif