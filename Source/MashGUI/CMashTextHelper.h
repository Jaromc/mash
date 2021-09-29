//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_TEXT_HELPER_H_
#define _C_MASH_TEXT_HELPER_H_

#include "MashGUIFont.h"
#include "MashRectangle2.h"
#include "MashTypes.h"
#include "MashString.h"
#include "MashGUIManager.h"

namespace mash
{
	//TODO : Remove this
	template<class T>
	class MashSmrtPtrAlloc
	{
	private:
		class cReferenceCounter
		{
		private:
			int32 m_iCount;
		public:
			cReferenceCounter():m_iCount(0){}

			void Grab()
			{
				m_iCount++;
			}

			int32 Release()
			{
				return --m_iCount;
			}
		};
	private:
		T *m_pObject;
		cReferenceCounter *m_pRefCounter;

	public:
		MashSmrtPtrAlloc(T *pObject):m_pObject(pObject), m_pRefCounter(0)
		{
			if (pObject)
			{
				m_pRefCounter = MASH_NEW_T_COMMON(cReferenceCounter);
				m_pRefCounter->Grab();
			}
		}

		MashSmrtPtrAlloc(const MashSmrtPtrAlloc<T> &copy):m_pObject(copy.m_pObject),
			m_pRefCounter(copy.m_pRefCounter)
		{
			if (m_pRefCounter)
				m_pRefCounter->Grab();
		}

		MashSmrtPtrAlloc<T>& operator=(const MashSmrtPtrAlloc<T> &copy)
		{
			if (this != &copy)
			{
				if (m_pRefCounter && (m_pRefCounter->Release() == 0))
				{
					if (m_pObject)
					{
						MASH_FREE(m_pObject);
						m_pObject = 0;
					}

					MASH_DELETE_T(cReferenceCounter, m_pRefCounter);
					m_pRefCounter = 0;
				}

				m_pObject = copy.m_pObject;
				m_pRefCounter = copy.m_pRefCounter;

				if (m_pRefCounter)
					m_pRefCounter->Grab();
			}

			return *this;
		}

		MashSmrtPtrAlloc<T>& operator=(T *p)
		{
			if (!m_pRefCounter && p)
			{
				m_pRefCounter = MASH_NEW_T_COMMON(cReferenceCounter);
			}
			else if (m_pRefCounter && (m_pRefCounter->Release() == 0))
			{
				if (m_pObject)
				{
					MASH_FREE(m_pObject);
					m_pObject = 0;
				}

				if (!p)
				{
					MASH_DELETE_T(cReferenceCounter, m_pRefCounter);
					m_pRefCounter = 0;
				}
			}

			m_pObject = p;

			if (m_pRefCounter)
				m_pRefCounter->Grab();

			return *this;
		}

		~MashSmrtPtrAlloc()
		{
			if (m_pRefCounter && (m_pRefCounter->Release() == 0))
			{
				if (m_pObject)
				{
					MASH_FREE (m_pObject);
					m_pObject = 0;
				}

				MASH_DELETE_T(cReferenceCounter, m_pRefCounter);
				m_pRefCounter = 0;
			}
		}

		T& operator*()
		{
			return *m_pObject;
		}

		T* operator->()
		{
			return m_pObject;
		}

		T* Get()
		{
			return m_pObject;
		}

		const T* Get()const
		{
			return m_pObject;
		}

	};

	class CMashTextHelper
	{
	private:
		enum eTEXT_UPDATE_FLAGS
		{
			eTEXT_UPDATE_FLAG_FULL = 1,
			eTEXT_UPDATE_POSITION_ONLY = 2
		};

		MashSmrtPtrAlloc<mash::MashVertexPosTex::sMashVertexPosTex> m_textVerticesPtr;
		uint32 m_reservedVertexBufferSize;
		uint32 m_currentTextVerticesCount;
		MashGUIFont *m_font;
		MashStringc m_text;
		uint8 m_updateFlags;
		MashGUIFont::eFONT_ALIGNMENT m_textAlignment;
		bool m_wordWrap;

		/*
			This is the whole text region
		*/
		mash::MashRectangle2 m_absRegion;
		/*
			This is the whole clipping area (say the window)
		*/
		mash::MashRectangle2 m_absClippingRegion;
		/*
			This is the renderable text area after the text region has been clipped by
			the clippnig region
		*/
		mash::MashRectangle2 m_absClippedRegion;
		f32 m_moveAmountX;
		f32 m_moveAmountY;

		int32 m_caratCharacterIndex;
		MashVector2 m_caratTextOffset;
		MashVector2 m_caratPosition;
		bool m_caratEnabled;

		void Update();
		void UpdateCaratOffset(int32 newCaratIndex);
	public:
		CMashTextHelper();
		CMashTextHelper(const CMashTextHelper &copyFrom);
		~CMashTextHelper();

		void SetFormat(MashGUIFont *font, MashGUIFont::eFONT_ALIGNMENT textAlignment, bool wordWrap);
		void SetString(const MashStringc &text);

		/*
			Assumes no culling has taken place. So a position change only.
		*/
		void AddPosition(f32 x, f32 y);

		/*
			Full region update
		*/
		void SetRegion(const mash::MashRectangle2 &absRect, const mash::MashRectangle2 &clippingRect);
		eMASH_STATUS Draw(MashGUIManager *manager, const mash::sMashColour &colour, const sGUIOverrideTransparency &overrideTransparency);

		MashGUIFont::eFONT_ALIGNMENT GetAlignment()const;
		bool GetWordWrap()const;
		const MashStringc& GetString()const;
		MashGUIFont* GetFont()const;

		const mash::MashRectangle2& GetAbsoluteClippedRegion()const;
		//used for carat positioning
		const mash::MashVector2& GetCaratAbsPosition()const;
		
		/*
			Note, carat movement must only be used on single
			lined text boxes (word wrap disabled). Otherwise bad
			things will occur. More work is needed to extend the
			functionality to multiline.
		*/
		void MoveCaratLeft();
		void MoveCaratRight();
		void EnableCarat(bool enable);
		void SendCaratToFront();
		void SendCaratToBack();

		void SetFont(MashGUIFont *font);
		void SetAlignment(MashGUIFont::eFONT_ALIGNMENT val);
		void SetWordWrap(bool val);
		void AddString(const MashStringc &text);
		void AddCharacter(int8 c);
		void InsertString(uint32 location, const int8 *string);
		void InsertCharacter(uint32 location, int8 c);
		void RemoveCharacters(uint32 count);
	};

	inline void CMashTextHelper::EnableCarat(bool enable)
	{
		m_caratEnabled = enable;
	}

	inline const mash::MashRectangle2& CMashTextHelper::GetAbsoluteClippedRegion()const
	{
		return m_absClippedRegion;
	}

	inline MashGUIFont* CMashTextHelper::GetFont()const
	{
		return m_font;
	}

	inline MashGUIFont::eFONT_ALIGNMENT CMashTextHelper::GetAlignment()const
	{
		return m_textAlignment;
	}

	inline bool CMashTextHelper::GetWordWrap()const
	{
		return m_wordWrap;
	}

	inline const MashStringc& CMashTextHelper::GetString()const
	{
		return m_text;
	}
}

#endif