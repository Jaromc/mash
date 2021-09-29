//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#include "MashInclude.h"

#include "MemoryAllocator/MashDefaultMemoryAllocator.h"
#include "D3D10/MashD3D10Creation.h"
#include "OpenGL3/MashOpenGL3Creation.h"

#include <map>
#include <vector>

#if defined (MASH_WINDOWS) && !defined(__MINGW32__)
    #define USE_DIRECTX
#endif

using namespace mash;

static const float g_resizeBoxBuffer = 8.0f;
static const float g_canvasScreenBuffer = 50.0f;
static float g_gridSize = 10.0f;
static bool g_enableSnapToGrid = true;
static bool g_drawGrid = true;
static const sMashColour g_resizeBoxColour(255, 255, 255, 255);

class GridViewRenderer : public MashGUICustomRender
{
private:
	MashGUIManager *m_guiManager;
public:
	GridViewRenderer(MashGUIManager *guiManager):MashGUICustomRender(), m_guiManager(guiManager){}
	GridViewRenderer(){}

	void Draw(MashGUIComponent *component)
	{
		if (g_drawGrid)
		{
			MashRectangle2 absRegion = component->GetAbsoluteClippedRegion();
			sMashColour lineColour(50,50,50,50);
			const unsigned int increment = g_gridSize;
			const unsigned int width = absRegion.right - absRegion.left;
			const unsigned int height = absRegion.bottom - absRegion.top;
			
			m_guiManager->FlushBuffers();
			for(unsigned int y = increment; y < height; y+=increment)
			{
				m_guiManager->DrawLine(MashVector2(absRegion.left, absRegion.top + y), MashVector2(absRegion.right-1, absRegion.top + y), lineColour);
			}

			for(unsigned int x = increment; x < width; x+=increment)
			{
				m_guiManager->DrawLine(MashVector2(absRegion.left + x, absRegion.top), MashVector2(absRegion.left + x, absRegion.bottom-1), lineColour);
			}

			m_guiManager->FlushBuffers();
		}
	}
};

class MainLoop : public MashGameLoop
{
private:
	enum eRIGHT_MOUSE_ACTION
	{
		aRMOUSE_DRAG,
		aRMOUSE_RESIZE_TL,
		aRMOUSE_RESIZE_TR,
		aRMOUSE_RESIZE_BL,
		aRMOUSE_RESIZE_BR,
		aRMOUSE_NONE
	};

	enum eSPECIAL_GUI_ELEMENTS
	{
		aSGUI_VERTICAL_SCROLL_BAR = aGUI_TYPE_COUNT,
		aSGUI_HORIZONTAL_SCROLL_BAR
	};

	struct sMenuBarItem
	{
		int returnValue;
		MashStringc text;
	};

	struct sCurrentMenuBarMenu
	{
		MashGUIPopupMenu *menu;
		int id;

		sCurrentMenuBarMenu():menu(0), id(0){}
	};

	class GUICreationCallback : public MashGUILoadCallback
	{
	public:
		GUICreationCallback(){}
		~GUICreationCallback(){}

		void OnCreateComponent(MashGUIComponent *component)
		{
			component->SetEventsEnabled(false);

			switch(component->GetGUIType())
			{
			case aGUI_VIEWPORT:
				component->SetDrawDebugBounds(true);
				break;
			case aGUI_STATIC_TEXT:
				component->SetDrawDebugBounds(true);
				break;
			}
		}
	};

	friend GUICreationCallback;
private:
	MashDevice *m_device;

	MashCamera *m_camera;
	MashGUIPopupMenu *m_rightClickMenu;
	MashGUIComponent *m_selectedElement;
	MashGUIView *m_focusedElementProperties;
	char m_actionStates[aKEYEVENT_SIZE];
	bool m_quit;
	eRIGHT_MOUSE_ACTION m_actionState;
	MashVector2 m_lastMousePosition;
	MashVector2 m_mouseGridPosition;
	MashGUIRect m_originalDragElementRegion;
	MashGUIView *m_mainEditorView;
	MashGUIView *m_mainPropertiesView;
	MashGUIView *m_canvasSizeView;

	//for listbox editing
	MashGUIListBox *m_currentItemsListbox;

	MashGUIOpenFileDialog *m_openFileDialog;
	MashGUIComponent *m_openFileDialodCaller;

	//for menu bar editing
	MashGUIPopupMenu *m_currentMenuBarMenu;
	MashVector2 m_listBoxIconPosition;
	MashVector2 m_listBoxIconDim;
	MashVector2 m_listBoxTextPosition;
	MashVector2 m_listBoxTextDim;
	MashRectangle2 m_listBoxIconSourceRegion;

	unsigned int m_canvasSizeX;
	unsigned int m_canvasSizeY;

	std::map<int, MashStringc> m_addGUIMap;
public:
	MainLoop(MashDevice *device):MashGameLoop(), m_device(device), m_selectedElement(0), m_actionState(aRMOUSE_NONE), 
		m_lastMousePosition(0.0f, 0.0f), m_mainEditorView(0), m_mouseGridPosition(0.0f, 0.0f),
		m_focusedElementProperties(0), m_currentMenuBarMenu(), m_camera(0), m_quit(false)
	{
		m_addGUIMap[aGUI_BUTTON] = "Button";
		m_addGUIMap[aGUI_CHECK_BOX] = "CheckBox";
		m_addGUIMap[aGUI_STATIC_TEXT] = "StaticText";
		m_addGUIMap[aGUI_TEXT_BOX] = "TextBox";
		m_addGUIMap[aGUI_VIEW] = "View";
		m_addGUIMap[aGUI_LISTBOX] = "ListBox";
		m_addGUIMap[aGUI_SPRITE] = "Sprite";
		m_addGUIMap[aSGUI_VERTICAL_SCROLL_BAR] = "VerticalScrollBar";
		m_addGUIMap[aSGUI_HORIZONTAL_SCROLL_BAR] = "HorizontalScrollBar";
		m_addGUIMap[aGUI_WINDOW] = "Window";
		m_addGUIMap[aGUI_TAB_CONTROL] = "TabControl";
		m_addGUIMap[aGUI_TREE] = "Tree";
		m_addGUIMap[aGUI_MENUBAR] = "MenuBar";
		m_addGUIMap[aGUI_VIEWPORT] = "RenderPort";

		memset(m_actionStates, 0, sizeof(m_actionStates));
	}

	~MainLoop()
	{
		m_rightClickMenu->Destroy();
		m_openFileDialog->Drop();
	}

