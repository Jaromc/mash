//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashD3D10SkinManager.h"
#include "CMashD3D10Renderer.h"
#include "CMashD3D10Effect.h"
#include "MashDevice.h"
#include "MashHelper.h"
#include "MashLog.h"
namespace mash
{
	CMashD3D10SkinManager::CMashD3D10SkinManager(CMashD3D10Renderer *pRenderer):MashMaterialManagerIntermediate(pRenderer),
		m_pRenderer(pRenderer)
	{
		
	}

	CMashD3D10SkinManager::~CMashD3D10SkinManager()
	{

	}

	MashEffect* CMashD3D10SkinManager::CreateEffect()
	{
		return MASH_NEW_COMMON CMashD3D10Effect(m_pRenderer);
	}

	bool CMashD3D10SkinManager::IsProfileSupported(eSHADER_PROFILE profile)const
	{
		switch(profile)
		{
		case aSHADER_PROFILE_VS_4_0:
			return true;
		case aSHADER_PROFILE_PS_4_0:
			return true;
		case aSHADER_PROFILE_GS_4_0:
			return true;
		};

		return false;
	}

	eSHADER_PROFILE CMashD3D10SkinManager::GetLatestVertexProfile()const
	{
		return aSHADER_PROFILE_VS_4_0;
	}

	eSHADER_PROFILE CMashD3D10SkinManager::GetLatestFragmentProfile()const
	{
		return aSHADER_PROFILE_PS_4_0;
	}

	eSHADER_PROFILE CMashD3D10SkinManager::GetLatestGeometryProfile()const
	{
		return aSHADER_PROFILE_GS_4_0;
	}
}