//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_D3D10_SKIN_MANAGER_H_
#define _C_MASH_D3D10_SKIN_MANAGER_H_

#include "MashDataTypes.h"

#ifdef MASH_WINDOWS
#include "MashMaterialManagerIntermediate.h"
#include "MashFileManager.h"
#include <d3d10_1.h>
#include <d3d10.h>

namespace mash
{
	class CMashD3D10Renderer;

	class CMashD3D10SkinManager : public MashMaterialManagerIntermediate
	{
	private:
		CMashD3D10Renderer *m_pRenderer;

	public:
		CMashD3D10SkinManager(CMashD3D10Renderer *pRenderer);
		~CMashD3D10SkinManager();

		MashEffect* CreateEffect();

		bool IsProfileSupported(eSHADER_PROFILE profile)const;

		eSHADER_PROFILE GetLatestVertexProfile()const;
		eSHADER_PROFILE GetLatestFragmentProfile()const;
		eSHADER_PROFILE GetLatestGeometryProfile()const;
	};
}

#endif

#endif
