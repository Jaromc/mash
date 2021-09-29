//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_GUI_MANAGER_H_
#define _C_MASH_GUI_MANAGER_H_

#include "MashCreationParameters.h"
#include "MashGUIManager.h"
#include "MashMaterial.h"
#include "MashList.h"
namespace mash
{
	class CMashGUIRenderBatch;
	class CMashGUIFontBatch;
	class CMashGUILineBatch;
	class CMashGUIPrimitiveBatch;
    class MashCamera;
	class MashGUIFactory;

	class CMashGUIManager : public MashGUIManager
	{
	private:
		MashVideo *m_pRenderer;
		MashInputManager *m_inputManager;

		CMashGUIRenderBatch *m_pRenderBatch;
		CMashGUIFontBatch *m_pFontBatch;
		CMashGUILineBatch *m_pLineBatch;
		CMashGUIPrimitiveBatch *m_pPrimitiveBatch;

		MashGUIFactory *m_guiFactory;

		//the following is used to remember camera state
		bool m_beginDrawCalled;
		bool m_oldCameraUpdateState;
		mash::MashCamera *m_sceneCamera;

		//used for popup windows
		MashList<MashGUIComponent*> m_focusLockStack;

		MashGUIComponent *m_pFocus;
		MashGUIComponent *m_pMouseHoverComponent;

		MashGUIStyle *m_activeStyle;
		std::map<MashStringc, MashGUIStyle*> m_styles;
		std::map<MashStringc, MashGUIFont*> m_fonts;
		MashArray<MashGUIComponent*> m_destroyList;

		eMASH_STATUS CreateDefaultGUIStyle();

		MashGUIView *m_pRootWindow;
		mash::sMashColour m_debugDrawColour;

		void OnFocusLost(MashGUIComponent *elm);
		void OnFocus(MashGUIComponent *elm);
		void ProcessDestroyList();
        MashGUIView* GetVerticalScrollableView(MashGUIComponent *component);

	public:
		CMashGUIManager();
		~CMashGUIManager();

		eMASH_STATUS _Initialise(const mash::sMashDeviceSettings &settings, MashVideo *pRenderer, MashInputManager *pInputManager);

		MashGUIViewport* AddViewport(const MashGUIRect &destRegion, MashGUIView *pParent = 0, int32 styleElement = -1);
		MashGUISprite* AddSprite(const MashGUIRect &destRegion, MashGUIView *pParent = 0, int32 styleElement = -1);
		MashGUIButton* AddButton(const MashGUIRect &destRegion, MashGUIView *pParent = 0, int32 styleElement = -1);
		MashGUICheckBox* AddCheckBox(const MashGUIRect &destRegion, MashGUIView *pParent = 0, int32 styleElement = -1);

		MashGUIWindow* AddWindow(const MashGUIRect &destRegion, MashGUIView *pParent = 0, int32 styleElement = -1);
		MashGUIScrollBar* AddScrollBar(const MashGUIRect &destRegion, bool isVertical, f32 incrementAmount = 0.1f, MashGUIView *pParent = 0, int32 styleElement = -1);
		MashGUITabControl* AddTabControl(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement = -1);
		MashGUIListBox* AddListBox(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement = -1);
		MashGUITree* AddTree(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement = -1);
		MashGUIView* AddView(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement = -1);
		MashGUIPopupMenu* CreatePopupMenu(int32 styleElement = -1);
		MashGUIOpenFileDialog* CreateOpenFileDialog(const MashGUIRect &destRegion);
		MashGUIMenuBar* AddMenuBar(MashGUIView *pParent, const MashGUIUnit &left = MashGUIUnit(0.0f, 0.0f), const MashGUIUnit &right = MashGUIUnit(1.0f, 0.0f), int32 styleElement = -1);
		MashGUITextBox* AddTextBox(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement = -1);
		MashGUIStaticText* AddStaticText(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement = -1);

		MashGUIWindow* AddDebugLogWindow(const MashGUIRect &destRegion, uint32 logFlags = MashLog::aERROR_LEVEL_ALL, uint32 maxMessageCount = 10);

		
		MashGUIFont* GetFont(mash::MashTexture *pTexture, const int8 *sDataFileName);

		void SetFocusedElement(MashGUIComponent *pHasFocus);
		MashGUIComponent* GetFocusedElement();

		eMASH_STATUS SetGUIStyle(const MashStringc &name);
		MashGUIStyle* GetGUIStyle(const MashStringc &name);
		MashGUIStyle* GetActiveGUIStyle();
        
		/*
			When an objects reference counter reaches zero then it
			calls this to delete it when the time is right.
			This is also called from an objects Destroy method.
		*/
		void _DestroyElement(MashGUIComponent *element);

		const mash::sMashColour& GetDebugDrawColour()const;
		void SetDebugDrawColour(const mash::sMashColour &c);

		eMASH_STATUS LoadGUILayout(const int8 *layoutFileName, MashGUIView *root = 0, MashGUILoadCallback *callback = 0); 
		eMASH_STATUS SaveGUILayout(const int8 *layoutFileName, MashGUIComponent *root, bool saveRoot);

		eMASH_STATUS SaveGUIStyle(const int8 *styleFileName, const MashArray<MashGUIStyle*> &styles);
		eMASH_STATUS LoadGUIStyle(const int8 *styleFileName, const int8 **customElements = 0);

