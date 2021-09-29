//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_VIEW_H_
#define _C_MASH_GUI_VIEW_H_

#include "MashGUIView.h"
#include "MashGUIScrollbarView.h"

namespace mash
{
	class CMashGUIView : public MashGUIView
	{
	private:
		MashGUIScrollbarView *m_verticalScrollBar;
		MashGUIScrollbarView *m_horizontalScrollBar;
		bool m_renderBackground;
		int32 m_styleElement;

		MashGUICustomRender *m_customRenderer;

		bool m_hscrollEnabled;
		bool m_vscrollEnabled;

		MashList<MashGUIComponent*> m_children;
		MashList<MashGUIComponent*> m_alwaysOnTopStack;

		void OnChildRegionChange(MashGUIComponent *component);
		bool m_updateScrollbars;
		void UpdateScrollbars();
		bool _RemoveChild(MashGUIComponent *pChild);
		bool _AddChild(MashGUIComponent *pChild);
		void _SendChildToBack(MashGUIComponent *pChild);
		void _SendChildToFront(MashGUIComponent *pChild);
		void OnVerticalSBValueChange(const sGUIEvent &eventData);
		void OnHorizontalSBValueChange(const sGUIEvent &eventData);
		void OnHorizontalSBRelease(const sGUIEvent &eventData);
		void _GetResetDestinationRegion(MashGUIRect &out)const;
		void OnResize(bool positionChangeOnly, f32 deltaX = 0, f32 deltaY = 0);
		void OnChildAlwaysOnTop(MashGUIComponent *component);
		void OnRemoveChildAlwaysOnTop(MashGUIComponent *component);
		void OnAddChildFromConstructor(MashGUIComponent *component);
	public:
		CMashGUIView(MashGUIManager *pGUIManager,
			MashInputManager *pInputManager,
			MashGUIComponent *pParent,
			const MashGUIRect &destination,
			int32 styleElement);

		~CMashGUIView();

		void OnStyleChange(MashGUIStyle *style);
		void SetOverrideTransparency(bool state, uint8 alpha, bool affectFont = true, f32 alphaMaskThreshold = 0.0f);

		MashGUIComponent* GetElementByName(const MashStringc &name, bool searchChildren = true);
		bool IsElementInHierarchy(MashGUIComponent *element, bool searchChildren = true);

		void SetCustomRenderer(MashGUICustomRender *customRenderer);

		void SetRenderBackgroundState(bool state);
		bool GetRenderbackgroundState()const;

		bool IsHorizontalScrollEnabled()const;
		bool IsVerticalScrollEnabled()const;
        
        bool IsHorizontalScrollInUse()const;
		bool IsVerticalScrollInUse()const;

		void DetachAllChildren();
		bool DetachChild(MashGUIComponent *pChild);
		void AddChild(MashGUIComponent *pChild);
		void SetVerticalScrollState(bool state);
		void SetHorizontalScrollState(bool state);
        
        void OnEvent(const sInputEvent &eventData);

		void SetScrollableAreaLock(bool enable, uint32 width = 100, uint32 height = 100);

		f32 GetVerticalScrollAmount()const;
		f32 GetHorizontalScrollAmount()const;

		void SendChildToFront(MashGUIComponent *pChild);
		void SendChildToBack(MashGUIComponent *pChild);

		void SetEventsEnabled(bool state);

		MashGUIView* GetView();

		
		MashGUIComponent* GetClosestIntersectingChild(const mash::MashVector2 &vScreenPos, bool bTestAllChildren = false);
		MashGUIView* GetClosestParentableObject(const mash::MashVector2 &screenPosition);

		const MashList<MashGUIComponent*>& GetChildren()const;

		eMASH_GUI_TYPE GetGUIType()const;
		void Draw();
	};

	inline bool CMashGUIView::GetRenderbackgroundState()const
	{
		return m_renderBackground;
	}

	inline bool CMashGUIView::IsHorizontalScrollEnabled()const
	{
		return m_hscrollEnabled;
	}

	inline bool CMashGUIView::IsVerticalScrollEnabled()const
	{
		return m_vscrollEnabled;
	}

	inline MashGUIView* CMashGUIView::GetView()
	{
		return this;
	}

	inline const MashList<MashGUIComponent*>& CMashGUIView::GetChildren()const
	{
		return m_children;
	}

	inline eMASH_GUI_TYPE CMashGUIView::GetGUIType()const
	{
		return aGUI_VIEW;
	}
}

#endif