	bool Initialise()
	{
		m_lastMousePosition = m_device->GetInputManager()->GetCursorPosition();
		m_device->GetInputManager()->RegisterReceiver(MashInputEventFunctor(&MainLoop::OnEvent,this));

		m_camera = (MashCamera*)m_device->GetSceneManager()->AddCamera(0, "Camera01");

		MashGUIManager *guiManager = m_device->GetGUIManager();

		m_canvasSizeX = 800;
		m_canvasSizeY = 600;

		MashGUIRect mainEditorViewRect(MashGUIUnit(0.0f,0.0f), MashGUIUnit(0.0f,0.0f),
			MashGUIUnit(1.0f, 0.0f), MashGUIUnit(1.0f, 0.0f));
		MashGUIView *editorView = guiManager->AddView(mainEditorViewRect, 0);
		editorView->SetRenderBackgroundState(false);

		m_device->GetGUIManager()->LoadGUILayout("./GUIEditorLayout.xml", editorView);

		MashGUIView *focusedElementProperties = (MashGUIView*)editorView->GetElementByName("ButtonProperties");
		focusedElementProperties->GetElementByName("ButtonText")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnButtonTextChange, this));

		focusedElementProperties = (MashGUIView*)editorView->GetElementByName("CheckBoxProperties");
		focusedElementProperties->GetElementByName("CheckBoxIsCheckedCheckBox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnCheckBoxIsChecked, this));
		focusedElementProperties->GetElementByName("CheckBoxIsCheckedCheckBox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnCheckBoxIsChecked, this));

		focusedElementProperties = (MashGUIView*)editorView->GetElementByName("WindowProperties");
		focusedElementProperties->GetElementByName("WindowTitlebarTitleTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnWindowTitleBarTextChange, this));
		focusedElementProperties->GetElementByName("WindowEnableMinimizeCheckBox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnWindowEnableMinimizeCheckBox, this));
		focusedElementProperties->GetElementByName("WindowEnableMinimizeCheckBox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnWindowEnableMinimizeCheckBox, this));
		focusedElementProperties->GetElementByName("WindowEnableCloseCheckBox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnWindowEnableCloseCheckBox, this));
		focusedElementProperties->GetElementByName("WindowEnableCloseCheckBox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnWindowEnableCloseCheckBox, this));
		focusedElementProperties->GetElementByName("WindowEnableDragCheckBox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnWindowEnableDragCheckBox, this));
		focusedElementProperties->GetElementByName("WindowEnableDragCheckBox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnWindowEnableDragCheckBox, this));
		focusedElementProperties->GetElementByName("WindowEnableFocusLockCheckBox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnWindowEnableFocusLockCheckBox, this));
		focusedElementProperties->GetElementByName("WindowEnableFocusLockCheckBox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnWindowEnableFocusLockCheckBox, this));

		focusedElementProperties = (MashGUIView*)editorView->GetElementByName("StaticTextProperties");
		focusedElementProperties->GetElementByName("StaticTextTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnStaticTextTextChange, this));
		focusedElementProperties->GetElementByName("StaticTextEnableWordWrap")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnStaticTextWordWrap, this));
		focusedElementProperties->GetElementByName("StaticTextEnableWordWrap")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnStaticTextWordWrap, this));

		focusedElementProperties = (MashGUIView*)editorView->GetElementByName("TextBoxProperties");
		focusedElementProperties->GetElementByName("TextBoxTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnTextBoxTextChange, this));
		focusedElementProperties->GetElementByName("TextBoxFloatPrecision")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnTextBoxPrecision, this));
		focusedElementProperties->GetElementByName("TextBoxEnableIncrementButtons")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnTextBoxIncrementButtons, this));
		focusedElementProperties->GetElementByName("TextBoxEnableIncrementButtons")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnTextBoxIncrementButtons, this));
		focusedElementProperties->GetElementByName("TextBoxTextType")->RegisterReceiver(aGUIEVENT_LB_SELECTION_CHANGE, MashGUIEventFunctor(&MainLoop::OnTextBoxTextType, this));
		focusedElementProperties->GetElementByName("TextBoxMinNum")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnTextBoxMinNumber, this));
		focusedElementProperties->GetElementByName("TextBoxMaxNum")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnTextBoxMaxNumber, this));

		focusedElementProperties = (MashGUIView*)editorView->GetElementByName("TreeProperties");
		focusedElementProperties->GetElementByName("TreeItemTextBox")->RegisterReceiver(aGUIEVENT_LOST_INPUTFOCUS, MashGUIEventFunctor(&MainLoop::OnTreeItemNameTextChange, this));
		focusedElementProperties->GetElementByName("TreeItemTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnTreeItemNameTextChange, this));
		focusedElementProperties->GetElementByName("TreeInsertItemButton")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnTreeInsertItem, this));
		focusedElementProperties->GetElementByName("TreeRemoveItemButton")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnTreeRemoveItem, this));

		focusedElementProperties = (MashGUIView*)editorView->GetElementByName("TabProperties");
		focusedElementProperties->GetElementByName("TabTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnTabNameTextChange, this));
		focusedElementProperties->GetElementByName("TabInsertTabButton")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnTabInsertItem, this));
		focusedElementProperties->GetElementByName("TabRemoveItemButton")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnTabRemoveItem, this));

		focusedElementProperties = (MashGUIView*)editorView->GetElementByName("ListBoxProperties");
		focusedElementProperties->GetElementByName("ListBoxItemValueTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxItemValueChange, this));
		focusedElementProperties->GetElementByName("ListBoxItemTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxItemNameChange, this));
		focusedElementProperties->GetElementByName("ListBoxInsertItem")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxInsertItem, this));
		focusedElementProperties->GetElementByName("ListBoxRemoveItemButton")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxRemoveItem, this));
		focusedElementProperties->GetElementByName("ListBoxItemIconFileButton")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxLoadIconFileDialog, this));
		
		focusedElementProperties->GetElementByName("ListBoxIconRegionLeftTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxIconRegionLeftChange, this));
		focusedElementProperties->GetElementByName("ListBoxIconRegionTopTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxIconRegionTopChange, this));
		focusedElementProperties->GetElementByName("ListBoxIconRegionRightTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxIconRegionRightChange, this));
		focusedElementProperties->GetElementByName("ListBoxIconRegionBottomTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxIconRegionBottomChange, this));
		focusedElementProperties->GetElementByName("ListBoxTextRegionLeftTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxTextRegionLeftChange, this));
		focusedElementProperties->GetElementByName("ListBoxTextRegionTopTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxTextRegionTopChange, this));
		focusedElementProperties->GetElementByName("ListBoxTextRegionRightTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxTextRegionRightChange, this));
		focusedElementProperties->GetElementByName("ListBoxTextRegionBottomTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxTextRegionBottomChange, this));
		focusedElementProperties->GetElementByName("ListBoxItemIconEnabledCheckbox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnListBoxIconEnableChange, this));
		focusedElementProperties->GetElementByName("ListBoxItemIconEnabledCheckbox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnListBoxIconEnableChange, this));
		focusedElementProperties->GetElementByName("ListBoxItemHeightTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnListBoxIconHeightChange, this));
		m_currentItemsListbox = (MashGUIListBox*)focusedElementProperties->GetElementByName("ListBoxItemsListBox");
		m_currentItemsListbox->RegisterReceiver(aGUIEVENT_LB_SELECTION_CHANGE, MashGUIEventFunctor(&MainLoop::OnListBoxItemChange, this));

		focusedElementProperties = (MashGUIView*)editorView->GetElementByName("MenuBarProperties");
		focusedElementProperties->GetElementByName("MenuBarListBox")->RegisterReceiver(aGUIEVENT_LB_SELECTION_CONFIRMED, MashGUIEventFunctor(&MainLoop::OnMenuBarListBoxConfirm, this));
		focusedElementProperties->GetElementByName("MenuBarListBox")->RegisterReceiver(aGUIEVENT_LB_SELECTION_CHANGE, MashGUIEventFunctor(&MainLoop::OnMenuBarListBoxChange, this));
		focusedElementProperties->GetElementByName("MenuBarExitSubMenuButton")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnExitSubMenuButton, this));
		focusedElementProperties->GetElementByName("MenuBarAddItemButton")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnMenuBarAddItem, this));
		focusedElementProperties->GetElementByName("MenuBarRemoveItemButton")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnMenuBarRemoveItem, this));
		focusedElementProperties->GetElementByName("MenuBarItemNameTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnMenuBarItemNameChange, this));
		focusedElementProperties->GetElementByName("MenuBarItemValueTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnMenuBarItemValueChange, this));

		focusedElementProperties = (MashGUIView*)editorView->GetElementByName("CommonProperties");
		focusedElementProperties->GetElementByName("NameTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnElementNameChange, this));
		focusedElementProperties->GetElementByName("LocalPosXTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnElementLocationXChange, this));
		focusedElementProperties->GetElementByName("LocalPosYTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnElementLocationYChange, this));
		focusedElementProperties->GetElementByName("ElementWidthTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnElementWidthChange, this));
		focusedElementProperties->GetElementByName("ElementHeightTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnElementHeightChange, this));

		focusedElementProperties = (MashGUIView*)editorView->GetElementByName("GlobalProperties");
		((MashGUITextBox*)focusedElementProperties->GetElementByName("NativeResolutionWidthTextBox"))->SetTextInt(m_canvasSizeX);
		((MashGUITextBox*)focusedElementProperties->GetElementByName("NativeResolutionHeightTextBox"))->SetTextInt(m_canvasSizeY);
		((MashGUITextBox*)focusedElementProperties->GetElementByName("GridSizeTextBox"))->SetTextInt(g_gridSize);
		focusedElementProperties->GetElementByName("NativeResolutionWidthTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNativeResolutionWidthChange, this));
		focusedElementProperties->GetElementByName("NativeResolutionHeightTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNativeResolutionHeightChange, this));
		focusedElementProperties->GetElementByName("GridSizeTextBox")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnGridSizeChange, this));
		((MashGUICheckBox*)focusedElementProperties->GetElementByName("EnableGridCheckBox"))->SetChecked(g_enableSnapToGrid);
		focusedElementProperties->GetElementByName("EnableGridCheckBox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnGridToggleChange, this));
		focusedElementProperties->GetElementByName("EnableGridCheckBox")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnGridToggleChange, this));

		focusedElementProperties = (MashGUIView*)editorView->GetElementByName("SpriteProperties");
		focusedElementProperties->GetElementByName("SpriteTextureFileButton")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnSpriteTextureFileDialog, this));

		focusedElementProperties = (MashGUIView*)editorView->GetElementByName("ScrollbarProperties");
		focusedElementProperties->GetElementByName("ScrollbarMinNumTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnScrollbarMinValChange, this));
		focusedElementProperties->GetElementByName("ScrollbarMaxNumTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnScrollbarMaxValChange, this));
		

		MashGUIMenuBar *menuBar = (MashGUIMenuBar*)editorView->GetElementByName("MenuBar");
		menuBar->RegisterReceiver(aGUIEVENT_MENUBAR_SELECTION, MashGUIEventFunctor(&MainLoop::OnMenuBarSelection, this));
		
		m_mainPropertiesView = (MashGUIView*)editorView->GetElementByName("Properties");
		
		/*
			This is the whole area minus the properties windows
		*/
		m_mainEditorView = (MashGUIView*)editorView->GetElementByName("MainSceneView");

		/*
			This is the area that the grid view will be parented to
		*/
		const float menuBarOffset = 20.0f;
		MashGUIRect canvasSizeViewRect(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, menuBarOffset),
			MashGUIUnit(0.0f, m_canvasSizeX + (g_canvasScreenBuffer * 2)), MashGUIUnit(0.0f, m_canvasSizeY + (g_canvasScreenBuffer * 2) + menuBarOffset));
		m_canvasSizeView = m_device->GetGUIManager()->AddView(canvasSizeViewRect, m_mainEditorView);
		m_canvasSizeView->SetHorizontalScrollState(false);
		m_canvasSizeView->SetVerticalScrollState(false);
		
		/*
			This is the grid view. The main editor view.
		*/
		MashGUIRect canvasViewRect(MashGUIUnit(0.0f,g_canvasScreenBuffer), MashGUIUnit(0.0f,g_canvasScreenBuffer),
			MashGUIUnit(1.0f, -g_canvasScreenBuffer), MashGUIUnit(1.0f, -g_canvasScreenBuffer));
		m_mainEditorView = m_device->GetGUIManager()->AddView(canvasViewRect, m_canvasSizeView);

		GridViewRenderer *outsideRenderer = MASH_NEW_COMMON GridViewRenderer(m_device->GetGUIManager());
		m_mainEditorView->SetCustomRenderer(outsideRenderer);
		outsideRenderer->Drop();

		MashGUIRect dialogDest(MashGUIUnit(), MashGUIUnit(), MashGUIUnit(0.0f, 500), MashGUIUnit(0.0f, 400));
		m_openFileDialog = m_device->GetGUIManager()->CreateOpenFileDialog(dialogDest);
		m_openFileDialog->CloseDialog();
		m_openFileDialog->RegisterReceiver(aGUIEVENT_OPENFILE_SELECTED, MashGUIEventFunctor(&MainLoop::OnOpenFileDialogSelection, this));

		MashGUIPopupMenu *addGUIElementMenu = guiManager->CreatePopupMenu();
		std::map<int, MashStringc>::iterator guiMapIter = m_addGUIMap.begin();
		std::map<int, MashStringc>::iterator guiMapIterEnd = m_addGUIMap.end();
		for(; guiMapIter != guiMapIterEnd; ++guiMapIter)
			addGUIElementMenu->AddItem(guiMapIter->second.GetCString(), guiMapIter->first);

		addGUIElementMenu->RegisterReceiver(aGUIEVENT_POPUP_SELECTION, MashGUIEventFunctor(&MainLoop::OnAddItem, this));

		m_rightClickMenu = guiManager->CreatePopupMenu();
		m_rightClickMenu->AddItem("Add Element", 0, addGUIElementMenu);

		return false;
	}

	void OnRightClick(const sInputEvent &e)
	{
		if (m_mainEditorView->GetAbsoluteClippedRegion().IntersectsGUI(m_lastMousePosition))
		{
			UpdateSelectedItem();
			m_rightClickMenu->Activate(m_device->GetInputManager()->GetCursorPosition());
		}
	}

	void UpdateSelectedItem()
	{
		MashGUIComponent *selected = m_selectedElement;
		selected = (MashGUIComponent*)GetDragableComponent(m_lastMousePosition, m_mainEditorView);

		if (selected && (m_selectedElement != selected))
		{
			m_selectedElement = selected;
			OnItemFocus();
		}
	}

	void OnAddItem(const sGUIEvent &e)
	{
		MashGUIPopupMenu *popup = (MashGUIPopupMenu*)e.component;
		int selectedElement = popup->GetItemUserValue(popup->GetSelectedItemId());

		//select a new parent
		MashGUIView *newParent = m_mainEditorView;
		if (m_selectedElement && m_selectedElement->GetView())
			newParent = m_selectedElement->GetView();

		MashVector2 topLeftStartPosition(m_rightClickMenu->GetAbsoluteRegion().left, m_rightClickMenu->GetAbsoluteRegion().top);

		//move the new element into parent space
		topLeftStartPosition.x -= newParent->GetAbsoluteRegion().left;
		topLeftStartPosition.y -= newParent->GetAbsoluteRegion().top;
		
		switch(selectedElement)
		{
		case aGUI_VIEWPORT:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 100), MashGUIUnit(0.0f, topLeftStartPosition.y + 100));

				MashGUIComponent *newComponent = m_device->GetGUIManager()->AddViewport(rect, newParent);

				newComponent->SetDrawDebugBounds(true);
				break;
			};
		case aGUI_SPRITE:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 100), MashGUIUnit(0.0f, topLeftStartPosition.y + 100));

				MashGUIComponent *newElement = m_device->GetGUIManager()->AddSprite(rect, newParent);
				newElement->SetDrawDebugBounds(true);
				break;
			};
		case aGUI_BUTTON:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 50), MashGUIUnit(0.0f, topLeftStartPosition.y + 20));

				MashGUIComponent *newElement = m_device->GetGUIManager()->AddButton(rect, newParent);
				newElement->SetEventsEnabled(false);
				break;
			};
		case aGUI_CHECK_BOX:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 20), MashGUIUnit(0.0f, topLeftStartPosition.y + 20));

				MashGUIComponent *newElement = m_device->GetGUIManager()->AddCheckBox(rect, newParent);
				//disable events so it doesnt check on/off when being moved or resized.
				newElement->SetEventsEnabled(false);
				break;
			};
		case aGUI_WINDOW:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 100), MashGUIUnit(0.0f, topLeftStartPosition.y + 100));

				MashGUIComponent *newElement = m_device->GetGUIManager()->AddWindow(rect, newParent);
				newElement->SetEventsEnabled(false);//stops tool bar buttons causing havok
				break;
			};
		case aSGUI_VERTICAL_SCROLL_BAR:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 20), MashGUIUnit(0.0f, topLeftStartPosition.y + 100));

				m_device->GetGUIManager()->AddScrollBar(rect, true, 0.1f, newParent);
				break;
			};
		case aSGUI_HORIZONTAL_SCROLL_BAR:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 100), MashGUIUnit(0.0f, topLeftStartPosition.y + 20));

				m_device->GetGUIManager()->AddScrollBar(rect, false, 0.1f, newParent);
				break;
			};
		case aGUI_TAB_CONTROL:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 100), MashGUIUnit(0.0f, topLeftStartPosition.y + 100));

				MashGUITabControl *newTabControl = m_device->GetGUIManager()->AddTabControl(rect, newParent);

				//need to handle tab changes of each tab control created
				newTabControl->RegisterReceiver(aGUIEVENT_TC_TAB_CHANGE, MashGUIEventFunctor(&MainLoop::OnTabChange, this));

				break;
			};
		case aGUI_STATIC_TEXT:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 100), MashGUIUnit(0.0f, topLeftStartPosition.y + 50));

				MashGUIComponent *newComponent = m_device->GetGUIManager()->AddStaticText(rect, newParent);

				newComponent->SetDrawDebugBounds(true);
				break;
			};
		case aGUI_TEXT_BOX:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 100), MashGUIUnit(0.0f, topLeftStartPosition.y + 50));

				MashGUIComponent *newElement = m_device->GetGUIManager()->AddTextBox(rect, newParent);
				newElement->SetEventsEnabled(false);
				break;
			};
		case aGUI_LISTBOX:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 100), MashGUIUnit(0.0f, topLeftStartPosition.y + 100));

				MashGUIComponent *newElement = m_device->GetGUIManager()->AddListBox(rect, newParent);
				newElement->SetEventsEnabled(false);
				break;
			};
		case aGUI_VIEW:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 100), MashGUIUnit(0.0f, topLeftStartPosition.y + 100));

				m_device->GetGUIManager()->AddView(rect, newParent);
				break;
			};
		case aGUI_TREE:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 100), MashGUIUnit(0.0f, topLeftStartPosition.y + 100));

				m_device->GetGUIManager()->AddTree(rect, newParent);
				break;
			};
		case aGUI_MENUBAR:
			{
				m_device->GetGUIManager()->AddMenuBar(newParent);
				break;
			};
		};
	}

	void OnResizeElement(const MashVector2 &movementConst)
	{
		if (m_selectedElement)
		{
			MashVector2 movement = movementConst;
			MashVector2 mousePosition = m_device->GetInputManager()->GetCursorPosition();

			{					
				if (g_enableSnapToGrid)
				{
					float gridX = floorf(m_lastMousePosition.x / g_gridSize);
					float gridY = floorf(m_lastMousePosition.y / g_gridSize);

					movement.x = ((int)gridX - (int)m_mouseGridPosition.x) * g_gridSize;
					movement.y = ((int)gridY - (int)m_mouseGridPosition.y) * g_gridSize;

					m_mouseGridPosition.x = gridX;
					m_mouseGridPosition.y = gridY;
				}

				MashRectangle2 futureRect = m_selectedElement->GetAbsoluteRegion();
				switch(m_actionState)
				{
				case aRMOUSE_RESIZE_TL:
					{
						//Max<float>();
						futureRect.left += movement.x;
						futureRect.top += movement.y;
						break;
					}
				case aRMOUSE_RESIZE_TR:
					{
						futureRect.right += movement.x;
						futureRect.top += movement.y;
						break;
					}
				case aRMOUSE_RESIZE_BR:
					{
						futureRect.right += movement.x;
						futureRect.bottom += movement.y;
						break;
					}
				case aRMOUSE_RESIZE_BL:
					{
						futureRect.left += movement.x;
						futureRect.bottom += movement.y;
						break;
					}
				};

				/*
					Only move the element if its totally within the parents region.
				*/
				bool allowMove = true;

				if (allowMove)
				{
					MashGUIRect destRegion = m_selectedElement->GetDestinationRegion();
					switch(m_actionState)
					{
					case aRMOUSE_RESIZE_TL:
						{
							//Max<float>();
							destRegion.left.offset += movement.x;
							destRegion.top.offset += movement.y;
							break;
						}
					case aRMOUSE_RESIZE_TR:
						{
							destRegion.right.offset += movement.x;
							destRegion.top.offset += movement.y;
							break;
						}
					case aRMOUSE_RESIZE_BR:
						{
							destRegion.right.offset += movement.x;
							destRegion.bottom.offset += movement.y;
							break;
						}
					case aRMOUSE_RESIZE_BL:
						{
							destRegion.left.offset += movement.x;
							destRegion.bottom.offset += movement.y;
							break;
						}
					};

					m_selectedElement->SetDestinationRegion(destRegion);

					UpdateFocusedTransformGUI();
				}
			}
		}
	}

	void OnDragElement(const MashVector2 &movementConst)
	{
		if (m_selectedElement)
		{
			MashVector2 movement = movementConst;

			MashVector2 mousePosition = m_device->GetInputManager()->GetCursorPosition();

			{					
				if (g_enableSnapToGrid)
				{
					float gridX = floorf(m_lastMousePosition.x / g_gridSize);
					float gridY = floorf(m_lastMousePosition.y / g_gridSize);

					movement.x = ((int)gridX - (int)m_mouseGridPosition.x) * g_gridSize;
					movement.y = ((int)gridY - (int)m_mouseGridPosition.y) * g_gridSize;

					m_mouseGridPosition.x = gridX;
					m_mouseGridPosition.y = gridY;
				}

				m_selectedElement->AddPosition(movement.x, movement.y);

				UpdateFocusedTransformGUI();
			}
		}
	}

	void OnEnableElementDrag()
	{
		if (m_selectedElement)
		{
			m_actionState = aRMOUSE_DRAG;

			m_originalDragElementRegion = m_selectedElement->GetDestinationRegion();

			//for snap to grid
			m_mouseGridPosition.x = floorf(m_lastMousePosition.x / g_gridSize);
			m_mouseGridPosition.y = floorf(m_lastMousePosition.y / g_gridSize);

			//snap the seleted object to the grid			
			m_selectedElement->AddPosition(-((int)m_selectedElement->GetDestinationRegion().left.offset % (int)g_gridSize), -((int)m_selectedElement->GetDestinationRegion().top.offset % (int)g_gridSize));

		}
	}

	void OnDragElementDisabled()
	{
		m_actionState = aRMOUSE_NONE;

		if (m_selectedElement)
		{
			/*
				Only move the element if its totally within the main view region.
			*/
			if (m_selectedElement->GetAbsoluteRegion().ClassifyGUI(m_mainEditorView->GetAbsoluteRegion()) == aCULL_CULLED)
			{
				m_selectedElement->SetDestinationRegion(m_originalDragElementRegion);
			}
			else
			{
				MashGUIView *newParent = FindParentNode(m_selectedElement, m_mainEditorView, m_device->GetInputManager()->GetCursorPosition());
				if (!newParent)
					newParent = m_mainEditorView;

				if (newParent && (newParent != m_selectedElement->GetParent()))
				{
					MashVector2 moveToParentSpace(0.0f, 0.0f);
					if (m_selectedElement->GetParent())
					{
						const MashRectangle2 *oldParentRect = &m_selectedElement->GetParent()->GetAbsoluteRegion();
						const MashRectangle2 *newParentRect = &newParent->GetAbsoluteRegion();
						moveToParentSpace.x = oldParentRect->left - newParentRect->left;
						moveToParentSpace.y = oldParentRect->top - newParentRect->top;
					}
					else
					{
						moveToParentSpace.x = -newParent->GetAbsoluteRegion().left;
						moveToParentSpace.y = -newParent->GetAbsoluteRegion().top;
					}
					
					newParent->AddChild(m_selectedElement);	
					m_selectedElement->AddPosition(moveToParentSpace.x, moveToParentSpace.y);
					
				}
			}
		}
	}

	void OnEnableElementResize(eRIGHT_MOUSE_ACTION action)
	{
		m_actionState = action;

		//for snap to grid
		m_mouseGridPosition.x = floorf(m_lastMousePosition.x / g_gridSize);
		m_mouseGridPosition.y = floorf(m_lastMousePosition.y / g_gridSize);

		MashGUIRect newDest = m_selectedElement->GetDestinationRegion();
		switch(m_actionState)
		{
		case aRMOUSE_RESIZE_TL:
			{
				//Max<float>();
				newDest.left.offset += ((int)newDest.left.offset % (int)g_gridSize);
				newDest.top.offset += ((int)newDest.top.offset % (int)g_gridSize);
				break;
			}
		case aRMOUSE_RESIZE_TR:
			{
				newDest.right.offset -= ((int)newDest.right.offset % (int)g_gridSize);
				newDest.top.offset += ((int)newDest.top.offset % (int)g_gridSize);
				break;
			}
		case aRMOUSE_RESIZE_BR:
			{
				newDest.bottom.offset -= ((int)newDest.bottom.offset % (int)g_gridSize);
				newDest.right.offset -= ((int)newDest.right.offset % (int)g_gridSize);
				break;
			}
		case aRMOUSE_RESIZE_BL:
			{
				newDest.bottom.offset -= ((int)newDest.bottom.offset % (int)g_gridSize);
				newDest.left.offset += ((int)newDest.left.offset % (int)g_gridSize);
				break;
			}
		};

		m_selectedElement->SetDestinationRegion(newDest);
	}

	void OnEvent(const sInputEvent &eventData)
	{
		if (eventData.eventType == sInputEvent::aEVENTTYPE_KEYBOARD)
		{
			switch(eventData.action)
			{
			case aKEYEVENT_CTRL:
				{
					if (eventData.isPressed && !m_openFileDialog->IsOpen())
					{
						if (m_actionStates[aMOUSEEVENT_B1])
							OnEnableElementDrag();
					}
					else
					{
						//on drag release
						if (m_actionState == aRMOUSE_DRAG)
							OnDragElementDisabled();
					}
					break;
				}
			case aKEYEVENT_DELETE:
				{
					if (m_selectedElement && m_selectedElement->GetHasFocus() && !m_openFileDialog->IsOpen())
					{
						m_selectedElement->Destroy();
						m_selectedElement = 0;

						UpdateFocusedTransformGUI();

						if (m_focusedElementProperties)
						{
							m_focusedElementProperties->SetRenderEnabled(false);
							m_focusedElementProperties = 0;
						}
					}
					break;
				}
			};
		}
		else if (eventData.eventType == sInputEvent::aEVENTTYPE_MOUSE)
		{
			MashVector2 mouseMovement = m_device->GetInputManager()->GetCursorPosition() - m_lastMousePosition;
			m_lastMousePosition = m_device->GetInputManager()->GetCursorPosition();

			switch(eventData.action)
			{
			case aMOUSEEVENT_AXISX:
			case aMOUSEEVENT_AXISY:
				{
					if (m_actionState == aRMOUSE_DRAG)
						OnDragElement(mouseMovement);
					else if (m_actionState == aRMOUSE_RESIZE_TL ||
						m_actionState == aRMOUSE_RESIZE_TR ||
						m_actionState == aRMOUSE_RESIZE_BL ||
						m_actionState == aRMOUSE_RESIZE_BR)
						OnResizeElement(mouseMovement);


					break;
				}
			case aMOUSEEVENT_B1:
				{
					if (eventData.isPressed && !m_openFileDialog->IsOpen())
					{
						m_actionState = aRMOUSE_NONE;

						
						if (m_selectedElement && m_actionStates[aKEYEVENT_CTRL])
						{
							OnEnableElementDrag();
						}
						else
						{
							//resize?
							if (m_selectedElement)
							{
								MashRectangle2 rect = m_selectedElement->GetAbsoluteRegion();
								if ((m_lastMousePosition - MashVector2(rect.left, rect.top)).Length() <= g_resizeBoxBuffer)
									OnEnableElementResize(aRMOUSE_RESIZE_TL);
								else if ((m_lastMousePosition - MashVector2(rect.right, rect.top)).Length() <= g_resizeBoxBuffer)
									OnEnableElementResize(aRMOUSE_RESIZE_TR);
								else if ((m_lastMousePosition - MashVector2(rect.right, rect.bottom)).Length() <= g_resizeBoxBuffer)
									OnEnableElementResize(aRMOUSE_RESIZE_BR);
								else if ((m_lastMousePosition - MashVector2(rect.left, rect.bottom)).Length() <= g_resizeBoxBuffer)
									OnEnableElementResize(aRMOUSE_RESIZE_BL);
								
							}
						}

						/*
							This is deferred till the end so that parent elements can resize
							even if a child is directly under the resizing tabs
						*/
						if (m_actionState == aRMOUSE_NONE)
						{
							if (m_mainEditorView->GetAbsoluteClippedRegion().IntersectsGUI(m_lastMousePosition))
							{
								UpdateSelectedItem();
							}

						}

						
					}
					else
					{
						//on drag release
						if (m_actionState == aRMOUSE_DRAG)
							OnDragElementDisabled();

						m_actionState = aRMOUSE_NONE;
					}
					
					break;
				}
			case aMOUSEEVENT_B2:
				{
					if ((eventData.isPressed == 1) && !m_openFileDialog->IsOpen())
					{
						OnRightClick(eventData);
					}
					
					break;
				}

			};
		}

		if (eventData.action < aKEYEVENT_SIZE)
			m_actionStates[eventData.action] = eventData.isPressed;
	}

	MashGUIView* FindParentNode(MashGUIComponent *possibleChild, MashGUIComponent *currentComponent, const MashVector2 &testPoint)
	{
		if (currentComponent != possibleChild)
		{
			MashGUIView *p = currentComponent->GetView();

			if (p)
			{
				MashList<MashGUIComponent*>::ConstIterator iter = p->GetChildren().Begin();
				MashList<MashGUIComponent*>::ConstIterator end = p->GetChildren().End();
				for(; iter != end; ++iter)
				{
					MashGUIView *newParent = FindParentNode(possibleChild, *iter, testPoint);
					if (newParent)
						return newParent;
				}
				
				if (p->GetAbsoluteRegion().IntersectsGUI(testPoint))
					return p;//new parent!
			}
		}

		return 0;
	}

	MashGUIComponent* GetDragableComponent(const MashVector2 &cursorCoords, MashGUIComponent *root)
	{
		MashGUIComponent *returnVal = 0;
		MashGUIView *parent = root->GetView();

		if (parent)
		{
			MashList<MashGUIComponent*>::ConstIterator iter = parent->GetChildren().Begin();
			MashList<MashGUIComponent*>::ConstIterator end = parent->GetChildren().End();
			for(; iter != end; ++iter)
			{
				returnVal = GetDragableComponent(cursorCoords, (MashGUIComponent*)*iter);
				if (returnVal)
					return returnVal;
			}
		}

		/*
			Looks at the root element rather than the parent because of the case with
			tabs where you dont want to drag the tab, but rather the tab control.
		*/
		if (!returnVal && 
			(root->GetGUIType() != aGUI_SCROLL_BAR_VIEW) &&
			(root != m_mainEditorView))
		{
			if (root->GetRenderEnabled())
			{
				if (root->GetAbsoluteClippedRegion().IntersectsGUI(cursorCoords))
					returnVal = root;
			}
		}

		return returnVal;
	}

	void Save(const char *filename)
	{
		m_device->GetGUIManager()->SaveGUILayout(filename, m_mainEditorView, false);
	}

	void UpdateFocusedTransformGUI()
	{
		MashGUIView *commonProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("CommonProperties");
		MashGUITextBox *nameTextBox = (MashGUITextBox*)commonProperties->GetElementByName("NameTextBox");
		MashGUITextBox *locaPosXTextBox = (MashGUITextBox*)commonProperties->GetElementByName("LocalPosXTextBox");
		MashGUITextBox *locaPosYTextBox = (MashGUITextBox*)commonProperties->GetElementByName("LocalPosYTextBox");
		MashGUITextBox *widthTextBox = (MashGUITextBox*)commonProperties->GetElementByName("ElementWidthTextBox");
		MashGUITextBox *heightTextBox = (MashGUITextBox*)commonProperties->GetElementByName("ElementHeightTextBox");

		nameTextBox->SetMute(true);
		locaPosXTextBox->SetMute(true);
		locaPosYTextBox->SetMute(true);
		widthTextBox->SetMute(true);
		heightTextBox->SetMute(true);

		if (!m_selectedElement)
		{
			nameTextBox->SetText("");
			locaPosXTextBox->SetTextInt(0);
			locaPosYTextBox->SetTextInt(0);
			widthTextBox->SetTextInt(0);
			heightTextBox->SetTextInt(0);
		}
		else
		{
			MashRectangle2 destRect = m_selectedElement->GetLocalVirtualRegion();
			nameTextBox->SetText(m_selectedElement->GetName());
			locaPosXTextBox->SetTextInt(destRect.left);
			locaPosYTextBox->SetTextInt(destRect.top);
			widthTextBox->SetTextInt(destRect.right - destRect.left);
			heightTextBox->SetTextInt(destRect.bottom - destRect.top);
		}

		nameTextBox->SetMute(false);
		locaPosXTextBox->SetMute(false);
		locaPosYTextBox->SetMute(false);
		widthTextBox->SetMute(false);
		heightTextBox->SetMute(false);
	}

	void UpdateListBoxItemGUI()
	{
		if (m_selectedElement)
		{
			MashGUIListBox *listBox = (MashGUIListBox*)m_selectedElement;

			if (m_currentItemsListbox->GetSelectedItemId() != -1)
				m_listBoxIconSourceRegion = listBox->GetItemIconSourceRegion(m_currentItemsListbox->GetItemUserValue(m_currentItemsListbox->GetSelectedItemId()));
			else
				m_listBoxIconSourceRegion = MashRectangle2(0.0f, 0.0f, 0.0f, 0.0f);

			MashGUIView *focusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("ListBoxProperties");

			MashGUITextBox *textBox = (MashGUITextBox*)focusedElementProperties->GetElementByName("ListBoxItemTextBox");
			textBox->SetMute(true);
			textBox->SetText(m_currentItemsListbox->GetItemText(m_currentItemsListbox->GetSelectedItemId()));
			textBox->SetMute(false);

			textBox = (MashGUITextBox*)focusedElementProperties->GetElementByName("ListBoxItemValueTextBox");
			textBox->SetMute(true);
			textBox->SetTextInt(listBox->GetItemUserValue(m_currentItemsListbox->GetItemUserValue(m_currentItemsListbox->GetSelectedItemId())));
			textBox->SetMute(false);
		}
	}

	void OnItemFocus(/*const sGUIEvent &eventData*/)
	{
		MashGUIView *newFocusedElementProperties = 0;
		if (m_selectedElement)
		{
			switch(/*eventData.component->GetGUIType()*/m_selectedElement->GetGUIType())
			{
			case aGUI_SCROLL_BAR:
				{
					newFocusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("ScrollbarProperties");
					MashGUIScrollBar *sb = (MashGUIScrollBar*)m_selectedElement;
					((MashGUITextBox*)newFocusedElementProperties->GetElementByName("ScrollbarMinNumTB"))->SetTextFloat(sb->GetSliderMinValue());
					((MashGUITextBox*)newFocusedElementProperties->GetElementByName("ScrollbarMaxNumTB"))->SetTextFloat(sb->GetSliderMaxValue());

					break;
				}
			case aGUI_SPRITE:
				{
					newFocusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("SpriteProperties");
					MashTexture *texture = ((MashGUISprite*)m_selectedElement)->GetSkin()->GetTexture();
					if (texture)
						((MashGUITextBox*)newFocusedElementProperties->GetElementByName("SpriteTextureTextBox"))->SetText(texture->GetName());

					break;
				}
			case aGUI_BUTTON:
				{
					newFocusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("ButtonProperties");
					((MashGUITextBox*)newFocusedElementProperties->GetElementByName("ButtonText"))->SetText(((MashGUIButton*)m_selectedElement)->GetText());
					break;
				}
			case aGUI_CHECK_BOX:
				{
					newFocusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("CheckBoxProperties");
					((MashGUICheckBox*)newFocusedElementProperties->GetElementByName("CheckBoxIsCheckedCheckBox"))->SetChecked(((MashGUICheckBox*)m_selectedElement)->IsChecked());
					break;
				}
			case aGUI_WINDOW:
				{
					newFocusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("WindowProperties");
					((MashGUITextBox*)newFocusedElementProperties->GetElementByName("WindowTitlebarTitleTextBox"))->SetText(((MashGUIWindow*)m_selectedElement)->GetTitleBarText());
					((MashGUICheckBox*)newFocusedElementProperties->GetElementByName("WindowEnableMinimizeCheckBox"))->SetChecked(((MashGUIWindow*)m_selectedElement)->GetMinimizeButtonEnabled());
					((MashGUICheckBox*)newFocusedElementProperties->GetElementByName("WindowEnableCloseCheckBox"))->SetChecked(((MashGUIWindow*)m_selectedElement)->GetCloseButtonEnabled());
					((MashGUICheckBox*)newFocusedElementProperties->GetElementByName("WindowEnableDragCheckBox"))->SetChecked(((MashGUIWindow*)m_selectedElement)->GetMouseDragEnabled());
					((MashGUICheckBox*)newFocusedElementProperties->GetElementByName("WindowEnableFocusLockCheckBox"))->SetChecked(((MashGUIWindow*)m_selectedElement)->LockFocusWhenActivated());
					break;
				}
			case aGUI_STATIC_TEXT:
				{
					newFocusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("StaticTextProperties");
					((MashGUITextBox*)newFocusedElementProperties->GetElementByName("StaticTextTextBox"))->SetText(((MashGUIStaticText*)m_selectedElement)->GetText());
					((MashGUICheckBox*)newFocusedElementProperties->GetElementByName("StaticTextEnableWordWrap"))->SetChecked(((MashGUIStaticText*)m_selectedElement)->GetWordWrap());
					break;
				}
			case aGUI_TEXT_BOX:
				{
					newFocusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("TextBoxProperties");

					m_device->GetGUIManager()->SetGlobalMute(true);

					((MashGUITextBox*)newFocusedElementProperties->GetElementByName("TextBoxTextBox"))->SetText(((MashGUITextBox*)m_selectedElement)->GetText());
					((MashGUITextBox*)newFocusedElementProperties->GetElementByName("TextBoxFloatPrecision"))->SetTextInt(((MashGUITextBox*)m_selectedElement)->GetFloatPrecision());
					((MashGUICheckBox*)newFocusedElementProperties->GetElementByName("TextBoxEnableIncrementButtons"))->SetChecked(((MashGUITextBox*)m_selectedElement)->GetNumberButtonState());
					//((MashGUICheckBox*)newFocusedElementProperties->GetElementByName("TextBoxEnableWordWrap"))->SetChecked(((MashGUITextBox*)m_selectedElement)->GetWordWrap());

					((MashGUITextBox*)newFocusedElementProperties->GetElementByName("TextBoxMinNum"))->SetTextInt(((MashGUITextBox*)m_selectedElement)->GetMinNumber());
					((MashGUITextBox*)newFocusedElementProperties->GetElementByName("TextBoxMaxNum"))->SetTextInt(((MashGUITextBox*)m_selectedElement)->GetMaxNumber());

					MashGUIListBox *lb = (MashGUIListBox*)newFocusedElementProperties->GetElementByName("TextBoxTextType");
					lb->SetActiveItemByUserValue(((MashGUITextBox*)m_selectedElement)->GetTextFormat());

					m_device->GetGUIManager()->SetGlobalMute(false);
					break;
				}
			case aGUI_TREE:
				{
					newFocusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("TreeProperties");
					break;
				}
			case aGUI_TAB_CONTROL:
				{
					newFocusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("TabProperties");
					MashGUITabControl *tabControl = (MashGUITabControl*)m_selectedElement;
					((MashGUITextBox*)newFocusedElementProperties->GetElementByName("TabTextBox"))->SetText(tabControl->GetTabText(tabControl->GetSelectedTabID()));
					//TabStaticText
					break;
				}
			case aGUI_LISTBOX:
				{
					newFocusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("ListBoxProperties");
					MashGUIListBox *listBox = (MashGUIListBox*)m_selectedElement;
					//set defaults
					MashRectangle2 iconOffsetRegion = listBox->GetItemIconOffsetRegion();
					MashRectangle2 textOffsetRegion = listBox->GetItemTextOffsetRegion();

					m_listBoxIconPosition.x = iconOffsetRegion.left; m_listBoxIconDim.x = iconOffsetRegion.right - iconOffsetRegion.left;
					m_listBoxIconPosition.y = iconOffsetRegion.top; m_listBoxIconDim.y = iconOffsetRegion.bottom - iconOffsetRegion.top;

					m_listBoxTextPosition.x = textOffsetRegion.left; m_listBoxTextDim.x = textOffsetRegion.right - textOffsetRegion.left;
					m_listBoxTextPosition.y = textOffsetRegion.top; m_listBoxTextDim.y = textOffsetRegion.bottom - textOffsetRegion.top;
					
					m_device->GetGUIManager()->SetGlobalMute(true);

					MashGUITextBox *textBox = (MashGUITextBox*)newFocusedElementProperties->GetElementByName("ListBoxIconRegionLeftTextBox");
					textBox->SetTextInt(m_listBoxIconPosition.y);
					textBox = (MashGUITextBox*)newFocusedElementProperties->GetElementByName("ListBoxIconRegionTopTextBox");
					textBox->SetTextInt(m_listBoxIconPosition.x);
					textBox = (MashGUITextBox*)newFocusedElementProperties->GetElementByName("ListBoxIconRegionRightTextBox");
					textBox->SetTextInt(m_listBoxIconDim.x);
					textBox = (MashGUITextBox*)newFocusedElementProperties->GetElementByName("ListBoxIconRegionBottomTextBox");
					textBox->SetTextInt(m_listBoxIconDim.y);

					textBox = (MashGUITextBox*)newFocusedElementProperties->GetElementByName("ListBoxTextRegionLeftTextBox");
					textBox->SetTextInt(m_listBoxTextPosition.x);
					textBox = (MashGUITextBox*)newFocusedElementProperties->GetElementByName("ListBoxTextRegionTopTextBox");
					textBox->SetTextInt(m_listBoxTextPosition.y);
					textBox = (MashGUITextBox*)newFocusedElementProperties->GetElementByName("ListBoxTextRegionRightTextBox");
					textBox->SetTextInt(m_listBoxTextDim.x);
					textBox = (MashGUITextBox*)newFocusedElementProperties->GetElementByName("ListBoxTextRegionBottomTextBox");
					textBox->SetTextInt(m_listBoxTextDim.y);

					int itemHeight = listBox->GetItemHeight();

					textBox = (MashGUITextBox*)newFocusedElementProperties->GetElementByName("ListBoxItemHeightTextBox");
					textBox->SetTextInt(itemHeight);

					bool iconsEnabled = listBox->GetIconsEnabled();

					MashGUICheckBox *checkBox = (MashGUICheckBox*)newFocusedElementProperties->GetElementByName("ListBoxItemIconEnabledCheckbox");
					checkBox->SetChecked(iconsEnabled);

					m_device->GetGUIManager()->SetGlobalMute(false);

					m_currentItemsListbox->ClearAllItems();
					for(unsigned int i = 0; i < listBox->GetItemCount(); ++i)
						m_currentItemsListbox->AddItem(listBox->GetItemText(i), i);

					UpdateListBoxItemGUI();

					break;
				}
			case aGUI_MENUBAR:
				{
					newFocusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("MenuBarProperties");
					MashGUIListBox *listBox = (MashGUIListBox*)newFocusedElementProperties->GetElementByName("MenuBarListBox");
					MashGUIMenuBar *menuBar = (MashGUIMenuBar*)m_selectedElement;

					listBox->ClearAllItems();

					MashArray<MashGUIMenuBar::sMenuBarItem> menuBarItems;
					menuBar->GetItems(menuBarItems);
					for(unsigned int i = 0; i < menuBarItems.Size(); ++i)
					{
						listBox->AddItem(menuBarItems[i].text, menuBarItems[i].id);
					}

					break;
				}
			};
		}

		UpdateFocusedTransformGUI();
			
		if (newFocusedElementProperties && (!m_focusedElementProperties || 
			(newFocusedElementProperties != m_focusedElementProperties)))
		{
			if (m_focusedElementProperties)
				m_focusedElementProperties->SetRenderEnabled(false);

			m_focusedElementProperties = newFocusedElementProperties;
			m_focusedElementProperties->SetRenderEnabled(true);
		}
		else if (!newFocusedElementProperties && m_focusedElementProperties)
		{
			m_focusedElementProperties->SetRenderEnabled(false);
			m_focusedElementProperties = 0;
		}
	}

	void OnGridSizeChange(const sGUIEvent &eventData)
	{
		g_gridSize = atoi(((MashGUITextBox*)eventData.component)->GetText().GetCString());
	}

	void OnGridToggleChange(const sGUIEvent &eventData)
	{
		g_enableSnapToGrid = ((MashGUICheckBox*)eventData.component)->IsChecked();
	}

	void OnMenuBarSelection(const sGUIEvent &eventData)
	{
		MashGUIPopupMenu *popup = ((MashGUIMenuBar*)eventData.component)->GetSelectedSubMenu();
		int popupValue = popup->GetItemUserValue(popup->GetSelectedItemId());
		switch(popupValue)
		{
		case 0://save
			{
				m_openFileDialodCaller = popup;
				m_openFileDialog->OpenDialog();
				break;
			}
		case 1://load
			{
				m_openFileDialodCaller = popup;
				m_openFileDialog->OpenDialog();
			}
			break;
		case 3://clear
			{
				if (m_mainEditorView)
					m_mainEditorView->DetachAllChildren();

				m_selectedElement = 0;
			}
			break;
		case 2://quit
			m_quit = true;
			break;
		}
	}

	void OnNativeResolutionWidthChange(const sGUIEvent &eventData)
	{
		MashGUIRect newRect = m_canvasSizeView->GetDestinationRegion();
		m_canvasSizeX = atoi(((MashGUITextBox*)eventData.component)->GetText().GetCString());
		newRect.right.offset = m_canvasSizeX + (g_canvasScreenBuffer * 2);
		m_canvasSizeView->SetDestinationRegion(newRect);
	}

	void OnNativeResolutionHeightChange(const sGUIEvent &eventData)
	{
		MashGUIRect newRect = m_canvasSizeView->GetDestinationRegion();
		m_canvasSizeY = atoi(((MashGUITextBox*)eventData.component)->GetText().GetCString());
		newRect.bottom.offset = m_canvasSizeY + (g_canvasScreenBuffer * 2);
		m_canvasSizeView->SetDestinationRegion(newRect);
	}

	void UpdateMenuBarGUIItem(MashGUIView *menuBarProperties, MashGUIListBox *editorItemListBox)
	{
		int itemIndex = editorItemListBox->GetSelectedItemId();
		if (itemIndex != -1)
		{
			MashGUITextBox *valTextBox = (MashGUITextBox*)menuBarProperties->GetElementByName("MenuBarItemNameTextBox");
			//disable some events as recursion might occur
			valTextBox->SetMute(true);
			valTextBox->SetText(editorItemListBox->GetItemText(itemIndex));
			valTextBox->SetMute(false);

			valTextBox = (MashGUITextBox*)menuBarProperties->GetElementByName("MenuBarItemValueTextBox");
			//disable some events as recursion might occur
			valTextBox->SetMute(true);

			if (m_currentMenuBarMenu)
				valTextBox->SetTextInt(m_currentMenuBarMenu->GetItemUserValue(editorItemListBox->GetItemUserValue(editorItemListBox->GetSelectedItemId())));
			else
			{
				valTextBox->SetTextInt(((MashGUIMenuBar*)m_selectedElement)->GetItemUserValue(editorItemListBox->GetItemUserValue(editorItemListBox->GetSelectedItemId())));
			}

			valTextBox->SetMute(false);
		}
	}

	void OnMenuBarItemValueChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUIView *focusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("MenuBarProperties");
			MashGUIListBox *listBox = (MashGUIListBox*)focusedElementProperties->GetElementByName("MenuBarListBox");

			int itemIndex = listBox->GetSelectedItemId();
			if (itemIndex != -1)
			{
				int menuItemId = listBox->GetItemUserValue(itemIndex);
				if (menuItemId != -1)
				{
					if (m_currentMenuBarMenu)
					{
						m_currentMenuBarMenu->SetMute(true);
						m_currentMenuBarMenu->SetItemUserValue(menuItemId, ((MashGUITextBox*)eventData.component)->GetTextAsInt());
						m_currentMenuBarMenu->SetMute(false);
					}
					else
					{
						MashGUIMenuBar *mb = (MashGUIMenuBar*)m_selectedElement;
						mb->SetMute(true);
						mb->SetItemValue(menuItemId, ((MashGUITextBox*)eventData.component)->GetTextAsInt());
						mb->SetMute(false);
					}
				}
			}
		}
	}

	void OnMenuBarItemNameChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUIView *focusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("MenuBarProperties");
			MashGUIListBox *listBox = (MashGUIListBox*)focusedElementProperties->GetElementByName("MenuBarListBox");

			int itemIndex = listBox->GetSelectedItemId();
			if (itemIndex != -1)
			{
				int menuItemId = listBox->GetItemUserValue(itemIndex);
				if (menuItemId != -1)
				{
					//disable some events as recursion might occur
					listBox->SetMute(true);
					
					listBox->SetItemText(itemIndex, ((MashGUITextBox*)eventData.component)->GetText());
					if (!m_currentMenuBarMenu)
					{
						((MashGUIMenuBar*)m_selectedElement)->SetMute(true);
						((MashGUIMenuBar*)m_selectedElement)->SetItemText(menuItemId, ((MashGUITextBox*)eventData.component)->GetText());
						((MashGUIMenuBar*)m_selectedElement)->SetMute(false);
					}
					else
					{
						m_currentMenuBarMenu->SetMute(true);
						m_currentMenuBarMenu->SetItemText(menuItemId, ((MashGUITextBox*)eventData.component)->GetText());
						m_currentMenuBarMenu->SetMute(false);
					}

					listBox->SetMute(false);
				}
			}
		}
	}

	void OnMenuBarListBoxChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUIView *focusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("MenuBarProperties");
			MashGUIListBox *listBox = (MashGUIListBox*)eventData.component;//(MashGUIListBox*)focusedElementProperties->GetElementByName("MenuBarListBox");

			UpdateMenuBarGUIItem(focusedElementProperties, listBox);
		}
	}

	void OnMenuBarListBoxConfirm(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUIView *focusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("MenuBarProperties");
			MashGUIListBox *listBox = (MashGUIListBox*)eventData.component;//(MashGUIListBox*)focusedElementProperties->GetElementByName("MenuBarListBox");

			int itemIndex = listBox->GetSelectedItemId();
			if (!m_currentMenuBarMenu)
			{
				if (itemIndex != -1)
				{
					m_currentMenuBarMenu = ((MashGUIMenuBar*)m_selectedElement)->GetItemSubMenu(listBox->GetItemUserValue(itemIndex));
				}
			}
			else
			{
				int menuItemId = listBox->GetItemUserValue(itemIndex);
				MashGUIPopupMenu *newPopup = m_currentMenuBarMenu->GetItemSubMenu(menuItemId);
				if (newPopup)
				{
					m_currentMenuBarMenu = newPopup;
				}
				else
				{
					//add a popup
					MashGUIPopupMenu *newPopup = m_device->GetGUIManager()->CreatePopupMenu();
					newPopup->AddItem("NewSubMenuItem", 0);
					m_currentMenuBarMenu->AttachSubMenu(menuItemId, newPopup);
					m_currentMenuBarMenu = newPopup;
				}
			}

			if (m_currentMenuBarMenu)
			{
				MashArray<MashGUIPopupMenu::sPopupItem> items;
				m_currentMenuBarMenu->GetItems(items);
				const unsigned int itemCount = items.Size();

				listBox->ClearAllItems();
				for(unsigned int i = 0; i < itemCount; ++i)
					listBox->AddItem(items[i].text, items[i].id, 0, 0);

				UpdateMenuBarGUIItem(focusedElementProperties, listBox);
			}
		}
	}

	void FillMenuBarListBox(MashGUIMenuBar *menuBar)
	{
		MashGUIView *focusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("MenuBarProperties");
		MashGUIListBox *listBox = (MashGUIListBox*)focusedElementProperties->GetElementByName("MenuBarListBox");

		if (m_currentMenuBarMenu)
		{
			MashArray<MashGUIPopupMenu::sPopupItem> items;
			m_currentMenuBarMenu->GetItems(items);
			const unsigned int itemCount = items.Size();

			listBox->ClearAllItems();
			for(unsigned int i = 0; i < itemCount; ++i)
				listBox->AddItem(items[i].text, items[i].id, 0, 0);
		}
		else
		{
			MashArray<MashGUIMenuBar::sMenuBarItem> items;
			menuBar->GetItems(items);

			listBox->ClearAllItems();
			const unsigned int itemCount = items.Size();
			for(unsigned int i = 0; i < itemCount; ++i)
				listBox->AddItem(items[i].text, items[i].id, 0, 0);
		}

		UpdateMenuBarGUIItem(focusedElementProperties, listBox);
	}

	void OnExitSubMenuButton(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{	
			if (m_currentMenuBarMenu && m_currentMenuBarMenu->GetPopupOwner())
			{
				m_currentMenuBarMenu = m_currentMenuBarMenu->GetPopupOwner();
			}
			else
			{
				m_currentMenuBarMenu = 0;
			}

			FillMenuBarListBox((MashGUIMenuBar*)m_selectedElement);
		}
	}

	void OnMenuBarAddItem(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			int itemIndex = 0;
			MashStringc itemText = "NewItem";
			MashStringc subMenuItemText = "NewSubMenuItem";
			if (m_currentMenuBarMenu)
			{
				itemIndex = m_currentMenuBarMenu->AddItem(itemText, 0);
			}
			else
			{
				MashGUIPopupMenu *newPopup = m_device->GetGUIManager()->CreatePopupMenu();
				newPopup->AddItem(subMenuItemText, 0);
				itemIndex = ((MashGUIMenuBar*)m_selectedElement)->AddItem(itemText, newPopup);
			}

			MashGUIView *focusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("MenuBarProperties");
			MashGUIListBox *listBox = (MashGUIListBox*)focusedElementProperties->GetElementByName("MenuBarListBox");
			listBox->AddItem(itemText, itemIndex);
			
			UpdateMenuBarGUIItem(focusedElementProperties, listBox);
		}
	}

	void OnMenuBarRemoveItem(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{	
			MashGUIView *focusedElementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("MenuBarProperties");
			MashGUIListBox *listBox = (MashGUIListBox*)focusedElementProperties->GetElementByName("MenuBarListBox");

			int itemUserValue = listBox->GetItemUserValue(listBox->GetSelectedItemId());

			listBox->RemoveItem(listBox->GetSelectedItemId());

			if (m_currentMenuBarMenu)
			{
				if (/*m_currentMenuBarMenu->GetPopupOwner() ||*/ (m_currentMenuBarMenu->GetItemCount() > 0))
				{
					m_currentMenuBarMenu->RemoveItem(itemUserValue);
					if (m_currentMenuBarMenu->GetItemCount() == 0)
					{
						if (m_currentMenuBarMenu->GetPopupOwner())
						{
							MashGUIPopupMenu *popupOwner = m_currentMenuBarMenu->GetPopupOwner();
							popupOwner->DetachSubMenu(m_currentMenuBarMenu->GetAttachmentIndex());
							m_currentMenuBarMenu = popupOwner;
						}
						else
							m_currentMenuBarMenu = 0;

						FillMenuBarListBox((MashGUIMenuBar*)m_selectedElement);
					}
				}
			}
			else
			{
				MashGUIMenuBar *menuBar = (MashGUIMenuBar*)m_selectedElement;
				menuBar->RemoveItem(itemUserValue);
			}			

			UpdateMenuBarGUIItem(focusedElementProperties, listBox);
		}
	}

	void OnElementNameChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			m_selectedElement->SetName(((MashGUITextBox*)eventData.component)->GetText());
		}
	}

	void OnElementLocationXChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			float newXLocation = atof(((MashGUITextBox*)eventData.component)->GetText().GetCString());
			float objectWidth = m_selectedElement->GetAbsoluteRegion().right - m_selectedElement->GetAbsoluteRegion().left;
			MashGUIRect dest = m_selectedElement->GetDestinationRegion();
			dest.left.offset = newXLocation;
			dest.right.offset = newXLocation + objectWidth;

			m_selectedElement->SetDestinationRegion(dest);
		}
	}
	
	void OnElementLocationYChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			float newYLocation = atof(((MashGUITextBox*)eventData.component)->GetText().GetCString());
			float objectHeight = m_selectedElement->GetAbsoluteRegion().bottom - m_selectedElement->GetAbsoluteRegion().top;
			MashGUIRect dest = m_selectedElement->GetDestinationRegion();
			dest.top.offset = newYLocation;
			dest.bottom.offset = newYLocation + objectHeight;

			m_selectedElement->SetDestinationRegion(dest);
		}
	}

	void OnElementWidthChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			float newWidth = atof(((MashGUITextBox*)eventData.component)->GetText().GetCString());
			MashGUIRect dest = m_selectedElement->GetDestinationRegion();
			dest.right.offset = dest.left.offset + newWidth;

			m_selectedElement->SetDestinationRegion(dest);
		}
	}

	void OnElementHeightChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			float newHeight = atof(((MashGUITextBox*)eventData.component)->GetText().GetCString());
			MashGUIRect dest = m_selectedElement->GetDestinationRegion();
			dest.bottom.offset = dest.top.offset + newHeight;

			m_selectedElement->SetDestinationRegion(dest);
		}
	}

	void OnTabChange(const sGUIEvent &eventData)
	{
		MashGUITabControl *tabControl = (MashGUITabControl*)eventData.component;
		((MashGUITextBox*)m_mainPropertiesView->GetElementByName("TabTextBox"))->SetText(tabControl->GetTabText(tabControl->GetSelectedTabID()));
	}
	
	void OnButtonTextChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			((MashGUIButton*)m_selectedElement)->SetText(((MashGUITextBox*)eventData.component)->GetText());
		}
	}

	void OnCheckBoxIsChecked(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			((MashGUICheckBox*)m_selectedElement)->SetChecked(((MashGUICheckBox*)eventData.component)->IsChecked());
		}
	}

	void OnWindowTitleBarTextChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			((MashGUIWindow*)m_selectedElement)->SetTitleBarText(((MashGUITextBox*)eventData.component)->GetText());
		}
	}

	void OnWindowEnableMinimizeCheckBox(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			((MashGUIWindow*)m_selectedElement)->EnableMinimizeButton(((MashGUICheckBox*)eventData.component)->IsChecked());
		}
	}

	void OnWindowEnableCloseCheckBox(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			((MashGUIWindow*)m_selectedElement)->EnableCloseButton(((MashGUICheckBox*)eventData.component)->IsChecked());
		}
	}

	void OnWindowEnableDragCheckBox(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			((MashGUIWindow*)m_selectedElement)->EnableMouseDrag(((MashGUICheckBox*)eventData.component)->IsChecked());
		}
	}

	void OnWindowEnableFocusLockCheckBox(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			((MashGUIWindow*)m_selectedElement)->SetLockFocusWhenActivated(((MashGUICheckBox*)eventData.component)->IsChecked());
		}
	}

	void OnStaticTextTextChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			((MashGUIStaticText*)m_selectedElement)->SetText(((MashGUITextBox*)eventData.component)->GetText());
		}
	}

	void OnStaticTextWordWrap(const sGUIEvent &eventData)
	{
		((MashGUIStaticText*)m_selectedElement)->SetWordWrap(((MashGUICheckBox*)eventData.component)->IsChecked());
	}

	void OnTextBoxTextChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			((MashGUITextBox*)m_selectedElement)->SetText(((MashGUITextBox*)eventData.component)->GetText());
		}
	}

	void OnTextBoxIncrementButtons(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUICheckBox *cb = (MashGUICheckBox*)eventData.component;
			((MashGUITextBox*)m_selectedElement)->SetNumberButtonState(cb->IsChecked());
		}
	}

	void OnTextBoxPrecision(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			((MashGUITextBox*)m_selectedElement)->SetFloatPrecision(((MashGUITextBox*)eventData.component)->GetTextAsInt());

			MashGUIView *properties = (MashGUIView*)m_mainPropertiesView->GetElementByName("TextBoxProperties");
			((MashGUITextBox*)properties->GetElementByName("TextBoxMinNum"))->SetFloatPrecision(((MashGUITextBox*)eventData.component)->GetTextAsInt());
			((MashGUITextBox*)properties->GetElementByName("TextBoxMaxNum"))->SetFloatPrecision(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		}
	}

	void OnTextBoxTextType(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUIListBox *lb = (MashGUIListBox*)eventData.component;
			((MashGUITextBox*)m_selectedElement)->SetTextFormat((eGUI_TEXT_FORMAT)lb->GetItemUserValue(lb->GetSelectedItemId()));
		}
	}

	void OnTextBoxMinNumber(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUITextBox *tb = (MashGUITextBox*)m_selectedElement;
			tb->SetNumberMinMax(((MashGUITextBox*)eventData.component)->GetTextAsInt(), tb->GetMaxNumber());

			//sets the min max limits of the textboxes based on the new settings
			MashGUIView *elementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("TextBoxProperties");
			MashGUITextBox *mintb = (MashGUITextBox*)elementProperties->GetElementByName("TextBoxMinNum", false);
			MashGUITextBox *maxtb = (MashGUITextBox*)elementProperties->GetElementByName("TextBoxMaxNum", false);
			mintb->SetNumberMinMax(math::MinFloat(), maxtb->GetTextAsFloat());
			maxtb->SetNumberMinMax(mintb->GetTextAsFloat(), math::MaxFloat());
		}
	}

	void OnTextBoxMaxNumber(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUITextBox *tb = (MashGUITextBox*)m_selectedElement;
			tb->SetNumberMinMax(tb->GetMinNumber(), ((MashGUITextBox*)eventData.component)->GetTextAsInt());

			//sets the min max limits of the textboxes based on the new settings
			MashGUIView *elementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("TextBoxProperties");
			MashGUITextBox *mintb = (MashGUITextBox*)elementProperties->GetElementByName("TextBoxMinNum", false);
			MashGUITextBox *maxtb = (MashGUITextBox*)elementProperties->GetElementByName("TextBoxMaxNum", false);
			mintb->SetNumberMinMax(math::MinFloat(), maxtb->GetTextAsFloat());
			maxtb->SetNumberMinMax(mintb->GetTextAsFloat(), math::MaxFloat());
		}
	}

	void OnTreeItemNameTextChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUITree *selectedItem = (MashGUITree*)m_selectedElement;
			selectedItem->SetItemText(selectedItem->GetSelectedItemID(), ((MashGUITextBox*)eventData.component)->GetText());
		}
	}

	void OnTreeInsertItem(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUITree *selectedItem = (MashGUITree*)m_selectedElement;
			selectedItem->AddItem("newItem", selectedItem->GetSelectedItemID());
		}
	}

	void OnTreeRemoveItem(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUITree *selectedItem = (MashGUITree*)m_selectedElement;
			selectedItem->RemoveItem(selectedItem->GetSelectedItemID());
		}
	}

	void OnTabNameTextChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUITabControl *selectedItem = (MashGUITabControl*)m_selectedElement;
			selectedItem->SetText(selectedItem->GetSelectedTabID(), ((MashGUITextBox*)eventData.component)->GetText());
		}
	}

	void OnTabInsertItem(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			((MashGUITabControl*)m_selectedElement)->AddTab("NewTab");
		}
	}

	void OnTabRemoveItem(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUITabControl *selectedItem = (MashGUITabControl*)m_selectedElement;
			selectedItem->RemoveTab(selectedItem->GetSelectedTabID());
		}
	}

	void OnListBoxItemNameChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			int selectedItemId = m_currentItemsListbox->GetItemUserValue(m_currentItemsListbox->GetSelectedItemId());
			((MashGUIListBox*)m_selectedElement)->SetItemText(selectedItemId, ((MashGUITextBox*)eventData.component)->GetText());
			m_currentItemsListbox->SetItemText(m_currentItemsListbox->GetSelectedItemId(), ((MashGUITextBox*)eventData.component)->GetText());
		}
	}

	void OnListBoxItemValueChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUIListBox *selectedListBox = (MashGUIListBox*)m_selectedElement;
			int selectedItemId = m_currentItemsListbox->GetItemUserValue(m_currentItemsListbox->GetSelectedItemId());
			/*
				Be sure not to change the user editable lb values as they contain indexes back to the scene lb.
			*/
			selectedListBox->SetItemUserValue(selectedItemId, ((MashGUITextBox*)eventData.component)->GetTextAsInt());
		}
	}

	void OnListBoxInsertItem(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			int itemId = ((MashGUIListBox*)m_selectedElement)->AddItem("newItem", 0, 0);
			m_currentItemsListbox->AddItem("newItem", itemId, 0);
			UpdateListBoxItemGUI();
		}
	}

	void OnListBoxRemoveItem(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			int selectedItemId = m_currentItemsListbox->GetItemUserValue(m_currentItemsListbox->GetSelectedItemId());
			((MashGUIListBox*)m_selectedElement)->RemoveItem(selectedItemId);
			m_currentItemsListbox->RemoveItem(m_currentItemsListbox->GetSelectedItemId());
			UpdateListBoxItemGUI();
		}
	}

	void OnListBoxLoadIconFileDialog(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			m_openFileDialodCaller = m_selectedElement;
			m_openFileDialog->OpenDialog();
		}
	}

	void OnSpriteTextureFileDialog(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			m_openFileDialodCaller = m_selectedElement;
			m_openFileDialog->OpenDialog();
		}
	}

	void OnScrollbarMinValChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUIScrollBar *sb = (MashGUIScrollBar*)m_selectedElement;
			MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
			sb->SetSliderMinMaxValues(tb->GetTextAsFloat(), sb->GetSliderMaxValue());

			//sets the min max limits of the textboxes based on the new settings
			MashGUIView *elementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("ScrollbarProperties");
			MashGUITextBox *mintb = (MashGUITextBox*)elementProperties->GetElementByName("ScrollbarMinNumTB", false);
			MashGUITextBox *maxtb = (MashGUITextBox*)elementProperties->GetElementByName("ScrollbarMaxNumTB", false);
			mintb->SetNumberMinMax(math::MinFloat(), maxtb->GetTextAsFloat());
			maxtb->SetNumberMinMax(mintb->GetTextAsFloat(), math::MaxFloat());
		}
	}

	void OnScrollbarMaxValChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			MashGUIScrollBar *sb = (MashGUIScrollBar*)m_selectedElement;
			MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
			sb->SetSliderMinMaxValues(sb->GetSliderMinValue(), tb->GetTextAsFloat());

			//sets the min max limits of the textboxes based on the new settings
			MashGUIView *elementProperties = (MashGUIView*)m_mainPropertiesView->GetElementByName("ScrollbarProperties");
			MashGUITextBox *mintb = (MashGUITextBox*)elementProperties->GetElementByName("ScrollbarMinNumTB", false);
			MashGUITextBox *maxtb = (MashGUITextBox*)elementProperties->GetElementByName("ScrollbarMaxNumTB", false);
			mintb->SetNumberMinMax(math::MinFloat(), maxtb->GetTextAsFloat());
			maxtb->SetNumberMinMax(mintb->GetTextAsFloat(), math::MaxFloat());
		}
	}

	void OnOpenFileDialogSelection(const sGUIEvent &eventData)
	{
		if (m_openFileDialodCaller)
		{
			/*
				The dialog is handled in only a few situations. And in each situation it's a different
				component using it. So we can simple check the type to determine how it will be used.
			*/
			switch(m_openFileDialodCaller->GetGUIType())
			{
			case aGUI_SPRITE:
				{
					MashStringc fullPath;
                    ConcatenatePaths(m_openFileDialog->GetSelectedFileDirectory(), m_openFileDialog->GetSelectedFileName(), fullPath);

					MashTexture *loadedTexture = 0;
					if (!fullPath.Empty())
						loadedTexture = m_device->GetRenderer()->GetTexture(fullPath);

					if (loadedTexture)
					{
						((MashGUISprite*)m_openFileDialodCaller)->GetSkin()->SetTexture(loadedTexture);
						((MashGUISprite*)m_openFileDialodCaller)->GetSkin()->ScaleSourceToTexture();
						MashGUITextBox *textureTB = (MashGUITextBox*)m_mainPropertiesView->GetElementByName("SpriteTextureTextBox");
						textureTB->SetText(fullPath);
					}

					break;
				}
			case aGUI_LISTBOX:
				{
					MashStringc fullPath;
                    ConcatenatePaths(m_openFileDialog->GetSelectedFileDirectory(), m_openFileDialog->GetSelectedFileName(), fullPath);

					MashTexture *loadedTexture = 0;
					if (!fullPath.Empty())
						loadedTexture = m_device->GetRenderer()->GetTexture(fullPath);

					
					if (loadedTexture)
					{
						MashGUIListBox *listBox = (MashGUIListBox*)m_openFileDialodCaller;
						int itemId = m_currentItemsListbox->GetItemUserValue(m_currentItemsListbox->GetSelectedItemId());
						//MashRectangle2 sourceRegion = listBox->GetItemIconSourceRegion(itemId);
						listBox->SetItemIcon(itemId, loadedTexture);
						
						MashGUITextBox *textureTB = (MashGUITextBox*)m_mainPropertiesView->GetElementByName("ListBoxItemIconTextBox");
						textureTB->SetText(fullPath);
					}

					break;
				}
			case aGUI_POPUP_MENU:
				{
					MashGUIPopupMenu *popup = (MashGUIPopupMenu*)m_openFileDialodCaller;
					int selectedItemId = popup->GetItemUserValue(popup->GetSelectedItemId());
					MashStringc fullPath;
                    ConcatenatePaths(m_openFileDialog->GetSelectedFileDirectory(), m_openFileDialog->GetSelectedFileName(), fullPath);

					switch(selectedItemId)
					{	
					case 0:
						Save(fullPath.GetCString());
						break;
					case 1:
						{
							GUICreationCallback *creationCallback = MASH_NEW_COMMON GUICreationCallback();
							m_device->GetGUIManager()->LoadGUILayout(fullPath.GetCString(), m_mainEditorView, creationCallback);
							creationCallback->Drop();
						}
						break;
					}
					
					break;
				}
			};

			m_openFileDialodCaller = 0;
		}
	}

	void OnListBoxIconRegionLeftChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			m_listBoxIconPosition.x = atoi(((MashGUITextBox*)eventData.component)->GetText().GetCString());

			MashGUIRect dest(MashGUIUnit(0.0f, m_listBoxIconPosition.x), MashGUIUnit(0.0f, m_listBoxIconPosition.y),
				MashGUIUnit(0.0f, m_listBoxIconPosition.x + m_listBoxIconDim.x), MashGUIUnit(0.0f, m_listBoxIconPosition.y + m_listBoxIconDim.y));

			((MashGUIListBox*)m_selectedElement)->SetItemIconRegion(dest);
		}
	}

	void OnListBoxIconRegionTopChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			m_listBoxIconPosition.y = atoi(((MashGUITextBox*)eventData.component)->GetText().GetCString());

			MashGUIRect dest(MashGUIUnit(0.0f, m_listBoxIconPosition.x), MashGUIUnit(0.0f, m_listBoxIconPosition.y),
				MashGUIUnit(0.0f, m_listBoxIconPosition.x + m_listBoxIconDim.x), MashGUIUnit(0.0f, m_listBoxIconPosition.y + m_listBoxIconDim.y));

			((MashGUIListBox*)m_selectedElement)->SetItemIconRegion(dest);
		}
	}

	void OnListBoxIconRegionRightChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			m_listBoxIconDim.x = atoi(((MashGUITextBox*)eventData.component)->GetText().GetCString());

			MashGUIRect dest(MashGUIUnit(0.0f, m_listBoxIconPosition.x), MashGUIUnit(0.0f, m_listBoxIconPosition.y),
				MashGUIUnit(0.0f, m_listBoxIconPosition.x + m_listBoxIconDim.x), MashGUIUnit(0.0f, m_listBoxIconPosition.y + m_listBoxIconDim.y));

			((MashGUIListBox*)m_selectedElement)->SetItemIconRegion(dest);
		}
	}

	void OnListBoxIconRegionBottomChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			m_listBoxIconDim.y = atoi(((MashGUITextBox*)eventData.component)->GetText().GetCString());

			MashGUIRect dest(MashGUIUnit(0.0f, m_listBoxIconPosition.x), MashGUIUnit(0.0f, m_listBoxIconPosition.y),
				MashGUIUnit(0.0f, m_listBoxIconPosition.x + m_listBoxIconDim.x), MashGUIUnit(0.0f, m_listBoxIconPosition.y + m_listBoxIconDim.y));

			((MashGUIListBox*)m_selectedElement)->SetItemIconRegion(dest);
		}
	}

	void OnListBoxTextRegionLeftChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			m_listBoxTextPosition.x = atoi(((MashGUITextBox*)eventData.component)->GetText().GetCString());

			MashGUIRect dest(MashGUIUnit(0.0f, m_listBoxTextPosition.x), MashGUIUnit(0.0f, m_listBoxTextPosition.y),
				MashGUIUnit(0.0f, m_listBoxTextPosition.x + m_listBoxTextDim.x), MashGUIUnit(0.0f, m_listBoxTextPosition.y + m_listBoxTextDim.y));

			((MashGUIListBox*)m_selectedElement)->SetItemTextRegion(dest);
		}
	}

	void OnListBoxTextRegionTopChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			m_listBoxTextPosition.y = atoi(((MashGUITextBox*)eventData.component)->GetText().GetCString());

			MashGUIRect dest(MashGUIUnit(0.0f, m_listBoxTextPosition.x), MashGUIUnit(0.0f, m_listBoxTextPosition.y),
				MashGUIUnit(0.0f, m_listBoxTextPosition.x + m_listBoxTextDim.x), MashGUIUnit(0.0f, m_listBoxTextPosition.y + m_listBoxTextDim.y));

			((MashGUIListBox*)m_selectedElement)->SetItemTextRegion(dest);
		}
	}

	void OnListBoxTextRegionRightChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			m_listBoxTextDim.x = atoi(((MashGUITextBox*)eventData.component)->GetText().GetCString());

			MashGUIRect dest(MashGUIUnit(0.0f, m_listBoxTextPosition.x), MashGUIUnit(0.0f, m_listBoxTextPosition.y),
				MashGUIUnit(0.0f, m_listBoxTextPosition.x + m_listBoxTextDim.x), MashGUIUnit(0.0f, m_listBoxTextPosition.y + m_listBoxTextDim.y));

			((MashGUIListBox*)m_selectedElement)->SetItemTextRegion(dest);
		}
	}

	void OnListBoxTextRegionBottomChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			m_listBoxTextDim.y = atoi(((MashGUITextBox*)eventData.component)->GetText().GetCString());

			MashGUIRect dest(MashGUIUnit(0.0f, m_listBoxTextPosition.x), MashGUIUnit(0.0f, m_listBoxTextPosition.y),
				MashGUIUnit(0.0f, m_listBoxTextPosition.x + m_listBoxTextDim.x), MashGUIUnit(0.0f, m_listBoxTextPosition.y + m_listBoxTextDim.y));

			((MashGUIListBox*)m_selectedElement)->SetItemTextRegion(dest);
		}
	}

	void OnListBoxIconEnableChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			((MashGUIListBox*)m_selectedElement)->EnableIcons(((MashGUICheckBox*)eventData.component)->IsChecked(), true);
		}
	}

	void OnListBoxIconHeightChange(const sGUIEvent &eventData)
	{
		if (m_selectedElement)
		{
			int height = atoi(((MashGUITextBox*)eventData.component)->GetText().GetCString());
			((MashGUIListBox*)m_selectedElement)->SetItemHeight(height);
		}
	}

	void OnListBoxItemChange(const sGUIEvent &eventData)
	{
		UpdateListBoxItemGUI();
	}

	bool Update(float dt)
	{
		return m_quit;
	}

	void Render()
	{
		m_device->GetRenderer()->SetRenderTargetDefault();

		m_device->GetSceneManager()->CullScene(m_camera);

		if (m_device->GetGUIManager()->BeginDraw() == aMASH_OK)
		{
			m_device->GetGUIManager()->DrawAll();

			//dont render these boxes while m_openFileDialog isactive
			if (m_selectedElement && !m_openFileDialog->IsOpen())
			{
				MashGUIComponent *parent = m_selectedElement->GetParent();
				MashRectangle2 parentRect(math::MinFloat(), math::MinFloat(), 
                                         math::MaxFloat(), math::MaxFloat());
				if (parent)
					parentRect = parent->GetAbsoluteClippedRegion();

				MashRectangle2 rect = m_selectedElement->GetAbsoluteRegion();
				MashRectangle2 tl(rect.left, rect.top, rect.left + g_resizeBoxBuffer, rect.top + g_resizeBoxBuffer);
				MashRectangle2 tr(rect.right - g_resizeBoxBuffer, rect.top, rect.right, rect.top + g_resizeBoxBuffer);
				MashRectangle2 bl(rect.left, rect.bottom - g_resizeBoxBuffer, rect.left + g_resizeBoxBuffer, rect.bottom);
				MashRectangle2 br(rect.right - g_resizeBoxBuffer, rect.bottom - g_resizeBoxBuffer, rect.right, rect.bottom);

				//cull any squares not within the parent region
				bool drawtl = (parentRect.left <= tl.left &&  parentRect.top <= tl.top);
				bool drawtr = (parentRect.right >= tr.right &&  parentRect.top <= tr.top);
				bool drawbl = (parentRect.left <= bl.left &&  parentRect.bottom >= bl.bottom);
				bool drawbr = (parentRect.right >= br.right &&  parentRect.bottom >= br.bottom);

				if (drawtl)
					m_device->GetGUIManager()->DrawSolidShape(tl, g_resizeBoxColour);
				if (drawtr)
					m_device->GetGUIManager()->DrawSolidShape(tr, g_resizeBoxColour);
				if (drawbl)
					m_device->GetGUIManager()->DrawSolidShape(bl, g_resizeBoxColour);
				if (drawbr)
					m_device->GetGUIManager()->DrawSolidShape(br, g_resizeBoxColour);

				sMashColour borderColour(0,0,0,255);
				if (drawtl)
					m_device->GetGUIManager()->DrawBorder(tl, borderColour);
				if (drawtr)
					m_device->GetGUIManager()->DrawBorder(tr, borderColour);
				if (drawbl)
					m_device->GetGUIManager()->DrawBorder(bl, borderColour);
				if (drawbr)
					m_device->GetGUIManager()->DrawBorder(br, borderColour);
			}
			
			m_device->GetGUIManager()->EndDraw();
		}
	}
};

int main()
{
	sMashDeviceSettings deviceSettings;
#ifdef USE_DIRECTX
	deviceSettings.rendererFunctPtr = CreateMashD3D10Device;
#else
    deviceSettings.rendererFunctPtr = CreateMashOpenGL3Device;
#endif
	deviceSettings.guiManagerFunctPtr = CreateMashGUI;
	deviceSettings.fullScreen = false;
	deviceSettings.screenWidth = 1024;
	deviceSettings.screenHeight = 700;
	deviceSettings.enableVSync = false;
	deviceSettings.preferredLightingMode = aLIGHT_TYPE_NONE;

	/*
		Set the gui style sheet. This is the basis for all gui in your application.
	*/
	deviceSettings.guiStyle = "GUIEditorStyle.xml";

    deviceSettings.rootPaths.PushBack("../../../../../Media/GUI");
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");
    deviceSettings.rootPaths.PushBack("../../../../DemoMedia/GUIEditor");

	MashDevice *device = CreateDevice(deviceSettings);

	if (!device)
		return 1;

	device->SetWindowCaption("GUI Editor");

	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();

	device->Drop();
    
    return 0;
}
