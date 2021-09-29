//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_LIST_BOX_H_
#define _C_MASH_GUI_LIST_BOX_H_

#include "MashGUIListBox.h"
//#include "CMashGUIEvent.h"
#include "MashGUIStaticText.h"
//#include "MashGUISprite.h"
#include "MashGUIScrollbarView.h"
//#include "CMashGUIListBoxBatch.h"
#include "CMashTextHelper.h"

namespace mash
{
	class CMashGUIListBox : public MashGUIListBox
	{
	private:
		struct sListBoxItem
		{
			int32 id;
			int32 returnValue;
			bool bCulled;
			mash::MashRectangle2 absoluteRect;
			mash::MashRectangle2 absoluteClippedRect;

			mash::MashRectangle2 vIconTexCoords;
			mash::MashTexture *pIcon;
			CMashTextHelper textHandler;

			sListBoxItem():absoluteRect(), absoluteClippedRect(), bCulled(false),
				vIconTexCoords(0.0f, 0.0f, 1.0f, 1.0f), pIcon(0), id(-1), returnValue(0){}
		};
	private:
		int32 m_itemHeight;
		MashGUIRect m_ItemIconDestination;
		MashGUIRect m_ItemTextDestination;

		int32 m_styleElement;

		uint32 m_iLastDoubleClickTime;
		uint32 m_iDoubleClickTimeLimit;

		bool m_displayIcons;

		int32 m_iSelectedItem;
		bool m_bUpdateNeeded;

		MashGUIScrollbarView *m_pScrollBar;
		MashArray<sListBoxItem> m_listBoxItems;

		void CalculateMaxVisibleItems();

		void OnSBValueChange(const sGUIEvent &eventData);
		void UpdateItem(sListBoxItem *pItem, int32 itemPos, MashGUIFont *font);
		void UpdateAllItems(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
		sListBoxItem* GetItem(int32 id);
		sListBoxItem* GetItemByUserId(int32 val);
		void UpdateDefaultRegions();
	protected:
		int32 m_itemIdCounter;
	public:
		CMashGUIListBox(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement);

		~CMashGUIListBox();

		void OnEvent(const sInputEvent &eventData);

		void SetOverrideTransparency(bool state, uint8 alpha, bool affectFont = true, f32 alphaMaskThreshold = 0.0f);
		const MashGUIRect& GetItemIconRegion()const;
		const MashGUIRect& GetItemTextRegion()const;
		mash::MashRectangle2 GetItemIconOffsetRegion()const;
		mash::MashRectangle2 GetItemTextOffsetRegion()const;
		uint32 GetItemHeight()const;
		const mash::MashRectangle2& GetItemIconSourceRegion(int32 id);

		void SetItemHeight(uint32 height);
		void SetItemIcon(int32 id, mash::MashTexture *icon, const mash::MashRectangle2 *source = 0);
		void SetItemIconRegion(const MashGUIRect &destination);
		void SetItemTextRegion(const MashGUIRect &destination);
		void SetItemText(int32 id, const MashStringc &text);
		void SetItemUserValue(int32 id, int32 val);
		void SetItemIconSourceRegion(int32 id, const mash::MashRectangle2 &source);
		void RemoveItem(int32 id);
		uint32 GetItemCount()const;
		int32 AddItem(const MashStringc &text, int32 userValue, mash::MashTexture *pIcon, const mash::MashRectangle2 *iconDestRegion = 0);

		void EnableIcons(bool state, bool setIconAndTextRegionsToDefault = false);
		bool GetIconsEnabled()const;

		int32 GetItemUserValue(int32 id);
		const MashStringc& GetItemText(int32 id);
		mash::MashTexture* GetItemIcon(int32 id);
		eMASH_GUI_TYPE GetGUIType()const;

		void Draw();

		void OnStyleChange(MashGUIStyle *style);

		void SetActiveItem(int32 id, bool sendConfirmedMessage = false);
		void SetActiveItemByUserValue(int32 val, bool sendConfirmedMessage = false);

		void ClearAllItems();
		int32 GetSelectedItemId()const;
		MashGUIComponent* GetClosestIntersectingChild(const mash::MashVector2 &vScreenPos, bool bTestAllChildren = false);
	};

	inline uint32 CMashGUIListBox::GetItemHeight()const
	{
		return m_itemHeight;
	}

	inline const MashGUIRect& CMashGUIListBox::GetItemIconRegion()const
	{
		return m_ItemIconDestination;
	}

	inline const MashGUIRect& CMashGUIListBox::GetItemTextRegion()const
	{
		return m_ItemTextDestination;
	}

	inline bool CMashGUIListBox::GetIconsEnabled()const
	{
		return m_displayIcons;
	}

	inline int32 CMashGUIListBox::GetSelectedItemId()const
	{
		return m_iSelectedItem;
	}

	inline eMASH_GUI_TYPE CMashGUIListBox::GetGUIType()const
	{
		return aGUI_LISTBOX;
	}
}

#endif