		void OnShowElement(MashGUIComponent *element);
		void OnHideElement(MashGUIComponent *element);
		void OnResize();

		void OnEvent(const sInputEvent &eventData);

		eMASH_STATUS BeginDraw();
		void DrawAll();
		void DrawComponent(MashGUIComponent *component);
		void EndDraw();
		void OnDestroyElement(MashGUIComponent *component);

		void FlushBuffers();

		void DrawSprite(const mash::MashVertexGUI::sMashVertexGUI *pVertices,
			uint32 iVertexCount,
			const MashGUISkin *skin,
			const sGUIOverrideTransparency &transparencyOverride);
		
		void DrawSprite(const mash::MashRectangle2 &rect,
			const mash::MashRectangle2 &clippingRect,
			const MashGUISkin *skin,
			const sGUIOverrideTransparency &transparencyOverride);

		void DrawText(const mash::MashVertexPosTex::sMashVertexPosTex *pVertices,
			uint32 iVertexCount,
			mash::MashTexture *pTexture,
			const mash::sMashColour &fontColour);

		void DrawBorder(const mash::MashRectangle2 &rect,
			const mash::sMashColour &colour);

		void DrawSolidShape(const mash::MashRectangle2 &rect,
			const mash::sMashColour &colour);

		void DrawSolidTriangles(const mash::MashVertexColour::sMashVertexColour *vertices, uint32 vertexCount);

		void DrawLine(const mash::MashVector2 &top, const mash::MashVector2 &bottom,
			const mash::sMashColour &colour);

		enum eDRAW_TYPE
		{
			aGUIDRAW_SPRITE,
			aGUIDRAW_FONT
		};

		void GetClickedElement(const mash::MashVector2 &vMousePos,
			MashGUIComponent *pCurrentElement,
			MashGUIComponent **pClosest);

		void GetMouseHoverElement(const mash::MashVector2 &vMousePos,
			MashGUIComponent *pCurrentElement,
			MashGUIComponent **pClosest);

		MashGUIFactory* _GetGUIFactory()const;

		MashGUIView* GetRootWindow()const;
	};

	inline const mash::sMashColour& CMashGUIManager::GetDebugDrawColour()const
	{
		return m_debugDrawColour;
	}

	inline void CMashGUIManager::SetDebugDrawColour(const mash::sMashColour &c)
	{
		m_debugDrawColour = c;
	}

	inline MashGUIView* CMashGUIManager::GetRootWindow()const
	{
		return m_pRootWindow;
	}

	inline MashGUIComponent* CMashGUIManager::GetFocusedElement()
	{
		return m_pFocus;
	}

	class CMashGUIFactory : public MashGUIFactory
	{
	private:
		MashGUIManager *m_guiManager;
		 MashInputManager *m_inputManager;
		 mash::MashVideo *m_renderer;
	public:
		CMashGUIFactory(MashGUIManager *manager, MashInputManager *inputManager, mash::MashVideo *renderer):m_guiManager(manager),
			m_inputManager(inputManager), m_renderer(renderer){}
		~CMashGUIFactory(){}

		MashGUIViewport* CreateViewport(const MashGUIRect &destRegion, MashGUIComponent *pParent = 0, int32 styleElement = -1);
		MashGUISprite* CreateSprite(const MashGUIRect &destRegion, MashGUIComponent *pParent = 0, int32 styleElement = -1);
		MashGUIButton* CreateButton(const MashGUIRect &destRegion, MashGUIComponent *pParent = 0, int32 styleElement = -1);
		MashGUIWindow* CreateWindowView(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement = -1);
		MashGUICheckBox* CreateCheckBox(const MashGUIRect &destRegion, MashGUIComponent *pParent = 0, int32 styleElement = -1);
		MashGUIScrollBar* CreateScrollBar(const MashGUIRect &destRegion, bool isVertical, f32 incrementAmount = 0.1f, MashGUIView *pParent = 0, int32 styleElement = -1);
		MashGUIScrollbarView* CreateScrollBarView(bool isVertical, MashGUIComponent *pParent = 0, int32 styleElement = -1);
		MashGUITabControl* CreateTabControl(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement = -1);
		MashGUIStaticText* CreateStaticText(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement = -1);
		MashGUITree* CreateTree(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement = -1);
		MashGUIListBox* CreateListBox(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement = -1);
		MashGUIPopupMenu* CreatePopupMenu(int32 styleElement = -1);
		MashGUIMenuBar* CreateMenuBar(MashGUIComponent *pParent, const MashGUIUnit &left = MashGUIUnit(0.0f, 0.0f), const MashGUIUnit &right = MashGUIUnit(1.0f, 0.0f), int32 styleElement = -1);
		MashGUITextBox* CreateTextBox(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement = -1);
		MashGUIView* CreateView(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement = -1);
		MashGUIOpenFileDialog* CreateOpenFileDialog(const MashGUIRect &destRegion);
		MashGUIStaticText* CreateDebugLog(const MashGUIRect &destRegion, MashGUIComponent *pParent, uint32 logFlags = MashLog::aERROR_LEVEL_ALL, uint32 maxMessageCount = 10);
	};
}

#endif