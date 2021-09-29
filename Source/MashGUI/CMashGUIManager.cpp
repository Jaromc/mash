//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIManager.h"
#include "MashDevice.h"
#include "MashVideo.h"
#include "CMashGUIRenderBatch.h"
#include "CMashGUIFontBatch.h"
#include "CMashGUILineBatch.h"
#include "CMashGUIPrimitiveBatch.h"
#include "MashLog.h"

#include "CMashGUIButton.h"
#include "CMashGUISprite.h"
#include "CMashGUIWindow.h"
#include "CMashGUISprite.h"
#include "CMashGUIScrollBar.h"
#include "CMashGUIScrollbarView.h"
#include "CMashGUICheckBox.h"
#include "MashGUIComponent.h"
#include "CMashGUITabControl.h"
#include "CMashGUIStaticText.h"
#include "CMashGUIFont.h"
#include "CMashGUIListBox.h"

#include "CMashGUIStyle.h"

#include "CMashGUIPopupMenu.h"
#include "CMashGUIMenuBar.h"
#include "CMashGUITextBox.h"
#include "CMashGUIOpenFileDialog.h"
#include "CMashGUIView.h"
#include "CMashGUITree.h"
#include "CMashGUIViewport.h"

#include "CMashGUIDebugLog.h"

#include "CMashGUIReader.h"
#include "CMashGUIWriter.h"

#include "MashCamera.h"
#include "MashGenericScriptReader.h"
#include <algorithm>
namespace mash
{
	MashGUIManager* CreateMashGUI()
	{
		CMashGUIManager *guiManager = MASH_NEW_COMMON CMashGUIManager();
		return guiManager;
	}

	CMashGUIManager::CMashGUIManager():MashGUIManager(),
		m_pRenderer(0), m_pFocus(0),
		m_activeStyle(0),m_pFontBatch(0), m_pLineBatch(0), m_pRenderBatch(0), m_pPrimitiveBatch(0),
		m_pMouseHoverComponent(0), m_beginDrawCalled(false), m_debugDrawColour(255, 255, 255, 255), m_pRootWindow(0)
	{
	}

	CMashGUIManager::~CMashGUIManager()
	{
		std::map<MashStringc, MashGUIStyle*>::iterator styleIter = m_styles.begin();
		std::map<MashStringc, MashGUIStyle*>::iterator styleIterEnd = m_styles.end();
		for(; styleIter != styleIterEnd; ++styleIter)
		{
			styleIter->second->Drop();
		}

		m_styles.clear();

		std::map<MashStringc, MashGUIFont*>::iterator fontIter = m_fonts.begin();
		std::map<MashStringc, MashGUIFont*>::iterator fontIterEnd = m_fonts.end();
		for(; fontIter != fontIterEnd; ++fontIter)
		{
			fontIter->second->Drop();
		}

		m_fonts.clear();

		if (m_pRootWindow)
		{
			m_pRootWindow->Drop();
			m_pRootWindow = 0;
		}

		if (m_pRenderBatch)
		{
			m_pRenderBatch->Drop();
			m_pRenderBatch = 0;
		}

		if (m_pFontBatch)
		{
			m_pFontBatch->Drop();
			m_pFontBatch = 0;
		}

		if (m_pLineBatch)
		{
			m_pLineBatch->Drop();
			m_pLineBatch = 0;
		}

		if (m_pPrimitiveBatch)
		{
			m_pPrimitiveBatch->Drop();
			m_pPrimitiveBatch = 0;
		}

		if (m_guiFactory)
		{
			m_guiFactory->Drop();
			m_guiFactory = 0;
		}

		ProcessDestroyList();
	}

	eMASH_STATUS CMashGUIManager::_Initialise(const mash::sMashDeviceSettings &settings, MashVideo *pRenderer, MashInputManager *pInputManager)
	{
		m_pRenderer = pRenderer;
		m_inputManager = pInputManager;

		m_guiFactory = MASH_NEW_COMMON CMashGUIFactory(this, m_inputManager, pRenderer);

		m_pRenderBatch = MASH_NEW_COMMON CMashGUIRenderBatch(this, m_pRenderer);
		if (m_pRenderBatch->Initialise() == aMASH_FAILED)
			return aMASH_FAILED;

		m_pFontBatch = MASH_NEW_COMMON CMashGUIFontBatch(this, m_pRenderer);
		if (m_pFontBatch->Initialise() == aMASH_FAILED)
			return aMASH_FAILED;

		m_pLineBatch = MASH_NEW_COMMON CMashGUILineBatch(this, m_pRenderer);
		if (m_pLineBatch->Initialise() == aMASH_FAILED)
			return aMASH_FAILED;

		m_pPrimitiveBatch = MASH_NEW_COMMON CMashGUIPrimitiveBatch(this, m_pRenderer);
		if (m_pPrimitiveBatch->Initialise() == aMASH_FAILED)
			return aMASH_FAILED;

        MashStringc guiStyleFile = settings.guiStyle;
        if (guiStyleFile.Empty())
        {
            // use default file if the user hasnt stated one.
            guiStyleFile = "MashGUIStyleDefault.xml";
        }
		
        if (LoadGUIStyle(guiStyleFile.GetCString()) == aMASH_FAILED)
        {
            MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_WARNING, "CMashGUIManager::Initialise",
                                "Failed to load gui style file '%s'", guiStyleFile.GetCString());
            return aMASH_FAILED;
        }

		mash::sMashViewPort viewPort = m_pRenderer->GetViewport();
		MashGUIRect rect(MashGUIUnit(0.0f, viewPort.x),MashGUIUnit(0.0f, viewPort.y),
			MashGUIUnit(0.0f, viewPort.x + viewPort.width),MashGUIUnit(0.0f, viewPort.y + viewPort.height));

		m_pRootWindow = m_guiFactory->CreateView(rect, 0);
		m_pRootWindow->SetHorizontalScrollState(false);
		m_pRootWindow->SetVerticalScrollState(false);
		m_pRootWindow->SetRenderBackgroundState(false);

