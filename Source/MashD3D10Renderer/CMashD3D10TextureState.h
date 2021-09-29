//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_D3D10_TEXTURE_STATE_H_
#define _C_MASH_D3D10_TEXTURE_STATE_H_

#ifdef MASH_WINDOWS
#include "MashTexture.h"
#include <d3d10_1.h>

namespace mash
{
	class CMashD3D10TextureState : public MashTextureState
	{
	private:
		ID3D10SamplerState  *m_pD3D10SamplerData;
	public:
		CMashD3D10TextureState(ID3D10SamplerState *pD3D10SamplerData,
			const sSamplerState &_stateData):MashTextureState(_stateData),
			m_pD3D10SamplerData(pD3D10SamplerData){}

		~CMashD3D10TextureState(){m_pD3D10SamplerData->Release();}

		const ID3D10SamplerState* GetD3D10SamplerState()const{return m_pD3D10SamplerData;}
	};
}

#endif

#endif
