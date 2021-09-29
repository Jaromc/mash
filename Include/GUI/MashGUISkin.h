//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_SKIN_H_
#define _MASH_GUI_SKIN_H_

#include "MashReferenceCounter.h"
#include "MashTexture.h"
#include "MashRectangle2.h"

namespace mash
{
	class MashGUISkin
	{
	private:
		//! Texture for this skin.
		MashTexture *m_baseTexture;
	public:
		//! Texture region to render in pixels. Eg (0, 0, textureWidth, textureHeight).
		MashRectangle2 baseSource;

		//! This is multiplied with the base texture.
		sMashColour baseColour;

		//! Set this to true to enable transparency.
		bool isTransparent;

		//! Pixels with a value less than this are not rendered, and therefore tranparent.
		f32 alphaMaskThreshold;

		//! A fonts colour will be multiplied by this value.
		sMashColour fontColour;

		//! Border colour
		sMashColour borderColour;

		//! Enables border rendering.
		bool renderBoarder;

	public:
		MashGUISkin():m_baseTexture(0),baseSource(0.0f, 0.0f, 1.0f, 1.0f),
			baseColour(255, 255, 255, 255), isTransparent(false),
			fontColour(255, 255, 255, 255), borderColour(255, 255, 255, 255),
			alphaMaskThreshold(0.5f),renderBoarder(false){}

		MashGUISkin(const MashGUISkin &copyFrom):baseSource(copyFrom.baseSource),
			baseColour(copyFrom.baseColour), isTransparent(copyFrom.isTransparent),
			fontColour(copyFrom.fontColour), borderColour(copyFrom.borderColour),
			alphaMaskThreshold(copyFrom.alphaMaskThreshold),renderBoarder(copyFrom.renderBoarder)
			{
				SetTexture(copyFrom.GetTexture());
			}

		~MashGUISkin()
		{
			if (m_baseTexture)
			{
				m_baseTexture->Drop();
				m_baseTexture = 0;
			}
		}

		MashGUISkin& operator=(MashGUISkin const& other)
		{
			baseSource = other.baseSource;
			baseColour = other.baseColour;
			isTransparent = other.isTransparent;
			alphaMaskThreshold = other.alphaMaskThreshold;
			fontColour = other.fontColour;
			borderColour = other.borderColour;
			renderBoarder = other.renderBoarder;

			SetTexture(other.GetTexture());

			return *this;
		}
		
		//! Returns the texture.
		/*!
			\return Texture.
		*/
		inline mash::MashTexture* GetTexture()const
		{
			return m_baseTexture;
		}

		//! Sets the texture.
		/*!
			\param texture texture.
		*/
		void SetTexture(MashTexture *texture)
		{
			if (texture)
				texture->Grab();

			if (m_baseTexture)
				m_baseTexture->Drop();

			m_baseTexture = texture;
		}

		//! Helper function.
		/*!
			Helper function for rendering an entire texture. 
			Call this to set the source region to match the current textures.
		*/
		void ScaleSourceToTexture()
		{
			uint32 iWidth, iHeight;

			if (m_baseTexture)
			{
				m_baseTexture->GetSize(iWidth, iHeight);
				baseSource.left = 0.0f;
				baseSource.top = 0.0f;
				baseSource.right = (f32)iWidth;
				baseSource.bottom = (f32)iHeight;
			}
		}

		//! Helper function to set transparency.
		/*!
			\param state Enable or disable transparency.
			\param alpha Alpha value between 0 - 255.
			\param affectFont Set to true to also affect the font.
			\param _alphaMaskThreshold Sets the alpha mask threshold between 0 - 255.
		*/
		void SetTransparency(bool state, uint8 alpha, bool affectFont = true, uint8 _alphaMaskThreshold = 0)
		{
			isTransparent = state;
			baseColour.SetAlpha(alpha);
			alphaMaskThreshold = _alphaMaskThreshold / 255.0f;

			if (affectFont)
				fontColour.SetAlpha(alpha);
		}
	};
}

#endif