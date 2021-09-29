//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashDummy.h"
#include "MashSceneManager.h"
namespace mash
{
	CMashDummy::CMashDummy(MashSceneNode *pParent,
			mash::MashSceneManager *pManager,
			const MashStringc &sName):MashDummy(pParent, pManager, sName),
			m_boundingBox(mash::MashVector3(-0.5f, -0.5f, -0.5f), mash::MashVector3(0.5f, 0.5f, 0.5f))
	{

	}

	CMashDummy::~CMashDummy()
	{
		
	}

	void CMashDummy::SetBoundingBox(const mash::MashAABB &bounds)
	{
		m_boundingBox = bounds;
	}

	MashSceneNode* CMashDummy::_CreateInstance(MashSceneNode *parent, const MashStringc &name)
	{
		CMashDummy *pNewDummy = (CMashDummy*)m_sceneManager->AddDummy(parent, name);

		if (!pNewDummy)
			return 0;

		pNewDummy->InstanceMembers(this);

		pNewDummy->m_boundingBox = m_boundingBox;

		return pNewDummy;
	}
}