//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_TREE_H_
#define _C_MASH_GUI_TREE_H_

#include "MashGUITree.h"
#include "MashGUIFont.h"
#include "CMashTextHelper.h"
#include "MashGUIScrollbarView.h"

namespace mash
{
	class CMashGUITree : public MashGUITree
	{
	private:

		struct sItem
		{
			int32 id;
			int32 userId;
			CMashTextHelper textHandler;
			bool expand;
			mash::MashRectangle2 expandButtonAbsRegion;
			MashArray<sItem> children;

			sItem():id(-1), userId(0), expand(false), expandButtonAbsRegion(0.0f, 0.0f, 0.0f, 0.0f){}
		};

		int32 m_styleElement;

		uint32 m_iLastDoubleClickTime;
		uint32 m_iDoubleClickTimeLimit;

		bool m_bUpdateNeeded;

		MashGUIScrollbarView *m_pVerticalScrollBar;
		MashGUIScrollbarView *m_pHorizontalScrollBar;
		sItem m_rootItem;
		int32 m_selectedItemId;
		uint32 m_itemCounter;

		sItem* GetItemByUserId(sItem *root, int32 id)const;
		sItem* GetItemById(sItem *root, int32 id)const;
		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
		void UpdateItemPositionsOnly(sItem *root, f32 deltaX, f32 deltaY);
		void OnResizeItemOnly(sItem *root, MashGUIFont *font);
		void OnResizeItems(sItem *root, MashGUIFont *font, mash::MashVector2 &topLeftPosition);
		void DrawItems(sItem *root, MashGUISkin *activeItemSkin, MashGUISkin *inactiveItemSkin, MashGUISkin *expandButtonSkin, MashGUISkin *retractButtonSkin);
		sItem* CheckElementSelection(sItem *root, uint32 currentTime, const mash::MashVector2 &cursorPos);
		void CalculateVerticalScrollDistance(sItem *root, MashGUIFont *font, f32 &distance);
		void CalculateHorizontalScrollDistance(sItem *root, MashGUIFont *font, f32 &distance, f32 &indent);
		bool RemoveItem(sItem *root, int32 id);
		void RemoveAllItems(sItem *root);
		void UpdateTree(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
		void GetItems(sItem *root, MashArray<sItemData> &out);
	public:
		CMashGUITree(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement);
		
		~CMashGUITree();

		void SetOverrideTransparency(bool state, uint8 alpha, bool affectFont = true, f32 alphaMaskThreshold = 0.0f);

		int32 GetSelectedItemID()const;
		void SetItemText(int32 id, const MashStringc &text);

		void SetActiveItem(int32 id);
		void SetActiveItemByUserId(int32 id);
		void OnStyleChange(MashGUIStyle *style);

		void GetItems(MashArray<sItemData> &out);
		void RemoveAllItems();
		void RemoveItem(int32 id);
		int32 AddItem(const MashStringc &text, int32 parent = -1, int32 userId = 0);
		void OnEvent(const sInputEvent &eventData);
		eMASH_GUI_TYPE GetGUIType()const;
		MashGUIComponent* GetClosestIntersectingChild(const mash::MashVector2 &vScreenPos, bool bTestAllChildren);
		void Draw();

		void SetItemUserId(int32 id, int32 userId);
		const MashStringc& GetItemText(int32 id);
		int32 GetItemUserId(int32 id);
		int32 GetItemIdByUserValue(int32 userValue);

		void OnVerticalSBValueChange(const sGUIEvent &eventData);
		void OnHorizontalSBValueChange(const sGUIEvent &eventData);

	};

	inline int32 CMashGUITree::GetSelectedItemID()const
	{
		return m_selectedItemId;
	}

	inline eMASH_GUI_TYPE CMashGUITree::GetGUIType()const
	{
		return aGUI_TREE;
	}
}

#endif