		m_inputManager->RegisterReceiver(MashInputEventFunctor(&CMashGUIManager::OnEvent,this));
		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIManager::CreateDefaultGUIStyle()
	{
		m_activeStyle = GetGUIStyle("DefaultStyle");

		mash::sMashColour baseColourUp(120, 120, 120, 255);
		mash::sMashColour baseColourDown(193, 202, 255, 255);
		mash::sMashColour baseColourHover(193, 202, 255, 255);

		mash::sMashColour fontColourUp(255, 255, 255, 255);
		mash::sMashColour fontColourDown(255, 255, 255, 255);
		mash::sMashColour fontColourHover(255, 255, 255, 255);
		mash::sMashColour fontColourTreeSelect(100, 100, 255, 255);

		mash::MashTexture *pBaseTexture = m_pRenderer->GetTexture("MashGUIDefault.png");
		MashGUIFont *font = GetFont(m_pRenderer->GetTexture("FontTexture.DDS"),"FontTextureData.bin");
		m_activeStyle->SetFont(font);

		if (!m_activeStyle->GetFont())
			return aMASH_FAILED;

		if (!pBaseTexture)
			return aMASH_FAILED;

		MashGUISkin *pSkin = 0;
		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_BUTTON, aGUI_ATTRIB_BUTTON_UP);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(0.0f, 416.0f, 96.0f, 480.0f);
		pSkin->baseColour = baseColourUp;
		pSkin->fontColour = fontColourUp;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_BUTTON, aGUI_ATTRIB_BUTTON_DOWN);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(128.0f, 416.0f, 224.0f, 480.0f);
		pSkin->baseColour = baseColourDown;
		pSkin->fontColour = fontColourDown;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_BUTTON, aGUI_ATTRIB_BUTTON_HOVER);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(128.0f, 416.0f, 224.0f, 480.0f);
		pSkin->baseColour = baseColourHover;
		pSkin->fontColour = fontColourHover;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_CHECKBOX, aGUI_ATTRIB_CHECKBOX_ON);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(192.0f, 0.0f, 256.0f, 64.0f);
		pSkin->baseColour = baseColourUp;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_CHECKBOX, aGUI_ATTRIB_CHECKBOX_OFF);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(288.0f, 0.0f, 352.0f, 64.0f);
		pSkin->baseColour = baseColourUp;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWUP_UP);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(0.0f, 96.0f, 64.0f, 160.0f);
		pSkin->baseColour = baseColourUp;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWUP_DOWN);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(0.0f, 96.0f, 64.0f, 160.0f);
		pSkin->baseColour = baseColourDown;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWUP_HOVER);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(0.0f, 96.0f, 64.0f, 160.0f);
		pSkin->baseColour = baseColourHover;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWDOWN_UP);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(288.0f, 96.0f, 352.0f, 160.0f);
		pSkin->baseColour = baseColourUp;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWDOWN_DOWN);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(288.0f, 96.0f, 352.0f, 160.0f);
		pSkin->baseColour = baseColourDown;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWDOWN_HOVER);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(288.0f, 96.0f, 352.0f, 160.0f);
		pSkin->baseColour = baseColourHover;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWLEFT_UP);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(96.0f, 96.0f, 160.0f, 160.0f);
		pSkin->baseColour = baseColourUp;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWLEFT_DOWN);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(96.0f, 96.0f, 160.0f, 160.0f);
		pSkin->baseColour = baseColourDown;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWLEFT_HOVER);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(96.0f, 96.0f, 160.0f, 160.0f);
		pSkin->baseColour = baseColourHover;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWRIGHT_UP);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(192.0f, 96.0f, 256.0f, 160.0f);
		pSkin->baseColour = baseColourUp;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWRIGHT_DOWN);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(192.0f, 96.0f, 256.0f, 160.0f);
		pSkin->baseColour = baseColourDown;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_ARROWRIGHT_HOVER);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(192.0f, 96.0f, 256.0f, 160.0f);
		pSkin->baseColour = baseColourHover;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_SLIDER_UP);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(448.0f, 192.0f, 480.0f, 384.0f);
		pSkin->baseColour = baseColourUp;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_SLIDER_DOWN);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(448.0f, 192.0f, 480.0f, 384.0f);
		pSkin->baseColour = baseColourDown;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_SLIDER_HOVER);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(448.0f, 192.0f, 480.0f, 384.0f);
		pSkin->baseColour = baseColourHover;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_BACKGROUND);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(384.0f, 192.0f, 416.0f, 384.0f);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SCROLLBAR, aGUI_ATTRIB_SCROLLBAR_END_CAP);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(480.0f, 0.0f, 512.0f, 32.0f);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_TAB, aGUI_ATTRIB_TAB_ACTIVE);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(256.0f, 416.0f, 352.0f, 480.0f);
		pSkin->baseColour = baseColourDown;
		pSkin->fontColour = fontColourDown;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_TAB, aGUI_ATTRIB_TAB_HOVER);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(256.0f, 416.0f, 352.0f, 480.0f);
		pSkin->baseColour = baseColourHover;
		pSkin->fontColour = fontColourHover;
		
		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_TAB, aGUI_ATTRIB_TAB_INACTIVE);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(384.0f, 416.0f, 480.0f, 480.0f);
		pSkin->baseColour = baseColourUp;
		pSkin->fontColour = fontColourUp;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_LISTBOX, aGUI_ATTRIB_ITEM_INACTIVE);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(96.0f, 0.0f, 160.0f, 64.0f);
		pSkin->baseColour = baseColourUp;
		pSkin->fontColour = fontColourUp;
		
		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_LISTBOX, aGUI_ATTRIB_ITEM_ACTIVE);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(0.0f, 0.0f, 64.0f, 64.0f);
		pSkin->baseColour = baseColourDown;
		pSkin->fontColour = fontColourDown;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_LISTBOX, aGUI_ATTRIB_BACKGROUND);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(96.0f, 192.0f, 160.0f, 256.0f);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_TREE, aGUI_ATTRIB_INCREMENT);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(384.0f, 0.0f, 448.0f, 64.0f);
		pSkin->baseColour = baseColourUp;
		pSkin->fontColour = fontColourUp;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_TREE, aGUI_ATTRIB_DECREMENT);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(384.0f, 96.0f, 448.0f, 160.0f);
		pSkin->baseColour = baseColourUp;
		pSkin->fontColour = fontColourUp;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_WINDOW, aGUI_ATTRIB_WINDOW_CLOSE_UP);
		*pSkin = *m_activeStyle->GetAttributeSkin(aGUI_ELEMENT_CHECKBOX, aGUI_ATTRIB_CHECKBOX_ON);
		pSkin->baseColour = baseColourUp;
		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_WINDOW, aGUI_ATTRIB_WINDOW_CLOSE_DOWN);
		*pSkin = *m_activeStyle->GetAttributeSkin(aGUI_ELEMENT_CHECKBOX, aGUI_ATTRIB_CHECKBOX_ON);
		pSkin->baseColour = baseColourDown;
		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_WINDOW, aGUI_ATTRIB_WINDOW_CLOSE_HOVER);
		*pSkin = *m_activeStyle->GetAttributeSkin(aGUI_ELEMENT_CHECKBOX, aGUI_ATTRIB_CHECKBOX_ON);
		pSkin->baseColour = baseColourHover;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_WINDOW, aGUI_ATTRIB_WINDOW_MINIMIZE_UP);
		*pSkin = *m_activeStyle->GetAttributeSkin(aGUI_ELEMENT_TREE, aGUI_ATTRIB_DECREMENT);
		pSkin->baseColour = baseColourUp;
		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_WINDOW, aGUI_ATTRIB_WINDOW_MINIMIZE_DOWN);
		*pSkin = *m_activeStyle->GetAttributeSkin(aGUI_ELEMENT_TREE, aGUI_ATTRIB_DECREMENT);
		pSkin->baseColour = baseColourDown;
		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_WINDOW, aGUI_ATTRIB_WINDOW_MINIMIZE_HOVER);
		*pSkin = *m_activeStyle->GetAttributeSkin(aGUI_ELEMENT_TREE, aGUI_ATTRIB_DECREMENT);
		pSkin->baseColour = baseColourHover;

		//uses the same as the list box items
		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_TREE, aGUI_ATTRIB_ITEM_INACTIVE);
		*pSkin = *m_activeStyle->GetAttributeSkin(aGUI_ELEMENT_LISTBOX, aGUI_ATTRIB_ITEM_INACTIVE);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_TREE, aGUI_ATTRIB_ITEM_ACTIVE);
		*pSkin = *m_activeStyle->GetAttributeSkin(aGUI_ELEMENT_LISTBOX, aGUI_ATTRIB_ITEM_ACTIVE);
		pSkin->fontColour = fontColourTreeSelect;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_TREE, aGUI_ATTRIB_BACKGROUND);
		*pSkin = *m_activeStyle->GetAttributeSkin(aGUI_ELEMENT_LISTBOX, aGUI_ATTRIB_BACKGROUND);
		
		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_POPUP, aGUI_ATTRIB_ITEM_INACTIVE);
		*pSkin = *m_activeStyle->GetAttributeSkin(aGUI_ELEMENT_LISTBOX, aGUI_ATTRIB_ITEM_INACTIVE);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_POPUP, aGUI_ATTRIB_ITEM_ACTIVE);
		*pSkin = *m_activeStyle->GetAttributeSkin(aGUI_ELEMENT_LISTBOX, aGUI_ATTRIB_ITEM_ACTIVE);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_POPUP, aGUI_ATTRIB_BACKGROUND);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(192.0f, 192.0f, 256.0f, 256.0f);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_MENUBAR, aGUI_ATTRIB_ITEM_INACTIVE);
		*pSkin = *m_activeStyle->GetAttributeSkin(aGUI_ELEMENT_LISTBOX, aGUI_ATTRIB_ITEM_INACTIVE);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_MENUBAR, aGUI_ATTRIB_ITEM_ACTIVE);
		*pSkin = *m_activeStyle->GetAttributeSkin(aGUI_ELEMENT_LISTBOX, aGUI_ATTRIB_ITEM_ACTIVE);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_MENUBAR, aGUI_ATTRIB_BACKGROUND);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(0.0f, 352.0f, 192.0f, 384.0f);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_TAB, aGUI_ATTRIB_BACKGROUND);
		*pSkin = *m_activeStyle->AddAttribute(aGUI_ELEMENT_MENUBAR, aGUI_ATTRIB_BACKGROUND);


		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_WINDOW, aGUI_ATTRIB_TITLEBAR);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(0.0f, 288.0f, 192.0f, 320.0f);
		pSkin->renderBoarder = true;

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_WINDOW, aGUI_ATTRIB_BACKGROUND);
		pSkin->SetTexture(pBaseTexture);
		pSkin->borderColour = mash::sMashColour(255,255,255,255);
		pSkin->renderBoarder = true;
		pSkin->baseSource = mash::MashRectangle2(0.0f, 192.0f, 64.0f, 256.0f);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_TEXTBOX, aGUI_ATTRIB_BACKGROUND);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(288.0f, 192.0f, 352.0f, 256.0f);
		pSkin->baseColour = mash::sMashColour(255,255,255,255);
		pSkin->fontColour = mash::sMashColour(255,255,255,255);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_STATIC_TEXTBOX, aGUI_ATTRIB_BACKGROUND);
		*pSkin = *m_activeStyle->AddAttribute(aGUI_ELEMENT_TEXTBOX, aGUI_ATTRIB_BACKGROUND);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_SPRITE, aGUI_ATTRIB_BACKGROUND);
		pSkin->baseColour = mash::sMashColour(255,255,255,255);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_FILE_DIALOG, aGUI_ATTRIB_FILE_ICON);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(224.0f, 288.0f, 256.0f, 320.0f);

		pSkin = m_activeStyle->AddAttribute(aGUI_ELEMENT_FILE_DIALOG, aGUI_ATTRIB_FOLDER_ICON);
		pSkin->SetTexture(pBaseTexture);
		pSkin->baseSource = mash::MashRectangle2(256.0f, 288.0f, 288.0f, 320.0f);

		return aMASH_OK;
	}

	MashGUIViewport* CMashGUIManager::AddViewport(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement)
	{
		if (!pParent)
			pParent = m_pRootWindow;

		MashGUIViewport *pNewElement = m_guiFactory->CreateViewport(destRegion, pParent, styleElement);

		return pNewElement;
	}

	MashGUISprite* CMashGUIManager::AddSprite(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement)
	{
		if (!pParent)
			pParent = m_pRootWindow;

		MashGUISprite *pNewElement = m_guiFactory->CreateSprite(destRegion, pParent, styleElement);

		return pNewElement;
	}

	MashGUIButton* CMashGUIManager::AddButton(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement)
	{
		if (!pParent)
			pParent = m_pRootWindow;

		MashGUIButton *pNewElement = m_guiFactory->CreateButton(destRegion, pParent, styleElement);

		return pNewElement;
	}

	MashGUIWindow* CMashGUIManager::AddWindow(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement)
	{
		if (!pParent)
			pParent = m_pRootWindow;

		MashGUIWindow *pNewElement = m_guiFactory->CreateWindowView(destRegion, pParent, styleElement);

		return pNewElement;
	}

	MashGUICheckBox* CMashGUIManager::AddCheckBox(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement)
	{
		if (!pParent)
			pParent = m_pRootWindow;

		MashGUICheckBox *pNewElement = m_guiFactory->CreateCheckBox(destRegion, pParent, styleElement);

		return pNewElement;
	}

	MashGUIStaticText* CMashGUIManager::AddStaticText(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement)
	{
		if (!pParent)
			pParent = m_pRootWindow;

		MashGUIStaticText *pNewElement = m_guiFactory->CreateStaticText(destRegion, pParent, styleElement);

		return pNewElement;
	}

	MashGUIWindow* CMashGUIManager::AddDebugLogWindow(const MashGUIRect &destRegion, uint32 logFlags, uint32 maxMessageCount)
	{
		MashGUIWindow *windowElement = m_guiFactory->CreateWindowView(destRegion, m_pRootWindow);
		windowElement->SetLockFocusWhenActivated(true);
		windowElement->EnableCloseButton(true);
		windowElement->SetCloseButtonEvent(MashGUIWindow::aCLOSE_AND_HIDE);
		windowElement->GetView()->SetHorizontalScrollState(true);
		windowElement->GetView()->SetVerticalScrollState(true);
		windowElement->GetView()->SetRenderBackgroundState(true);

		MashGUIRect logRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f), MashGUIUnit(1.0f, 0.0f), MashGUIUnit(1.0f, 0.0f));
		MashGUIStaticText *debugLogElement = m_guiFactory->CreateDebugLog(logRegion, 0, logFlags, maxMessageCount);
		debugLogElement->AutoResizeToFitText(true);
		debugLogElement->SetRenderBackground(false);
		debugLogElement->SetWordWrap(false);
		windowElement->GetView()->AddChild(debugLogElement);
		debugLogElement->Drop();

		return windowElement;
	}

	MashGUIFont* CMashGUIManager::GetFont(mash::MashTexture *pTexture, const int8 *sDataFileName)
	{
		//set if its already been loaded
		std::map<MashStringc, MashGUIFont*>::iterator fontIter = m_fonts.find(sDataFileName);
		if (fontIter != m_fonts.end())
			return fontIter->second;

		CMashGUIFont *pNewElement = MASH_NEW_COMMON CMashGUIFont();
		if (pNewElement->LoadFont(pTexture, sDataFileName) == aMASH_FAILED)
		{
			MASH_DELETE pNewElement;
			pNewElement = 0;
		}

		m_fonts.insert(std::make_pair(sDataFileName, pNewElement));
		
		return pNewElement;
	}

	MashGUIView* CMashGUIManager::AddView(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement)
	{
		if (!pParent)
			pParent = m_pRootWindow;

		MashGUIView *pNewElement = m_guiFactory->CreateView(destRegion, pParent, styleElement);

		return pNewElement;
	}

	MashGUIListBox* CMashGUIManager::AddListBox(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement)
	{
		if (!pParent)
			pParent = m_pRootWindow;

		MashGUIListBox *pNewElement = m_guiFactory->CreateListBox(destRegion, pParent, styleElement);

		return pNewElement;
	}

	MashGUITree* CMashGUIManager::AddTree(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement)
	{
		if (!pParent)
			pParent = m_pRootWindow;

		MashGUITree *pNewElement = m_guiFactory->CreateTree(destRegion, pParent, styleElement);

		return pNewElement;
	}

	MashGUIOpenFileDialog* CMashGUIManager::CreateOpenFileDialog(const MashGUIRect &destRegion)
	{
		return m_guiFactory->CreateOpenFileDialog(destRegion);
	}

	MashGUIPopupMenu* CMashGUIManager::CreatePopupMenu(int32 styleElement)
	{
		return m_guiFactory->CreatePopupMenu(styleElement);
	}

	MashGUIMenuBar* CMashGUIManager::AddMenuBar(MashGUIView *pParent, const MashGUIUnit &left, const MashGUIUnit &right, int32 styleElement)
	{
		if (!pParent)
			pParent = m_pRootWindow;



		MashGUIMenuBar *pNewElement = m_guiFactory->CreateMenuBar(pParent, left, right, styleElement);

		return pNewElement;
	}

	MashGUITabControl* CMashGUIManager::AddTabControl(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement)
	{
		if (!pParent)
			pParent = m_pRootWindow;

		MashGUITabControl *pNewElement = m_guiFactory->CreateTabControl(destRegion, pParent, styleElement);

		return pNewElement;
	}

	MashGUIScrollBar* CMashGUIManager::AddScrollBar(const MashGUIRect &destRegion, bool isVertical, f32 incrementAmount, MashGUIView *pParent, int32 styleElement)
	{
		if (!pParent)
			pParent = m_pRootWindow;

		MashGUIScrollBar *pNewElement = m_guiFactory->CreateScrollBar(destRegion, isVertical, incrementAmount, pParent, styleElement);

		return pNewElement;
	}

	MashGUITextBox* CMashGUIManager::AddTextBox(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement)
	{
		if (!pParent)
			pParent = m_pRootWindow;

		MashGUITextBox *pNewElement = m_guiFactory->CreateTextBox(destRegion, pParent, styleElement);

		return pNewElement;
	}

	void CMashGUIManager::OnFocusLost(MashGUIComponent *elm)
	{
		sGUIEvent newGUIMsg;
		
		newGUIMsg.GUIEvent = aGUIEVENT_LOST_INPUTFOCUS;
		newGUIMsg.component = elm;
		ImmediateBroadcast(newGUIMsg);	
	}

	void CMashGUIManager::OnFocus(MashGUIComponent *elm)
	{
		sGUIEvent newGUIMsg;
		
		newGUIMsg.GUIEvent = aGUIEVENT_INPUTFOCUS;
		newGUIMsg.component = elm;
		ImmediateBroadcast(newGUIMsg);	
	}

	void CMashGUIManager::SetFocusedElement(MashGUIComponent *pHasFocus)
	{
		if (m_pFocus == pHasFocus)
			return;

		if (pHasFocus && !pHasFocus->GetCanHaveFocus())
			return;

		if (m_pFocus)
		{
			m_pFocus->_SetHasFocus(false);
			//stop rare recursion from happening
			MashGUIComponent *temp = m_pFocus;
			m_pFocus = 0;
			OnFocusLost(temp);
		}

		m_pFocus = pHasFocus;

		if (m_pFocus)
		{
			m_pFocus->_SetHasFocus(true);
			
			//This may have issue if the mouse is not actually over the compoenent
			m_pMouseHoverComponent = m_pFocus;

			OnFocus(m_pFocus);
		}
	}

	void CMashGUIManager::OnResize()
	{
		mash::sMashViewPort viewPort = m_pRenderer->GetViewport();
		MashGUIRect rect(MashGUIUnit(0.0f, viewPort.x),MashGUIUnit(0.0f, viewPort.y),
			MashGUIUnit(0.0f, viewPort.x + viewPort.width),MashGUIUnit(0.0f, viewPort.y + viewPort.height));

		m_pRootWindow->SetDestinationRegion(rect);
	}

	void CMashGUIManager::_DestroyElement(MashGUIComponent *element)
	{
		OnHideElement(element);
		m_destroyList.PushBack(element);
	}

	void CMashGUIManager::OnShowElement(MashGUIComponent *element)
	{
		if (element->LockFocusWhenActivated())
		{
			element->SendToFront();
			MashList<MashGUIComponent*>::Iterator iter = m_focusLockStack.Search(element);
			if (iter == m_focusLockStack.End())
				m_focusLockStack.PushBack(element);
		}
	}

	void CMashGUIManager::OnHideElement(MashGUIComponent *element)
	{
		if (m_pFocus == element)
			m_pFocus = 0;

		if (m_pMouseHoverComponent == element)
			m_pMouseHoverComponent = 0;

		if (element->LockFocusWhenActivated())
		{
			MashList<MashGUIComponent*>::Iterator iter = m_focusLockStack.Search(element);
			if (iter != m_focusLockStack.End())
				m_focusLockStack.Erase(iter);
		}
	}

	eMASH_STATUS CMashGUIManager::LoadGUILayout(const int8 *layoutFileName, MashGUIView *root, MashGUILoadCallback *callback)
	{
		if (!root)
			root = m_pRootWindow;

		CMashGUIReader guiReader;
		return guiReader.Load(MashDevice::StaticDevice, layoutFileName, root, callback);
	}

	eMASH_STATUS CMashGUIManager::SaveGUILayout(const int8 *layoutFileName, MashGUIComponent *root, bool saveRoot)
	{
		CMashGUIWriter guiWriter;
		return guiWriter.Save(MashDevice::StaticDevice, layoutFileName, root, saveRoot);
	}

	eMASH_STATUS CMashGUIManager::SaveGUIStyle(const int8 *styleFileName, const MashArray<MashGUIStyle*> &styles)
	{
		MashXMLWriter *xmlWriter = MashDevice::StaticDevice->GetFileManager()->CreateXMLWriter(styleFileName, "root");
		for(uint32 i = 0; i < styles.Size(); ++i)
		{
			MashGUIStyle *activeStyle = styles[i];

			xmlWriter->WriteChild("style");
			xmlWriter->WriteAttributeString("name", activeStyle->GetStyleName().GetCString());

			if (activeStyle->GetFont())
			{
				xmlWriter->WriteAttributeString("fonttexture", activeStyle->GetFont()->GetFontTexture()->GetName().GetCString());
				xmlWriter->WriteAttributeString("fontformat", activeStyle->GetFont()->GetFontFormatFileName().GetCString());
			}
			
			MashArray<MashGUIStyle::sCollectionStyle> elementCollection;
			activeStyle->CollectStyleData(elementCollection);
			for(uint32 elm = 0; elm < elementCollection.Size(); ++elm)
			{
				xmlWriter->WriteChild("element");
				xmlWriter->WriteAttributeString("name", elementCollection[elm].styleName.GetCString());

				for(uint32 attrib = 0; attrib < elementCollection[elm].attributes.Size(); ++attrib)
				{
					xmlWriter->WriteChild("attribute");

					MashGUISkin *currentSkin = elementCollection[elm].attributes[attrib].skin;

					xmlWriter->WriteAttributeString("name", GetGUIStyleAttributeString(elementCollection[elm].attributes[attrib].attrib));

					if (currentSkin->GetTexture())
						xmlWriter->WriteAttributeString("texture", currentSkin->GetTexture()->GetName().GetCString());

					xmlWriter->WriteAttributeDouble("sourcetop", currentSkin->baseSource.top);
					xmlWriter->WriteAttributeDouble("sourceleft", currentSkin->baseSource.left);
					xmlWriter->WriteAttributeDouble("sourcebottom", currentSkin->baseSource.bottom);
					xmlWriter->WriteAttributeDouble("sourceright", currentSkin->baseSource.right);
					xmlWriter->WriteAttributeInt("colour", currentSkin->baseColour.colour);
					xmlWriter->WriteAttributeInt("istransparent", currentSkin->isTransparent);
					xmlWriter->WriteAttributeDouble("alphamaskthreshold", currentSkin->alphaMaskThreshold);
					xmlWriter->WriteAttributeInt("fontcolour", currentSkin->fontColour.colour);
					xmlWriter->WriteAttributeInt("bordercolour", currentSkin->borderColour.colour);
					xmlWriter->WriteAttributeInt("drawborder", currentSkin->renderBoarder);
					
					xmlWriter->PopChild();
				}

				xmlWriter->PopChild();
			}
			
			xmlWriter->PopChild();
		}

		xmlWriter->SaveAndDestroy();

		return aMASH_OK;
	}

	eMASH_STATUS CMashGUIManager::LoadGUIStyle(const int8 *styleFileName, const int8 **customElements)
	{
		MashXMLReader *xmlReader = MashDevice::StaticDevice->GetFileManager()->CreateXMLReader(styleFileName);
		
		if (xmlReader->MoveToFirstChild("style"))
		{
			MashStringc fontTextureString;
			MashStringc fontFormatFile;

			MashStringc styleName;
			MashStringc elementName;
			MashStringc attributeName;
			MashStringc textureName;
			int32 tempBool = 0;
			int32 tempInt = 0;
			do
			{
				xmlReader->GetAttributeString("name", styleName);
				MashGUIStyle *newStyle = GetGUIStyle(styleName);

				if (newStyle)
				{
					fontTextureString.Clear();
					fontFormatFile.Clear();
					xmlReader->GetAttributeString("fonttexture", fontTextureString);
					xmlReader->GetAttributeString("fontformat", fontFormatFile);
					newStyle->SetFont(GetFont(m_pRenderer->GetTexture(fontTextureString), fontFormatFile.GetCString()));
				}

				if(newStyle && xmlReader->MoveToFirstChild("element"))
				{
					do
					{
						xmlReader->GetAttributeString("name", elementName);

						int32 elementId = aGUI_ELEMENT_UNDEFINED;
						if (customElements)
						{
							int32 i = 0;
							while(customElements[i])
							{
								if (scriptreader::CompareStrings(elementName.GetCString(), customElements[i]))
								{
									elementId = (eGUI_STYLE_ELEMENT)i;
									break;
								}

								++i;
							}
						}
						else
						{
							elementId = GetGUIStyleElementFromString(elementName);
						}

						if (elementId == aGUI_ELEMENT_UNDEFINED)
						{
							MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
								"CMashGUIManager::LoadGUIStyle", 
								"GUI element '%s' was not found.", 
								elementName.GetCString());
						}				

						if(xmlReader->MoveToFirstChild("attribute"))
						{
							do
							{
								xmlReader->GetAttributeString("name", attributeName);
								eGUI_STYLE_ATTRIBUTE attributeId = GetGUIStyleAttributeFromString(attributeName);

								MashGUISkin *newAttribSkin = newStyle->AddAttribute(elementId, attributeId);
								if (newAttribSkin)
								{
									xmlReader->GetAttributeString("texture", textureName);
									if (!textureName.Empty())
									{
										newAttribSkin->SetTexture(m_pRenderer->GetTexture(textureName));
									}

									xmlReader->GetAttributeFloat("sourcetop", newAttribSkin->baseSource.top);
									xmlReader->GetAttributeFloat("sourceleft", newAttribSkin->baseSource.left);
									xmlReader->GetAttributeFloat("sourcebottom", newAttribSkin->baseSource.bottom);
									xmlReader->GetAttributeFloat("sourceright", newAttribSkin->baseSource.right);
									xmlReader->GetAttributeInt("colour", tempInt);
									newAttribSkin->baseColour.colour = tempInt;
									xmlReader->GetAttributeInt("istransparent", tempBool);
									newAttribSkin->isTransparent = tempBool;
									xmlReader->GetAttributeFloat("alphamaskthreshold", newAttribSkin->alphaMaskThreshold);
									xmlReader->GetAttributeInt("fontcolour", tempInt);
									newAttribSkin->fontColour.colour = tempInt;
									xmlReader->GetAttributeInt("bordercolour", tempInt);
									newAttribSkin->borderColour.colour = tempInt;
									xmlReader->GetAttributeInt("drawborder", tempBool);
									newAttribSkin->renderBoarder = tempBool;
								}
								
							}while(xmlReader->MoveToNextSibling("attribute"));

							xmlReader->PopChild();
						}
					}while(xmlReader->MoveToNextSibling("element"));

					xmlReader->PopChild();
				}

			}while(xmlReader->MoveToNextSibling("style"));
			
			xmlReader->PopChild();
		}

		xmlReader->Destroy();

		return aMASH_OK;
	}

	MashGUIStyle* CMashGUIManager::GetActiveGUIStyle()
	{
		return m_activeStyle;
	}

	eMASH_STATUS CMashGUIManager::SetGUIStyle(const MashStringc &name)
	{
		MashGUIStyle *pStyle = GetGUIStyle(name);

		if (!pStyle)
			return aMASH_FAILED;

		m_activeStyle = pStyle;

		if (m_pRootWindow)
			m_pRootWindow->OnStyleChange(m_activeStyle);

		return aMASH_OK;
	}

	MashGUIStyle* CMashGUIManager::GetGUIStyle(const MashStringc &name)
	{
		std::map<MashStringc, MashGUIStyle*>::const_iterator styleIter = m_styles.find(name);
		if (styleIter != m_styles.end())
			return styleIter->second;

		MashGUIStyle *pStyle = MASH_NEW_COMMON CMashGUIStyle(name);
		m_styles.insert(std::make_pair(name, pStyle));

		if (!GetActiveGUIStyle())
			SetGUIStyle(name);

		return pStyle;
	}
    
    MashGUIView* CMashGUIManager::GetVerticalScrollableView(MashGUIComponent *component)
    {
        if (!component)
            return 0;
        
        MashGUIView *hoverView = component->GetView();
        if (!hoverView)
        {
            if (component->GetParent())
                hoverView = component->GetParent()->GetView();
        }
        
        if (hoverView)
        {
            if (hoverView->IsVerticalScrollInUse())
                return hoverView;
            else
            {
                return GetVerticalScrollableView(component->GetParent());
            }
        }
        
        return 0;
    }

	void CMashGUIManager::OnEvent(const sInputEvent &eventData)
	{
        bool skipMessage = false;
		{
			if (eventData.eventType == sInputEvent::aEVENTTYPE_MOUSE)
			{
				switch(eventData.action)
				{
				case aMOUSEEVENT_AXISX:
				case aMOUSEEVENT_AXISY:
					{
						mash::MashVector2 vMousePos = m_inputManager->GetCursorPosition();

						MashGUIComponent *pMouseHoverComponent = 0;
						if (!m_focusLockStack.Empty())
							pMouseHoverComponent = m_focusLockStack.Back()->GetClosestIntersectingChild(vMousePos, false);
						else
							pMouseHoverComponent = m_pRootWindow->GetClosestIntersectingChild(vMousePos, false);

						if (pMouseHoverComponent && (pMouseHoverComponent != m_pMouseHoverComponent))
						{
							if (m_pMouseHoverComponent)
								m_pMouseHoverComponent->_SetMouseHover(vMousePos, false);

							m_pMouseHoverComponent = pMouseHoverComponent;
							m_pMouseHoverComponent->_SetMouseHover(vMousePos, true);
						}
						else if (!pMouseHoverComponent && m_pMouseHoverComponent)
						{
							m_pMouseHoverComponent->_SetMouseHover(vMousePos, false);
							m_pMouseHoverComponent = 0;
						}

						/*
							Components that have mouse hover but not focus still get
							notified on mouse move events.
							This is needed for components such as menu bars that must
							react to movement when the popups they generate
							gain focus.
						*/
						if (m_pMouseHoverComponent && (m_pMouseHoverComponent != m_pFocus))
							m_pMouseHoverComponent->OnEvent(eventData);

						break;
					}
				case aMOUSEEVENT_B1:
				case aMOUSEEVENT_B2:
					{
						if (eventData.isPressed == 1)
						{
							if (m_pMouseHoverComponent && (m_pMouseHoverComponent != m_pFocus))
							{
								SetFocusedElement(m_pMouseHoverComponent);

								/*
									Send event message to new focused element so it can respond
									immediatly
								*/
								//m_pFocus->OnEvent(eventData);
							}
							else if (!m_pMouseHoverComponent && m_pFocus)
							{
								m_pFocus->_SetHasFocus(false);
								//stop rare recursion from happening
								MashGUIComponent *temp = m_pFocus;
								m_pFocus = 0;
								OnFocusLost(temp);
							}
						}
						
						break;
					}
                    case aMOUSEEVENT_AXISZ:
                    {
                        if (m_pMouseHoverComponent)
                        {
                            MashGUIView *hoverView = GetVerticalScrollableView(m_pMouseHoverComponent);
                            
                            if (hoverView)
                            {
                                if (m_pFocus == hoverView)
                                    skipMessage = true;
                                
                                hoverView->OnEvent(eventData);
                            }
                        }
                        
                        break;
                    }

				};
			}
		}
		
		if (!skipMessage && m_pFocus)
			m_pFocus->OnEvent(eventData);
	}

	void CMashGUIManager::FlushBuffers()
	{
		m_pPrimitiveBatch->Flush();
		m_pRenderBatch->Flush();
		m_pFontBatch->Flush();
		m_pLineBatch->Flush();
	}

	eMASH_STATUS CMashGUIManager::BeginDraw()
	{
		m_sceneCamera = m_pRenderer->GetRenderInfo()->GetCamera();

		if (!m_sceneCamera)
			return aMASH_FAILED;

		m_sceneCamera->Grab();

		m_sceneCamera->Enable2D(true);

		m_beginDrawCalled = true;

		return aMASH_OK;
	}

	void CMashGUIManager::DrawAll()
	{
		if (m_beginDrawCalled)
		{
			m_pRootWindow->Draw();
		}
	}

	void CMashGUIManager::DrawComponent(MashGUIComponent *component)
	{
		if (m_beginDrawCalled)
		{
			component->Draw();
		}
	}

	void CMashGUIManager::OnDestroyElement(MashGUIComponent *component)
	{
		if (m_pMouseHoverComponent == component)
			m_pMouseHoverComponent = 0;

		if (m_pFocus == component)
			m_pFocus = 0;

		MashList<MashGUIComponent*>::Iterator iter = m_focusLockStack.Search(component);
		if (iter != m_focusLockStack.End())
		{
			m_focusLockStack.Erase(iter);
		}
	}

	void CMashGUIManager::ProcessDestroyList()
	{
		if (!m_destroyList.Empty())
		{
			for(uint32 i = 0; i < m_destroyList.Size(); ++i)
			{
				MASH_DELETE m_destroyList[i];
			}

			m_destroyList.Clear();
		}
	}

	void CMashGUIManager::EndDraw()
	{
		if (m_beginDrawCalled)
		{
			FlushBuffers();

			m_sceneCamera->Enable2D(false);
			m_beginDrawCalled = false;

			ProcessDestroyList();

			m_sceneCamera->Drop();
			m_sceneCamera = 0;
		}
	}

	void CMashGUIManager::DrawText(const mash::MashVertexPosTex::sMashVertexPosTex *pVertices,
			uint32 iVertexCount,
			mash::MashTexture *pTexture,
			const mash::sMashColour &fontColour)
	{
		if (m_pFontBatch)
		{
			m_pFontBatch->Draw(pVertices, iVertexCount, pTexture, fontColour);
		}
	}

	void CMashGUIManager::DrawSolidShape(const mash::MashRectangle2 &rect,
			const mash::sMashColour &colour)
	{
		if (m_pPrimitiveBatch)
		{
			m_pPrimitiveBatch->Draw(rect, colour);
		}
	}

	void CMashGUIManager::DrawSolidTriangles(const mash::MashVertexColour::sMashVertexColour *vertices, uint32 vertexCount)
	{
		if (m_pPrimitiveBatch)
		{
			m_pPrimitiveBatch->Draw(vertices, vertexCount);
		}
	}

	void CMashGUIManager::DrawBorder(const mash::MashRectangle2 &rect,
			const mash::sMashColour &colour)
	{
		if (m_pLineBatch)
		{
			m_pLineBatch->Draw(rect, colour);
		}
	}

	void CMashGUIManager::DrawLine(const mash::MashVector2 &top, const mash::MashVector2 &bottom,
			const mash::sMashColour &colour)
	{
		if (m_pLineBatch)
		{
			m_pLineBatch->Draw(top, bottom, colour);
		}
	}

	void CMashGUIManager::DrawSprite(const mash::MashVertexGUI::sMashVertexGUI *pVertices,
			uint32 iVertexCount,
			const MashGUISkin *material,
			const sGUIOverrideTransparency &transparencyOverride)
	{
		if (!material)
			return;

		if (m_pRenderBatch)
		{
			m_pRenderBatch->Draw(pVertices, iVertexCount, material, transparencyOverride);
		}
	}

	void CMashGUIManager::DrawSprite(const mash::MashRectangle2 &rect,
			const mash::MashRectangle2 &clippingRect,
			const MashGUISkin *skin,
			const sGUIOverrideTransparency &transparencyOverride)
	{
		if (!skin)
			return;

		mash::MashRectangle2 finalclippedRect = rect;

		if (finalclippedRect.ClipGUI(clippingRect) != aCULL_CULLED)
		{
			//do scissor test
			mash::MashRectangle2 baseTextCoords = skin->baseSource;
			if (rect.left < clippingRect.left)
			{
				f32 fLerp = (clippingRect.left - rect.left) / (rect.right - rect.left);
				baseTextCoords.left = math::Lerp(baseTextCoords.left, baseTextCoords.right, fLerp);
			}
			if (rect.right > clippingRect.right)
			{
				f32 fLerp = (rect.right - clippingRect.right) / (rect.right - rect.left);
				baseTextCoords.right = math::Lerp(baseTextCoords.right, baseTextCoords.left, fLerp);
			}
			if (rect.top < clippingRect.top)
			{
				f32 fLerp = (clippingRect.top - rect.top) / (rect.bottom - rect.top);
				baseTextCoords.top = math::Lerp(baseTextCoords.top, baseTextCoords.bottom, fLerp);
			}
			if (rect.bottom > clippingRect.bottom)
			{
				f32 fLerp = (rect.bottom - clippingRect.bottom) / (rect.bottom - rect.top);
				baseTextCoords.bottom = math::Lerp(baseTextCoords.bottom, baseTextCoords.top, fLerp);
			}

			mash::MashVertexGUI::sMashVertexGUI pVertices[6] = {
				mash::MashVertexGUI::sMashVertexGUI(mash::MashVector3(finalclippedRect.left, finalclippedRect.bottom, 1.0f)),
				mash::MashVertexGUI::sMashVertexGUI(mash::MashVector3(finalclippedRect.left, finalclippedRect.top, 1.0f)),
				mash::MashVertexGUI::sMashVertexGUI(mash::MashVector3(finalclippedRect.right, finalclippedRect.top, 1.0f)),

				mash::MashVertexGUI::sMashVertexGUI(mash::MashVector3(finalclippedRect.left, finalclippedRect.bottom, 1.0f)),
				mash::MashVertexGUI::sMashVertexGUI(mash::MashVector3(finalclippedRect.right, finalclippedRect.top, 1.0f)),
				mash::MashVertexGUI::sMashVertexGUI(mash::MashVector3(finalclippedRect.right, finalclippedRect.bottom, 1.0f))};

			if (skin->GetTexture())
			{
				uint32 iWidth, iHeight;
				skin->GetTexture()->GetSize(iWidth, iHeight);
				const f32 fTexWidth = 1.0f / (f32)iWidth;
				const f32 fTexHeight = 1.0f / (f32)iHeight;
				pVertices[0].baseTexCoord.x = baseTextCoords.left * fTexWidth;
				pVertices[0].baseTexCoord.y = baseTextCoords.bottom * fTexHeight;
				pVertices[1].baseTexCoord.x = baseTextCoords.left * fTexWidth;
				pVertices[1].baseTexCoord.y = baseTextCoords.top * fTexHeight;
				pVertices[2].baseTexCoord.x = baseTextCoords.right * fTexWidth;
				pVertices[2].baseTexCoord.y = baseTextCoords.top * fTexHeight;

				pVertices[3].baseTexCoord.x = baseTextCoords.left * fTexWidth;
				pVertices[3].baseTexCoord.y = baseTextCoords.bottom * fTexHeight;
				pVertices[4].baseTexCoord.x = baseTextCoords.right * fTexWidth;
				pVertices[4].baseTexCoord.y = baseTextCoords.top * fTexHeight;
				pVertices[5].baseTexCoord.x = baseTextCoords.right * fTexWidth;
				pVertices[5].baseTexCoord.y = baseTextCoords.bottom * fTexHeight;
			}

			sMashColour baseColour = skin->baseColour;
			if (transparencyOverride.enableOverrideTransparency)
				baseColour.SetAlpha(transparencyOverride.alphaValue);

			pVertices[0].colour = baseColour;
			pVertices[1].colour = baseColour;
			pVertices[2].colour = baseColour;
			pVertices[3].colour = baseColour;
			pVertices[4].colour = baseColour;
			pVertices[5].colour = baseColour;

			DrawSprite(pVertices, 6, skin, transparencyOverride);
			
			if (skin->renderBoarder)
			{
				sMashColour boarderColour = skin->borderColour;
				if (transparencyOverride.enableOverrideTransparency)
					boarderColour.SetAlpha(transparencyOverride.alphaValue);

				DrawBorder(finalclippedRect, boarderColour);
			}
		}
	}

	MashGUIFactory* CMashGUIManager::_GetGUIFactory()const
	{
		return m_guiFactory;
	}

	MashGUIViewport* CMashGUIFactory::CreateViewport(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement)
	{
		return MASH_NEW_COMMON CMashGUIViewport(m_guiManager, m_inputManager, pParent, destRegion, m_renderer);
	}

	MashGUISprite* CMashGUIFactory::CreateSprite(const MashGUIRect &destRegion, MashGUIComponent *parent, int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_SPRITE;

		return MASH_NEW_COMMON CMashGUISprite(m_guiManager, m_inputManager, parent, destRegion, styleElement);
	}

	MashGUIButton* CMashGUIFactory::CreateButton(const MashGUIRect &destRegion, MashGUIComponent *parent, int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_BUTTON;

		return MASH_NEW_COMMON CMashGUIButton(m_guiManager, m_inputManager, parent, destRegion, styleElement);
	}

	MashGUIWindow* CMashGUIFactory::CreateWindowView(const MashGUIRect &destRegion, MashGUIComponent *parent, int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_WINDOW;

		return MASH_NEW_COMMON CMashGUIWindow(m_guiManager, m_inputManager, parent, destRegion, styleElement);
	}

	MashGUICheckBox* CMashGUIFactory::CreateCheckBox(const MashGUIRect &destRegion, MashGUIComponent *parent, int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_CHECKBOX;

		return MASH_NEW_COMMON CMashGUICheckBox(m_guiManager, m_inputManager, parent, destRegion, styleElement);
	}

	MashGUIScrollBar* CMashGUIFactory::CreateScrollBar(const MashGUIRect &destRegion, bool isVertical, f32 incrementAmount, MashGUIView *parent, int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_SCROLLBAR;

		return MASH_NEW_COMMON CMashGUIScrollBar(m_guiManager, m_inputManager, parent, destRegion, incrementAmount, isVertical, styleElement);
	}

	MashGUIScrollbarView* CMashGUIFactory::CreateScrollBarView(/*const MashGUIRect &destRegion, */bool isVertical, MashGUIComponent *pParent, int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_SCROLLBAR;

		return MASH_NEW_COMMON CMashGUIScrollbarView(m_guiManager, m_inputManager, pParent/*, destRegion*/, isVertical, styleElement);
	}

	MashGUITabControl* CMashGUIFactory::CreateTabControl(const MashGUIRect &destRegion, MashGUIComponent *parent, int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_TAB;

		return MASH_NEW_COMMON CMashGUITabControl(m_guiManager, m_inputManager, parent, destRegion, styleElement);
	}

	MashGUITree* CMashGUIFactory::CreateTree(const MashGUIRect &destRegion, MashGUIComponent *parent, int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_TREE;

		return MASH_NEW_COMMON CMashGUITree(m_guiManager, m_inputManager, parent, destRegion, styleElement);
	}

	MashGUIStaticText* CMashGUIFactory::CreateStaticText(const MashGUIRect &destRegion, MashGUIComponent *parent, int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_STATIC_TEXTBOX;

		return MASH_NEW_COMMON CMashGUIStaticText(m_guiManager, m_inputManager, parent, destRegion, styleElement);
	}

	MashGUIListBox* CMashGUIFactory::CreateListBox(const MashGUIRect &destRegion, MashGUIComponent *parent, int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_LISTBOX;

		return MASH_NEW_COMMON CMashGUIListBox(m_guiManager, m_inputManager, parent, destRegion, styleElement);
	}

	MashGUIPopupMenu* CMashGUIFactory::CreatePopupMenu(int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_POPUP;

		return MASH_NEW_COMMON CMashGUIPopupMenu(m_guiManager, m_inputManager, 0, styleElement);
	}

	MashGUIMenuBar* CMashGUIFactory::CreateMenuBar(MashGUIComponent *parent, const MashGUIUnit &left, const MashGUIUnit &right, int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_MENUBAR;

		//make the menu bar a little bigger than the text
		f32 menuBarHeight = 10.0f;
		const MashGUIStyle *style = m_guiManager->GetActiveGUIStyle();
		if (style)
			menuBarHeight += style->GetFont()->GetMaxCharacterHeight();

		MashGUIRect itemRect(left, 
			mash::MashGUIUnit(0.0f, 0.0f),
			right,
			mash::MashGUIUnit(0.0f, menuBarHeight));

		return MASH_NEW_COMMON CMashGUIMenuBar(m_guiManager, m_inputManager, parent, itemRect, menuBarHeight, styleElement);
	}

	MashGUITextBox* CMashGUIFactory::CreateTextBox(const MashGUIRect &destRegion, MashGUIComponent *parent, int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_TEXTBOX;

		return MASH_NEW_COMMON CMashGUITextBox(m_guiManager, m_inputManager, parent, destRegion, styleElement);
	}

	MashGUIView* CMashGUIFactory::CreateView(const MashGUIRect &destRegion, MashGUIComponent *parent, int32 styleElement)
	{
		if (styleElement == -1)
			styleElement = aGUI_ELEMENT_WINDOW;

		return MASH_NEW_COMMON CMashGUIView(m_guiManager, m_inputManager, parent, destRegion, styleElement);
	}

	MashGUIOpenFileDialog* CMashGUIFactory::CreateOpenFileDialog(const MashGUIRect &destRegion)
	{
		//uses window elements
		return MASH_NEW_COMMON CMashGUIOpenFileDialog(m_guiManager, destRegion, aGUI_ELEMENT_FILE_DIALOG);
	}

	MashGUIStaticText* CMashGUIFactory::CreateDebugLog(const MashGUIRect &destRegion, MashGUIComponent *parent, uint32 logFlags, uint32 maxMessageCount)
	{
		//uses static text elements
		return MASH_NEW_COMMON CMashGUIDebugLog(m_guiManager, m_inputManager, parent, destRegion, aGUI_ELEMENT_STATIC_TEXTBOX, logFlags, maxMessageCount);
	}
}
