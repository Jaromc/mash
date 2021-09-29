//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#include "MashInclude.h"

#include "MemoryAllocator/MashDefaultMemoryAllocator.h"
#include "D3D10/MashD3D10Creation.h"
#include "OpenGL3/MashOpenGL3Creation.h"

#include "GUI/MashGUIManager.h"
#include "MashHelper.h"
#include <sstream>

#if defined (MASH_WINDOWS) && !defined(__MINGW32__)
    #define USE_DIRECTX
#endif

using namespace mash;

static const char *const g_triangleColliderTypeName[] = {
		"Standard",
		"KD Tree",
		0};

static const char *const g_sceneViewerXMLFileName = "_SceneViewer";

class MainLoop : public MashGameLoop
{
private:

	enum eFILE_DIALOG_USAGE
	{
		DIALOG_USAGE_LOAD_MATERIAL,
		DIALOG_USAGE_LOAD_MATERIAL_ALL,
		DIALOG_USAGE_LOAD_ANIM_RANGE,
		DIALOG_USAGE_LOAD_PARTICLE_MESH,
		DIALOG_USAGE_LOAD_PARTICLE_TEXTURE,
		DIALOG_USAGE_SAVE_STANDARD_MATERIALS,
        DIALOG_USAGE_RELOAD_MATERIAL,
		DIALOG_USAGE_LOAD_SHADOWS,
		DIALOG_USAGE_SAVE_SHADOWS,
		DIALOG_USAGE_NONE
	};

	enum eANIM_UPDATE
	{
		ANIM_UPDATE_NONE = 0,
		ANIM_UPDATE_NAME_ONLY = 1,
		ANIM_UPDATE_ALL = 0xFFFFFFFF,
	};

	enum eAPPLY_MATERIAL_TYPE
	{
		APPLY_MATERIAL_SINGLE,
		APPLY_MATERIAL_ALL
	};

	enum eCONFIRM_TYPE
	{
		CONFIRM_DELETE_NODE
	};

	struct sEntityData
	{
		unsigned int selectedLod;
		unsigned int selectedMesh;
	};

	struct sAnimRange
	{
		MashStringc name;
		unsigned int start;
		unsigned int end;

		sAnimRange():name(""), start(0xFFFFFFFF), end(0){}
	};

	struct sSceneNodeData
	{
		MashAnimationBuffer *originalAnimBuffer;
		int treeId;
		/*
			Keep track or orientation in euler angles because conversion
			from quaternions can be troublesome
		*/
		MashVector3 euler;

		struct sParticleData
		{
			char *diffuseTextureName;
			char *meshName;
		};

		union
		{
			sParticleData particleData;
		};

		sSceneNodeData():originalAnimBuffer(0), treeId(0), euler(0.0f, 0.0f, 0.0f){}
	};

	struct sSceneInfo
	{
		unsigned int nodeCount;
		unsigned int lightCount;
		unsigned int forwardLightCount;
		unsigned int materialChangesPerRender;

		sSceneInfo():nodeCount(0), lightCount(0), forwardLightCount(0), materialChangesPerRender(0){}
	};
    
    struct sMaterial
    {
        MashMaterial *material;
        MashStringc filePath;
        MashStringc fileName;
    };

	sSceneInfo m_sceneInfo;

	std::map<int, sAnimRange> m_selectedNodeAnimRanges;
	//holds all the child node buffers for the selected node + the selected node
	MashArray<MashSceneNode*> m_selectedNodeAnimBuffers;
	eANIM_UPDATE m_selectedNodeAnimRangeUpdate;
	eAPPLY_MATERIAL_TYPE m_applyMaterialType;

	std::map<MashSceneNode*, sSceneNodeData> m_loadedNodeData;
	MashList<MashLight*> m_loadedLights;//kept for debugging;

	MashDevice *m_device;
	bool m_quit;
	eFILE_DIALOG_USAGE m_fileDialogUsage;
	eCONFIRM_TYPE m_confirmDialogUsage;
	MashGUIView *m_viewerRoot;
	MashGUIWindow *m_loadSettingsWindow;
	MashGUIWindow *m_materialSelectWindow;
	MashGUIWindow *m_animationRangeWindow;
	MashGUIWindow *m_triColliderWindow;
	MashGUIWindow *m_particleCreationWindow;
	MashGUIWindow *m_saveSettingsWindow;
	MashGUIWindow *m_changeParentWindow;
	MashGUIWindow *m_preferredLightingWindow;
	MashGUIWindow *m_shadowCasterWindow;
    MashGUIWindow *m_sceneTreeWindow;
	//MashGUIView *m_mainSceneView;
	MashGUIViewport *m_viewportElement;

	MashGUIOpenFileDialog *m_openFileDialog;
	MashSceneNode *m_selectedNode;
	MashSceneNode *m_sceneRoot;

	MashGUIWindow *m_debugLogWindow;
    MashFreeMovementController *m_freeCameraController;

	sEntityData m_entityData;

	MashCamera *m_defaultCamera;
	MashLight *m_defaultLight;

	//for node animation
	int32 m_playingAnimtionIndex;
	MashTexture *m_playIcon;
	MashTexture *m_errorIcons;
	MashGUIStaticText *m_fpsText;
	MashGUIStaticText *m_debugText;
	int32 m_lastSelectedNodeCycle;
	uint32 m_logReceiverID;

	std::map<MashStringc, sMaterial> m_editorMaterials;
public:
	MainLoop(MashDevice *device):m_device(device),m_quit(false), m_selectedNode(0), m_selectedNodeAnimRangeUpdate(ANIM_UPDATE_NONE), m_playIcon(0), m_playingAnimtionIndex(-1),
		m_openFileDialog(0), m_errorIcons(0), m_freeCameraController(0), m_lastSelectedNodeCycle(0), m_logReceiverID(0){}

	virtual ~MainLoop()
	{
	#ifdef MASH_LOG_ENABLED
		MashLog::Instance()->RemoveReceiver(m_logReceiverID);
	#endif

		if (m_openFileDialog)
		{
			m_openFileDialog->Drop();
			m_openFileDialog = 0;
		}

		std::map<MashSceneNode*, sSceneNodeData>::iterator iter = m_loadedNodeData.begin();
		std::map<MashSceneNode*, sSceneNodeData>::iterator iterEnd = m_loadedNodeData.end();
		for(; iter != iterEnd; ++iter)
		{
			if (iter->second.originalAnimBuffer)
				iter->second.originalAnimBuffer->Drop();
		}

		m_loadedNodeData.clear();
	}

	bool Initialise()
	{
		int playerControl = m_device->GetInputManager()->CreatePlayer();
		const MashStringc inputContext = "play";
		m_device->GetInputManager()->CreateDefaultActionMap(playerControl,inputContext);
		m_device->GetInputManager()->SetCurrentPlayerContext(playerControl, inputContext);
		m_device->GetInputManager()->AssignPlayerToController(playerControl, 0);//keyboard/mouse

		MashArray<eINPUT_EVENT> actions;
		actions.PushBack(aKEYEVENT_DELETE);
		m_device->GetInputManager()->SetPlayerActionMap(playerControl, inputContext, aINPUTMAP_USER_REGION, actions, MashInputEventFunctor(&MainLoop::OnDelete, this));

		actions.Clear();
		actions.PushBack(aKEYEVENT_LEFTBRACKET);
		actions.PushBack(aKEYEVENT_RIGHTBRACKET);
		m_device->GetInputManager()->SetPlayerActionMap(playerControl, inputContext, aINPUTMAP_USER_REGION+1, actions, MashInputEventFunctor(&MainLoop::OnCameraSpeedChange, this));

		//det up debug gui
		MashGUIRect fpsRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 30.0f),
			MashGUIUnit(0.0f, 100.0f), MashGUIUnit(0.0f, 50.0f));
		m_fpsText = m_device->GetGUIManager()->AddStaticText(fpsRegion, 0);
		m_fpsText->SetRenderEnabled(false);

		MashGUIRect debugTextRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 50.0f),
			MashGUIUnit(0.0f, 400.0f), MashGUIUnit(0.0f, 200.0f));
		m_debugText = m_device->GetGUIManager()->AddStaticText(debugTextRegion, 0);
		m_debugText->SetRenderEnabled(false);

		m_sceneRoot = m_device->GetSceneManager()->AddDummy(0, "SceneRoot");
		FillNodeData(m_sceneRoot);
		//m_loadedNodeData.insert(std::make_pair(m_sceneRoot, sSceneNodeData()));

		m_defaultCamera = (MashCamera*)m_device->GetSceneManager()->AddCamera(m_sceneRoot, "Camera01");
        m_freeCameraController = m_device->GetSceneManager()->CreateFreeMovementController(playerControl, inputContext, 0.1f, 300.0f);
        m_defaultCamera->AddCallback(m_freeCameraController);
        m_freeCameraController->Drop();
        
		m_defaultCamera->SetZFar(1000);
		m_defaultCamera->SetZNear(1.0f);
		m_defaultCamera->SetPosition(MashVector3(0, 0, -100));
		m_freeCameraController->SetInputState(false);
		FillNodeData(m_defaultCamera);
		//m_loadedNodeData.insert(std::make_pair(m_defaultCamera, sSceneNodeData()));

		m_defaultLight = (MashLight*)m_device->GetSceneManager()->AddLight(m_sceneRoot, "Light01", aLIGHT_DIRECTIONAL, aLIGHT_RENDERER_FORWARD, true);
		m_defaultLight->SetLookAtDirection(MashVector3(-1,0,0), true);
		m_defaultLight->SetDiffuse(sMashColour4(1.0f, 1.0f, 1.0f,0.0f));
		m_defaultLight->SetAmbient(sMashColour4(0.2f, 0.2f, 0.2f, 0.0f));
		m_defaultLight->SetSpecular(sMashColour4(1.0f, 1.0f, 1.0f,0.0f));
		m_defaultLight->SetRange(1000);
		m_defaultLight->SetShadowsEnabled(false);
		FillNodeData(m_defaultLight);
		//m_loadedNodeData.insert(std::make_pair(m_defaultLight, sSceneNodeData()));

		m_playIcon = m_device->GetRenderer()->GetTexture("PlayIcon.png");
		m_errorIcons = m_device->GetRenderer()->GetTexture("ErrorIcons.png");

		//load default material into user selectable materials
        sMaterial newMaterial;
        newMaterial.material = m_device->GetRenderer()->GetMaterialManager()->GetStandardMaterial(MashMaterialManager::aSTANDARD_MATERIAL_DEFAULT_MESH);
        
		m_editorMaterials[newMaterial.material->GetMaterialName()] = newMaterial;

		MashGUIRect mainEditorViewRect(MashGUIUnit(0.0f,0.0f), MashGUIUnit(0.0f,0.0f),
			MashGUIUnit(1.0f, 0.0f), MashGUIUnit(1.0f, 0.0f));
		m_viewerRoot = m_device->GetGUIManager()->AddView(mainEditorViewRect, 0);
		m_viewerRoot->SetRenderBackgroundState(false);
		m_viewerRoot->SetHorizontalScrollState(false);
		m_viewerRoot->SetVerticalScrollState(false);

		m_device->GetGUIManager()->LoadGUILayout("./SceneViewerGUILayout.xml", m_viewerRoot);

		MashGUIView *elementPropertiesView = (MashGUIView*)m_viewerRoot->GetElementByName("Properties");
        
        m_device->GetGUIManager()->LoadGUILayout("./SceneTreeGUILayout.xml", m_viewerRoot);
        m_sceneTreeWindow = (MashGUIWindow*)m_viewerRoot->GetElementByName("TreeWindow");
        m_sceneTreeWindow->SetCloseButtonEvent(MashGUIWindow::aCLOSE_AND_HIDE);
		m_sceneTreeWindow->GetElementByName("SceneTree")->RegisterReceiver(aGUIEVENT_TREE_SELECTION_CHANGE, MashGUIEventFunctor(&MainLoop::OnSceneTreeSelection, this));
        m_sceneTreeWindow->SetMinimizeState(true);
        m_sceneTreeWindow->AddPosition(400.0, 30.0);

		MashGUIMenuBar *menuBar = (MashGUIMenuBar*)m_viewerRoot->GetElementByName("MenuBar");
		menuBar->RegisterReceiver(aGUIEVENT_MENUBAR_SELECTION, MashGUIEventFunctor(&MainLoop::OnMenuBarSelection, this));
		
		MashGUIRect dialogDest(MashGUIUnit(), MashGUIUnit(), MashGUIUnit(0.0f, 500), MashGUIUnit(0.0f, 400));
		m_openFileDialog = m_device->GetGUIManager()->CreateOpenFileDialog(dialogDest);
		m_openFileDialog->CloseDialog();
		m_openFileDialog->RegisterReceiver(aGUIEVENT_OPENFILE_SELECTED, MashGUIEventFunctor(&MainLoop::OnFileDialogSelection, this));

		m_debugLogWindow = m_device->GetGUIManager()->AddDebugLogWindow(dialogDest, MashLog::aERROR_LEVEL_WARNING | MashLog::aERROR_LEVEL_ERROR, 20);
		m_debugLogWindow->SetRenderEnabled(false);

		#ifdef MASH_LOG_ENABLED
			m_logReceiverID = MashLog::Instance()->AddReceiver(MashLogEventFunctor(&MainLoop::OnLogMessage, this));
			MashGUIListBox *logMessageLB = (MashGUIListBox*)m_viewerRoot->GetElementByName("LogMessageLB");
			logMessageLB->RegisterReceiver(aGUIEVENT_LB_SELECTION_CONFIRMED, MashGUIEventFunctor(&MainLoop::OnNewLogMessageSelected, this));
			logMessageLB->EnableIcons(true, true);
		#endif

		m_device->GetGUIManager()->LoadGUILayout("./ShadowCasterGUILayout.xml", m_viewerRoot);
		m_shadowCasterWindow = (MashGUIWindow*)m_viewerRoot->GetElementByName("ShadowCasterSettingsWindow");
		m_shadowCasterWindow->GetElementByName("ShadowCasterLightTypeLB")->RegisterReceiver(aGUIEVENT_LB_SELECTION_CHANGE, MashGUIEventFunctor(&MainLoop::OnChangeShadowCasterLightType, this));
		m_shadowCasterWindow->GetElementByName("ShadowCasterCasterUpdateBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnUpdateShadowCaster, this));
		m_shadowCasterWindow->GetElementByName("ShadowCasterCasterResetBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnResetShadowCaster, this));
		((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("ShadowCasterLightTypeLB",false))->SetActiveItemByUserValue(aLIGHT_DIRECTIONAL, false);
		m_shadowCasterWindow->SetCloseButtonEvent(MashGUIWindow::aCLOSE_AND_HIDE);
		m_shadowCasterWindow->SetAlwaysOnTop(true);
		m_shadowCasterWindow->SetRenderEnabled(false);

		m_device->GetGUIManager()->LoadGUILayout("./PreferredLightingGUILayout.xml", m_viewerRoot);
		m_preferredLightingWindow = (MashGUIWindow*)m_viewerRoot->GetElementByName("PreferredLightingWindow");
		m_preferredLightingWindow->GetElementByName("PreferredLightingSelectBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnChangePreferredLightingSelect, this));
		m_preferredLightingWindow->GetElementByName("PreferredLightingCancelBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnChangePreferredLightingCancel, this));
		m_preferredLightingWindow->SetRenderEnabled(false);

		m_device->GetGUIManager()->LoadGUILayout("./ChangeParentGUILayout.xml", m_viewerRoot);
		m_changeParentWindow = (MashGUIWindow*)m_viewerRoot->GetElementByName("ChangeParentWindow");
		m_changeParentWindow->GetElementByName("ChangeParentSelectBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnChangeParentSelect, this));
		m_changeParentWindow->GetElementByName("ChangeParentCancelBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnChangeParentCancel, this));
		m_changeParentWindow->SetRenderEnabled(false);

		m_device->GetGUIManager()->LoadGUILayout("./LoadSettingsGUILayout.xml", m_viewerRoot);
		m_loadSettingsWindow = (MashGUIWindow*)m_viewerRoot->GetElementByName("LoadSettingsWindow");
		m_loadSettingsWindow->GetElementByName("LoadButton")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLoad, this));
		m_loadSettingsWindow->GetElementByName("CancelButton")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnCancelLoad, this));
		m_loadSettingsWindow->GetElementByName("FilePathButton")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnSetLoadFilePath, this));
		m_loadSettingsWindow->SetRenderEnabled(false);

		m_device->GetGUIManager()->LoadGUILayout("./SaveSettingsGUILayout.xml", m_viewerRoot);
		m_saveSettingsWindow = (MashGUIWindow*)m_viewerRoot->GetElementByName("SaveSettingsWindow");
		m_saveSettingsWindow->GetElementByName("SaveBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnSave, this));
		m_saveSettingsWindow->GetElementByName("SaveCancelBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnSaveCancel, this));
		m_saveSettingsWindow->GetElementByName("SaveFilePathBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnSaveSetLoadFilePath, this));
		m_saveSettingsWindow->SetRenderEnabled(false);

		m_device->GetGUIManager()->LoadGUILayout("./MaterialSelectGUILayout.xml", m_viewerRoot);
		m_materialSelectWindow = (MashGUIWindow*)m_viewerRoot->GetElementByName("MaterialWindow");
		m_materialSelectWindow->GetElementByName("MaterialLoadBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnMaterialLoad, this));
		m_materialSelectWindow->GetElementByName("MaterialLoadAllBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnMaterialLoadAll, this));
		m_materialSelectWindow->GetElementByName("MaterialSelectBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnMaterialSelected, this));
		m_materialSelectWindow->GetElementByName("MaterialCancelBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnMaterialSelectCancelled, this));
        m_materialSelectWindow->GetElementByName("MaterialReloadBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnMaterialReload, this));
		m_materialSelectWindow->SetRenderEnabled(false);

		m_device->GetGUIManager()->LoadGUILayout("./AnimationRangeGUILayout.xml", m_viewerRoot);
		m_animationRangeWindow = (MashGUIWindow*)m_viewerRoot->GetElementByName("AnimRangeWindow");
		m_animationRangeWindow->GetElementByName("AnimRangeInsertBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnAnimRangeInsert, this));
		m_animationRangeWindow->GetElementByName("AnimRangeRemoveBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnAnimRangeRemove, this));
		m_animationRangeWindow->GetElementByName("AnimRangeUpdateBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnAnimRangeUpdate, this));

		m_animationRangeWindow->GetElementByName("AnimRangeCancelBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnAnimRangeCancelled, this));
		m_animationRangeWindow->GetElementByName("AnimRangeLoadBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnAnimRangeLoad, this));
		m_animationRangeWindow->GetElementByName("AnimRangeSaveBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnAnimRangeSave, this));
		m_animationRangeWindow->GetElementByName("AnimRangeLB")->RegisterReceiver(aGUIEVENT_LB_SELECTION_CHANGE, MashGUIEventFunctor(&MainLoop::OnAnimRangeSelected, this));
		m_animationRangeWindow->GetElementByName("AnimRangeStartFrameTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnAnimRangeStartFrame, this));
		m_animationRangeWindow->GetElementByName("AnimRangeEndFrameTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnAnimRangeEndFrame, this));
		m_animationRangeWindow->GetElementByName("AnimRangeNameTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnAnimRangeName, this));
		m_animationRangeWindow->SetRenderEnabled(false);

		m_device->GetGUIManager()->LoadGUILayout("./NodeCommonGUILayout.xml", elementPropertiesView);
		MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
		{
			MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");
			lb->EnableIcons(true, true);
			lb->RegisterReceiver(aGUIEVENT_LB_SELECTION_CHANGE, MashGUIEventFunctor(&MainLoop::OnNodeAnimChange, this));
		}
		windowElement->GetElementByName("NodeIDTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeIDChange, this));
		windowElement->GetElementByName("HasAnimMixerCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnToggleAnimMixer, this));
		windowElement->GetElementByName("HasAnimMixerCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnToggleAnimMixer, this));
		windowElement->GetElementByName("NodePlayAnimBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodePlayAnimation, this));
		windowElement->GetElementByName("NodeStopAnimBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeStopAnimation, this));
		windowElement->GetElementByName("NodeEditAnimationBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnEditNodeAnimation, this));
		windowElement->GetElementByName("NodeScaleZTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeScaleZ, this));
		windowElement->GetElementByName("NodeScaleYTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeScaleY, this));
		windowElement->GetElementByName("NodeScaleXTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeScaleX, this));
		windowElement->GetElementByName("NodeRotZTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeRotZ, this));
		windowElement->GetElementByName("NodeRotYTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeRotY, this));
		windowElement->GetElementByName("NodeRotXTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeRotX, this));
		windowElement->GetElementByName("NodePosZTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodePosZ, this));
		windowElement->GetElementByName("NodePosYTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodePosY, this));
		windowElement->GetElementByName("NodePosXTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodePosX, this));
		windowElement->GetElementByName("NodeNameTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeName, this));
		windowElement->GetElementByName("NodeChangeParentBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeChangeParent, this));

		//Disable user input for weight. Let the editor set it automatically.
		//windowElement->GetElementByName("NodeAnimWeightTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeAnimWeight, this));
		windowElement->GetElementByName("NodeAnimWeightTB")->SetRenderEnabled(false);
		windowElement->GetElementByName("NodeAnimWeightST")->SetRenderEnabled(false);
		windowElement->GetElementByName("NodeAnimTrackTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeAnimTrack, this));
		windowElement->GetElementByName("NodeAnimFPSTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeAnimFPS, this));
		windowElement->GetElementByName("NodeAnimFrameTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeAnimFrame, this));
		windowElement->GetElementByName("NodeAnimSpeedTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeAnimSpeed, this));
		windowElement->GetElementByName("NodeAnimTransitionTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeAnimTransitionTime, this));
		windowElement->GetElementByName("NodeAnimTransitionBtn")->RegisterReceiver(aGUIEVENT_BTN_DOWN, MashGUIEventFunctor(&MainLoop::OnNodeAnimTransition, this));
		windowElement->GetElementByName("NodeAnimTransitionBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnNodeAnimTransition, this));
		windowElement->GetElementByName("NodeAnimTransitionBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CANCEL, MashGUIEventFunctor(&MainLoop::OnNodeAnimTransition, this));
		windowElement->GetElementByName("NodeAnimWrapLB")->RegisterReceiver(aGUIEVENT_LB_SELECTION_CHANGE, MashGUIEventFunctor(&MainLoop::OnNodeAnimWrapMode, this));
		windowElement->GetElementByName("NodeAnimBlendLB")->RegisterReceiver(aGUIEVENT_LB_SELECTION_CHANGE, MashGUIEventFunctor(&MainLoop::OnNodeAnimBlendMode, this));
		windowElement->SetRenderEnabled(false);

		m_device->GetGUIManager()->LoadGUILayout("./ParticleSettingsGUILayout.xml", elementPropertiesView);
		windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("ParticleSettingsWindow");
		windowElement->GetElementByName("ParticleStartTimeTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleStartTime, this));
		windowElement->GetElementByName("ParticleIsPlayingCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnParticleIsPlaying, this));
		windowElement->GetElementByName("ParticleIsPlayingCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnParticleIsPlaying, this));
		windowElement->GetElementByName("ParticleSoftScaleTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleSoftScale, this));
		windowElement->GetElementByName("ParticleMinStartColATB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinStartColA, this));
		windowElement->GetElementByName("ParticleMinStartColBTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinStartColB, this));
		windowElement->GetElementByName("ParticleMinStartColGTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinStartColG, this));
		windowElement->GetElementByName("ParticleMinStartColRTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinStartColR, this));
		windowElement->GetElementByName("ParticleMaxStartColATB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxStartColA, this));
		windowElement->GetElementByName("ParticleMaxStartColBTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxStartColB, this));
		windowElement->GetElementByName("ParticleMaxStartColGTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxStartColG, this));
		windowElement->GetElementByName("ParticleMaxStartColRTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxStartColR, this));
		windowElement->GetElementByName("ParticleMinEndColATB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinEndColA, this));
		windowElement->GetElementByName("ParticleMinEndColBTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinEndColB, this));
		windowElement->GetElementByName("ParticleMinEndColGTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinEndColG, this));
		windowElement->GetElementByName("ParticleMinEndColRTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinEndColR, this));
		windowElement->GetElementByName("ParticleMaxEndColATB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxEndColA, this));
		windowElement->GetElementByName("ParticleMaxEndColBTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxEndColB, this));
		windowElement->GetElementByName("ParticleMaxEndColGTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxEndColG, this));
		windowElement->GetElementByName("ParticleMaxEndColRTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxEndColR, this));
		windowElement->GetElementByName("ParticleMinVelocityZTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinVelZ, this));
		windowElement->GetElementByName("ParticleMinVelocityYTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinVelY, this));
		windowElement->GetElementByName("ParticleMinVelocityXTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinVelX, this));
		windowElement->GetElementByName("ParticleMaxVelocityZTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxVelZ, this));
		windowElement->GetElementByName("ParticleMaxVelocityYTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxVelY, this));
		windowElement->GetElementByName("ParticleMaxVelocityXTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxVelX, this));
		windowElement->GetElementByName("ParticleMaxCountTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxCount, this));
		windowElement->GetElementByName("ParticlePerSecondTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticlePerSecond, this));
		windowElement->GetElementByName("ParticleMinStartSizeTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinStartSize, this));
		windowElement->GetElementByName("ParticleMaxStartSizeTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxStartSize, this));
		windowElement->GetElementByName("ParticleMinEndSizeTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinEndSize, this));
		windowElement->GetElementByName("ParticleMaxEndSizeTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxEndSize, this));
		windowElement->GetElementByName("ParticleMinRotateTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinRotate, this));
		windowElement->GetElementByName("ParticleMaxRotateTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxRotate, this));
		windowElement->GetElementByName("ParticleMinDurationTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMinDuration, this));
		windowElement->GetElementByName("ParticleMaxDurationTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMaxDuration, this));

		windowElement->GetElementByName("ParticleGravityXTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleGravityX, this));
		windowElement->GetElementByName("ParticleGravityYTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleGravityY, this));
		windowElement->GetElementByName("ParticleGravityZTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleGravityZ, this));
		windowElement->GetElementByName("ParticleTextureBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleTextureLoad, this));
		windowElement->GetElementByName("ParticleMeshBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleMeshLoad, this));
		((MashGUITextBox*)windowElement->GetElementByName("ParticleMeshTB"))->SetEventsEnabled(false);
		((MashGUITextBox*)windowElement->GetElementByName("ParticleTextureTB"))->SetEventsEnabled(false);
		MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("ParticleTextureWrapModeLB");
		lb->RegisterReceiver(aGUIEVENT_LB_SELECTION_CHANGE, MashGUIEventFunctor(&MainLoop::OnParticleWrapMode, this));
		lb->SetMute(true);
		lb->SetActiveItem(0);
		lb->SetMute(false);
		windowElement->SetRenderEnabled(false);
		//this window starts from the bottom of the node common window
		windowElement->AddPosition(0.0f, elementPropertiesView->GetElementByName("NodeCommonWindow")->GetDestinationRegion().bottom.offset);

		m_device->GetGUIManager()->LoadGUILayout("./LightSettingsGUILayout.xml", elementPropertiesView);
		windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("LightSettingsWindow");
		windowElement->GetElementByName("LightAtten0TB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightAtten0, this));
		windowElement->GetElementByName("LightAtten1TB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightAtten1, this));
		windowElement->GetElementByName("LightAtten2TB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightAtten2, this));
		windowElement->GetElementByName("LightIsEnabledCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnLightIsEnabled, this));
		windowElement->GetElementByName("LightIsEnabledCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnLightIsEnabled, this));
		windowElement->GetElementByName("LightIsMainCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnLightIsMain, this));
		windowElement->GetElementByName("LightIsMainCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnLightIsMain, this));
		windowElement->GetElementByName("LightDiffuseRTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightDiffuseR, this));
		windowElement->GetElementByName("LightDiffuseGTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightDiffuseG, this));
		windowElement->GetElementByName("LightDiffuseBTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightDiffuseB, this));
		windowElement->GetElementByName("LightAmbientRTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightAmbientR, this));
		windowElement->GetElementByName("LightAmbientGTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightAmbientG, this));
		windowElement->GetElementByName("LightAmbientBTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightAmbientB, this));
		windowElement->GetElementByName("LightSpecularRTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightSpecularR, this));
		windowElement->GetElementByName("LightSpecularGTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightSpecularG, this));
		windowElement->GetElementByName("LightSpecularBTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightSpecularB, this));
		windowElement->GetElementByName("LightRangeTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightRange, this));
		windowElement->GetElementByName("LightFalloffTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightFalloff, this));
		windowElement->GetElementByName("LightInnerConeTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightInnerCone, this));
		windowElement->GetElementByName("LightOuterConeTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnLightOuterCone, this));
		windowElement->GetElementByName("ShadowsEnabledCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnLightShadows, this));
		windowElement->GetElementByName("ShadowsEnabledCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnLightShadows, this));
		windowElement->GetElementByName("ForwardRenderedLightCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnForwardRenderedLight, this));
		windowElement->GetElementByName("ForwardRenderedLightCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnForwardRenderedLight, this));
		windowElement->GetElementByName("LightTypeLB")->RegisterReceiver(aGUIEVENT_LB_SELECTION_CHANGE, MashGUIEventFunctor(&MainLoop::OnLightType, this));
		windowElement->SetRenderEnabled(false);
        
        //this window starts from the bottom of the node common window
		windowElement->AddPosition(0.0f, elementPropertiesView->GetElementByName("NodeCommonWindow")->GetDestinationRegion().bottom.offset);

		m_device->GetGUIManager()->LoadGUILayout("./CameraSettingsGUILayout.xml", elementPropertiesView);
		windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("CameraSettingsWindow");
		windowElement->GetElementByName("CameraIsMainCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnCameraIsMain, this));
		windowElement->GetElementByName("CameraIsMainCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnCameraIsMain, this));
		windowElement->GetElementByName("CameraZFarTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnCameraZFar, this));
		windowElement->GetElementByName("CameraZNearTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnCameraZNear, this));
		windowElement->GetElementByName("CameraAspectTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnCameraAspect, this));
		windowElement->GetElementByName("CameraFOVTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnCameraFOV, this));
		windowElement->GetElementByName("CameraAutoAspectCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnCameraAutoAspect, this));
		windowElement->GetElementByName("CameraAutoAspectCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnCameraAutoAspect, this));
		windowElement->SetRenderEnabled(false);
		//this window starts from the bottom of the node common window
		windowElement->AddPosition(0.0f, elementPropertiesView->GetElementByName("NodeCommonWindow")->GetDestinationRegion().bottom.offset);

		m_device->GetGUIManager()->LoadGUILayout("./EntitySettingsGUILayout.xml", elementPropertiesView);
		windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("EntitySettingsWindow");
		windowElement->GetElementByName("EntityTriBufferRemoveBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnEntityTriColliderRemove, this));
		windowElement->GetElementByName("EntityTriBufferModifyBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnEntityTriColliderModify, this));
		windowElement->GetElementByName("EntityLodDistanceTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnEntityLodDistance, this));
		//windowElement->GetElementByName("MaterialNameTB")->RegisterReceiver(aGUIEVENT_TB_CONFIRM, MashGUIEventFunctor(&MainLoop::OnEntityMaterialName, this));
		windowElement->GetElementByName("SubEntityEnabledCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_ON, MashGUIEventFunctor(&MainLoop::OnSubEntityEnabled, this));
		windowElement->GetElementByName("SubEntityEnabledCB")->RegisterReceiver(aGUIEVENT_CB_TOGGLE_OFF, MashGUIEventFunctor(&MainLoop::OnSubEntityEnabled, this));
		windowElement->GetElementByName("EntityApplyMaterialBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnApplyMaterial, this));
		windowElement->GetElementByName("EntityApplyMaterialAllBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnApplyMaterialAll, this));
		windowElement->GetElementByName("EntityLodLB")->RegisterReceiver(aGUIEVENT_LB_SELECTION_CHANGE, MashGUIEventFunctor(&MainLoop::OnSelectActiveEntityLod, this));
		windowElement->GetElementByName("SubEntityLB")->RegisterReceiver(aGUIEVENT_LB_SELECTION_CHANGE, MashGUIEventFunctor(&MainLoop::OnSelectActiveSubEntity, this));
		windowElement->SetRenderEnabled(false);
		//this window starts from the bottom of the node common window
		windowElement->AddPosition(0.0f, elementPropertiesView->GetElementByName("NodeCommonWindow")->GetDestinationRegion().bottom.offset);

		m_device->GetGUIManager()->LoadGUILayout("./ParticleCreationGUILayout.xml", m_viewerRoot);
		m_particleCreationWindow = (MashGUIWindow*)m_viewerRoot->GetElementByName("ParticleCreationWindow");
		m_particleCreationWindow->GetElementByName("ParticleCreationCancelBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleCreationCancel, this));
		m_particleCreationWindow->GetElementByName("ParticleCreationAddBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnParticleCreationConfirm, this));
		m_particleCreationWindow->SetRenderEnabled(false);

		m_device->GetGUIManager()->LoadGUILayout("./TriangleColliderGUILayout.xml", m_viewerRoot);
		m_triColliderWindow = (MashGUIWindow*)m_viewerRoot->GetElementByName("TriColliderWindow");
		m_triColliderWindow->GetElementByName("TriColliderCancelBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnTriColliderCancel, this));
		m_triColliderWindow->GetElementByName("TriColliderCreateBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnTriColliderCreate, this));
		m_triColliderWindow->SetRenderEnabled(false);

		m_device->GetGUIManager()->LoadGUILayout("./ConfirmWindowGUILayout.xml", m_viewerRoot);
		windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("ConfirmWindow");
		windowElement->GetElementByName("ConfirmYesBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnConfirmYes, this));
		windowElement->GetElementByName("ConfirmNoBtn")->RegisterReceiver(aGUIEVENT_BTN_UP_CONFIRM, MashGUIEventFunctor(&MainLoop::OnConfirmNo, this));
		windowElement->SetRenderEnabled(false);

		m_viewportElement = (MashGUIViewport*)m_viewerRoot->GetElementByName("MainSceneViewport");
		m_viewportElement->SetInputEventCallback(MashInputEventFunctor(&MainLoop::OnViewportEvent, this));
		m_viewportElement->RegisterReceiver(aGUIEVENT_LOST_INPUTFOCUS, MashGUIEventFunctor(&MainLoop::OnViewportLostFocus, this));
		m_viewportElement->RegisterReceiver(aGUIEVENT_INPUTFOCUS, MashGUIEventFunctor(&MainLoop::OnViewportGainedFocus, this));

		UpdateSceneTree();

		return false;
	}

	bool Update(float dt)
	{
		m_device->GetSceneManager()->UpdateScene(dt, m_sceneRoot);
		
		return m_quit;
	}

	void Render()
	{
		m_viewportElement->SetViewport();

		m_device->GetSceneManager()->CullScene(m_sceneRoot);
		
		m_device->GetSceneManager()->DrawScene();

		if (m_selectedNode)
		{
			//some nodes have huge bounds that render badly
			if (m_selectedNode->GetNodeType() == aNODETYPE_BONE || 
				m_selectedNode->GetNodeType() == aNODETYPE_ENTITY ||
				m_selectedNode->GetNodeType() == aNODETYPE_DUMMY)
			{
				m_device->GetSceneManager()->DrawAABB(m_selectedNode->GetTotalBoundingBox(), sMashColour(255, 255, 255, 255));
			}
			else
			{
				//Render a fixed box around any node other than the current active camera
				if (!(m_selectedNode->GetNodeType() == aNODETYPE_CAMERA && ((MashCamera*)m_selectedNode)->IsActiveCamera()))
				{
					float aabbHalfWidth = 5.0f;
					MashAABB bounds(MashVector3(-aabbHalfWidth, -aabbHalfWidth, -aabbHalfWidth), MashVector3(aabbHalfWidth, aabbHalfWidth, aabbHalfWidth));
					bounds.min += m_selectedNode->GetRenderTransformState().translation;
					bounds.max += m_selectedNode->GetRenderTransformState().translation;
					m_device->GetSceneManager()->DrawAABB(bounds, sMashColour(255, 255, 255, 255));
				}
			}

			f32 arrowLength = (m_device->GetSceneManager()->GetActiveCamera()->GetWorldTransformState().translation - m_selectedNode->GetWorldTransformState().translation).Length() * 0.1f;
			MashVector3 lineStart = m_selectedNode->GetWorldTransformState().translation;
			MashVector3 zArrow = MashVector3(0.0f, 0.0f, arrowLength) + lineStart;
			m_device->GetSceneManager()->DrawLine(lineStart, zArrow, sMashColour(0, 0, 255, 255), false);

			MashVector3 xArrow = MashVector3(arrowLength, 0.0f, 0.0f) + lineStart;
			m_device->GetSceneManager()->DrawLine(lineStart, xArrow, sMashColour(255, 0, 0, 255), false);

			MashVector3 yArrow = MashVector3(0.0f, arrowLength, 0.0f) + lineStart;
			m_device->GetSceneManager()->DrawLine(lineStart, yArrow, sMashColour(0, 255, 0, 255), false);

			MashVector3 facing = m_selectedNode->GetWorldTransformState().TransformRotation(MashVector3(0.0f, 0.0f, arrowLength * 2.0f));
			m_device->GetSceneManager()->DrawLine(lineStart - facing, lineStart + facing, sMashColour(255, 255, 255, 255), false);
		}

        //flush all drawn components to the active viewport
		m_device->GetSceneManager()->FlushGeometryBuffers();

		m_viewportElement->RestoreOriginalViewport();

		if (m_fpsText->GetRenderEnabled())
		{
			char buffer[256];
            helpers::PrintToBuffer(buffer, sizeof(buffer), "FPS : %d", m_device->GetFPS());
			m_fpsText->SetText(buffer);
		}

		if (m_debugText->GetRenderEnabled())
		{
			const MashSceneManager::sSceneRenderInfo *sceneRenderInfo = m_device->GetSceneManager()->GetCurrentSceneRenderInfo();

			std::ostringstream stringStream;
			stringStream << "Node Count : " << m_loadedNodeData.size() << "\n";

			unsigned int activeLights = 0;
			unsigned int activeForwardRenderedLights = 0;
			MashList<MashLight*>::Iterator lightIter = m_loadedLights.Begin();
			MashList<MashLight*>::Iterator lightIterEnd = m_loadedLights.End();
			for(; lightIter != lightIterEnd; ++lightIter)
			{
				if ((*lightIter)->IsLightEnabled())
				{
					++activeLights;

					if ((*lightIter)->IsForwardRenderedLight())
						++activeForwardRenderedLights;
				}
			}

			if (sceneRenderInfo->forwardRenderedSolidObjectCount == 0 || sceneRenderInfo->forwardRenderedTransparentObjectCount == 0)
				activeForwardRenderedLights = 0;

			//lights in the scene that are currently enabled
			stringStream << "Active Light Count : " << activeLights << "\n";
			//forward rendered lights that are enabled and being used to light 1 or more objects
			stringStream << "Used Forward Light Count : " << activeForwardRenderedLights << "\n";
			
			//do this before drawing the gui because we only want the results from the scene
			stringStream << "Technique Changes : " << m_device->GetRenderer()->GetCurrentFrameTechniqueChangeCount() << "\n";
			stringStream << "Draw Calls : " << m_device->GetRenderer()->GetCurrentFrameDrawCount() << "\n\n";
			
			stringStream << "Rendered Shadow Objects : " << sceneRenderInfo->shadowObjectCount << "\n";
			stringStream << "Forward Rendered Solid Objects : " << sceneRenderInfo->forwardRenderedSolidObjectCount << "\n";
			stringStream << "Forward Rendered Transparent Objects : " << sceneRenderInfo->forwardRenderedTransparentObjectCount << "\n";
			stringStream << "Deferred Rendered Objects : " << sceneRenderInfo->deferredObjectSolidCount << "\n";
			
			m_debugText->SetText(stringStream.str().c_str());
		}

		m_device->GetGUIManager()->BeginDraw();
		m_device->GetGUIManager()->DrawAll();
		m_device->GetGUIManager()->EndDraw();
	}

	void OnCameraSpeedChange(const sInputEvent &eventData)
	{
		if (eventData.isPressed)
		{
			switch(eventData.character)
			{
			case '[':
				{
					m_freeCameraController->SetLinearSpeed(math::Max<f32>(10.0f, m_freeCameraController->GetLinearSpeed()-50));
					break;
				}
			case ']':
				{
					m_freeCameraController->SetLinearSpeed(m_freeCameraController->GetLinearSpeed()+50);
					break;
				}
			}
		}
	}

	void _UpdateSceneTree(MashGUITree *sceneTree, MashSceneNode *node, int parent)
	{
		std::map<MashSceneNode*, sSceneNodeData>::iterator nodeIter = m_loadedNodeData.find(node);
		nodeIter->second.treeId = sceneTree->AddItem(node->GetNodeName(), parent, node->GetNodeID());
		MashList<MashSceneNode*>::ConstIterator iter = node->GetChildren().Begin();
		MashList<MashSceneNode*>::ConstIterator iterEnd = node->GetChildren().End();
		for(; iter != iterEnd; ++iter)
			_UpdateSceneTree(sceneTree, *iter, nodeIter->second.treeId);
	}

	void UpdateSceneTree()
	{
		MashGUITree *sceneTree = (MashGUITree*)m_sceneTreeWindow->GetElementByName("SceneTree");
		sceneTree->RemoveAllItems();
		_UpdateSceneTree(sceneTree, m_sceneRoot, -1);
	}

	void UpdateItemInSceneTree(MashSceneNode *node)
	{
		RemoveItemFromSceneTree(node);
		AddItemToSceneTree(node);
	}

	void RemoveItemFromSceneTree(MashSceneNode *node)
	{
		MashGUITree *sceneTree = (MashGUITree*)m_sceneTreeWindow->GetElementByName("SceneTree");
		std::map<MashSceneNode*, sSceneNodeData>::iterator nodeIter = m_loadedNodeData.find(node);

		if (nodeIter != m_loadedNodeData.end())
			sceneTree->RemoveItem(nodeIter->second.treeId);
	}

	void AddItemToSceneTree(MashSceneNode *newNode)
	{
		MashGUITree *sceneTree = (MashGUITree*)m_sceneTreeWindow->GetElementByName("SceneTree");
		int32 parentItem = -1;
		if (newNode->GetParent())
		{
			std::map<MashSceneNode*, sSceneNodeData>::iterator nodeIter = m_loadedNodeData.find(newNode->GetParent());
			parentItem = nodeIter->second.treeId;
		}

		std::map<MashSceneNode*, sSceneNodeData>::iterator nodeIter = m_loadedNodeData.find(newNode);
		nodeIter->second.treeId = sceneTree->AddItem(newNode->GetNodeName(), parentItem, newNode->GetNodeID());
	}

	void UpdateSelectedSubEntityGUISettings(MashEntity *entity, MashGUIWindow *windowElement)
	{
		MashGUITextBox *tb = 0;

		if (entity->GetLodCount() > 0)
		{
			tb = (MashGUITextBox*)windowElement->GetElementByName("EntityLodDistanceTB");
			tb->SetTextInt(entity->GetLodDistances()[m_entityData.selectedLod]);

			MashSubEntity *subEntity = entity->GetSubEntity(m_entityData.selectedMesh, m_entityData.selectedLod);
			tb = (MashGUITextBox*)windowElement->GetElementByName("MaterialNameTB");
			tb->SetText(subEntity->GetMaterial()->GetMaterialName());
			MashGUICheckBox *cb = (MashGUICheckBox*)windowElement->GetElementByName("SubEntityEnabledCB");
			cb->SetChecked(subEntity->GetIsActive());

			MashGUIStaticText *st = (MashGUIStaticText*)windowElement->GetElementByName("EntityLodTrianglesST");
			unsigned int meshCount = entity->GetSubEntityCount(m_entityData.selectedLod);
			unsigned int triangleCount = 0;
			for(unsigned int i = 0; i < meshCount; ++i)
			{
				MashSubEntity *subEntity = entity->GetSubEntity(i, m_entityData.selectedLod);
				triangleCount += subEntity->GetMesh()->GetPrimitiveCount();
			}

			st->SetTextInt(triangleCount);

			st = (MashGUIStaticText*)windowElement->GetElementByName("EntityMeshTrianglesST");
			st->SetTextInt(subEntity->GetMesh()->GetPrimitiveCount());
		}
		else
		{
			tb = (MashGUITextBox*)windowElement->GetElementByName("EntityLodDistanceTB");
			tb->SetTextInt(0);

			tb = (MashGUITextBox*)windowElement->GetElementByName("MaterialNameTB");
			tb->SetText("");
			MashGUICheckBox *cb = (MashGUICheckBox*)windowElement->GetElementByName("SubEntityEnabledCB");
			cb->SetChecked(false);

			MashGUIStaticText *st = (MashGUIStaticText*)windowElement->GetElementByName("EntityTriangleCountST");
			st->SetTextInt(0);
			st = (MashGUIStaticText*)windowElement->GetElementByName("EntityMeshTrianglesST");
			st->SetTextInt(0);
		}
	}

	void UpdateMaterialSelectionWindow()
	{
		MashGUIListBox *lb = (MashGUIListBox*)m_materialSelectWindow->GetElementByName("MaterialLB");
		lb->ClearAllItems();

        std::map<MashStringc, sMaterial>::iterator iter = m_editorMaterials.begin();
        std::map<MashStringc, sMaterial>::iterator end = m_editorMaterials.end();
        for(; iter != end; ++iter)
        {
            lb->AddItem(iter->second.material->GetMaterialName(), 0);
        }			
	}

	void UpdateSelectedAnimationSettings(MashGUIWindow *windowElement)
	{
		bool useDefaults = true;
		if (m_selectedNode && m_selectedNode->GetAnimationMixer())
		{
			MashGUIEventDispatch::SetGlobalMute(true);

			MashAnimationMixer *mixer = m_selectedNode->GetAnimationMixer();
			MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");
			int selectedItemId = lb->GetSelectedItemId();
			if (selectedItemId > -1)
			{
				MashStringc animName = lb->GetItemText(selectedItemId);

				MashGUITextBox *tb = (MashGUITextBox*)windowElement->GetElementByName("NodeAnimWeightTB");
				tb->SetTextFloat(mixer->GetWeight(animName.GetCString()));
				tb = (MashGUITextBox*)windowElement->GetElementByName("NodeAnimTrackTB");
				tb->SetTextInt(mixer->GetTrack(animName.GetCString()));

				tb = (MashGUITextBox*)windowElement->GetElementByName("NodeAnimFrameTB");
				tb->SetTextInt(mixer->GetFrame(animName.GetCString()));
				tb = (MashGUITextBox*)windowElement->GetElementByName("NodeAnimSpeedTB");
				tb->SetTextFloat(mixer->GetSpeed(animName.GetCString()));

				tb = (MashGUITextBox*)windowElement->GetElementByName("NodeAnimFPSTB");
				tb->SetTextInt(mixer->GetFrameRate());

				MashGUIListBox *wrapLb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimWrapLB");
				wrapLb->SetActiveItemByUserValue(mixer->GetWrapMode(animName.GetCString()));
				MashGUIListBox *blendLb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimBlendLB");
				blendLb->SetActiveItemByUserValue(mixer->GetBlendMode(animName.GetCString()));

				useDefaults = false;
			}
		}

		if (useDefaults)
		{
			((MashGUITextBox*)windowElement->GetElementByName("NodeAnimWeightTB"))->SetTextFloat(1.0f);
			((MashGUITextBox*)windowElement->GetElementByName("NodeAnimTrackTB"))->SetTextInt(0);
			((MashGUITextBox*)windowElement->GetElementByName("NodeAnimFrameTB"))->SetTextInt(0);
			((MashGUITextBox*)windowElement->GetElementByName("NodeAnimFPSTB"))->SetTextInt(0);
			((MashGUITextBox*)windowElement->GetElementByName("NodeAnimSpeedTB"))->SetTextFloat(1.0f);
			((MashGUIListBox*)windowElement->GetElementByName("NodeAnimWrapLB"))->SetActiveItem(-1);
			((MashGUIListBox*)windowElement->GetElementByName("NodeAnimBlendLB"))->SetActiveItem(-1);
		}

		MashGUIEventDispatch::SetGlobalMute(false);
	}

	void UpdateEntiryTriangleColliderGUI(MashEntity *entity, MashGUIWindow *windowElement)
	{
		MashGUIStaticText *st = (MashGUIStaticText*)windowElement->GetElementByName("EntityTriColliderTypeST");
		MashTriangleCollider *collider = entity->GetModel()->GetTriangleCollider();
		if (!collider)
		{
			st->SetText("none");
			st = (MashGUIStaticText*)windowElement->GetElementByName("EntityTriColliderCountST");
			st->SetTextInt(0);
		}
		else
		{
			st->SetText(g_triangleColliderTypeName[collider->GetColliderType()]);
			st = (MashGUIStaticText*)windowElement->GetElementByName("EntityTriColliderCountST");

			unsigned int triangleCount = 0;
			for(unsigned int i = 0; i < collider->GetTriangleBufferCount(); ++i)
				triangleCount += collider->GetTriangleBuffer(i)->GetTriangleCount();

			st->SetTextInt(triangleCount);
		}
	}

	void UpdateLightGUI(MashLight *light, MashGUIWindow *windowElement)
	{
		MashGUIEventDispatch::SetGlobalMute(true);

		MashGUITextBox *tb = (MashGUITextBox*)windowElement->GetElementByName("LightAmbientRTB");
		tb->SetTextInt(math::FloatColourToInt(light->GetLightData()->ambient.r));
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightAmbientGTB");
		tb->SetTextInt(math::FloatColourToInt(light->GetLightData()->ambient.g));
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightAmbientBTB");
		tb->SetTextInt(math::FloatColourToInt(light->GetLightData()->ambient.b));
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightDiffuseRTB");
		tb->SetTextInt(math::FloatColourToInt(light->GetLightData()->diffuse.r));
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightDiffuseGTB");
		tb->SetTextInt(math::FloatColourToInt(light->GetLightData()->diffuse.g));
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightDiffuseBTB");
		tb->SetTextInt(math::FloatColourToInt(light->GetLightData()->diffuse.b));
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightSpecularRTB");
		tb->SetTextInt(math::FloatColourToInt(light->GetLightData()->specular.r));
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightSpecularGTB");
		tb->SetTextInt(math::FloatColourToInt(light->GetLightData()->specular.g));
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightSpecularBTB");
		tb->SetTextInt(math::FloatColourToInt(light->GetLightData()->specular.b));
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightOuterConeTB");
		tb->SetTextFloat(math::RadsToDegs(acos(light->GetLightData()->outerCone)));
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightInnerConeTB");
		tb->SetTextFloat(math::RadsToDegs(acos(light->GetLightData()->innerCone)));
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightFalloffTB");
		tb->SetTextFloat(light->GetLightData()->falloff);
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightRangeTB");
		tb->SetTextFloat(light->GetLightData()->range);
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightAtten0TB");
		tb->SetTextFloat(light->GetLightData()->atten.x);
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightAtten1TB");
		tb->SetTextFloat(light->GetLightData()->atten.y);
		tb = (MashGUITextBox*)windowElement->GetElementByName("LightAtten2TB");
		tb->SetTextFloat(light->GetLightData()->atten.z);

		MashGUICheckBox *cb = (MashGUICheckBox*)windowElement->GetElementByName("ForwardRenderedLightCB");
		cb->SetChecked(light->IsForwardRenderedLight());
		cb = (MashGUICheckBox*)windowElement->GetElementByName("ShadowsEnabledCB");
		cb->SetChecked(light->IsShadowsEnabled());
		cb = (MashGUICheckBox*)windowElement->GetElementByName("LightIsMainCB");
		cb->SetChecked(light->IsMainForwardRenderedLight());
		cb = (MashGUICheckBox*)windowElement->GetElementByName("LightIsEnabledCB");
		cb->SetChecked(light->IsLightEnabled());

		MashGUIEventDispatch::SetGlobalMute(false);
	}

	void UpdateSelectedNodeSettings(MashSceneNode *newNode)
	{
		if (newNode != m_selectedNode)
		{
			MashGUIView *elementPropertiesView = (MashGUIView*)m_viewerRoot->GetElementByName("Properties");

			if (m_selectedNode)
			{
				switch(m_selectedNode->GetNodeType())
				{
				case aNODETYPE_CAMERA:
					{
						MashGUIWindow *windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("CameraSettingsWindow");
						windowElement->SetRenderEnabled(false);
						break;
					}
				case aNODETYPE_ENTITY:
					{
						MashGUIWindow *windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("EntitySettingsWindow");
						windowElement->SetRenderEnabled(false);
						break;
					}
				case aNODETYPE_LIGHT:
					{
						MashGUIWindow *windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("LightSettingsWindow");
						windowElement->SetRenderEnabled(false);
						break;
					}
				case aNODETYPE_PARTICLE_EMITTER:
					{
						MashGUIWindow *windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("ParticleSettingsWindow");
						windowElement->SetRenderEnabled(false);
						break;
					}
				case aNODETYPE_DUMMY:
				case aNODETYPE_DECAL:
				case aNODETYPE_BONE:
					{
						break;
					}
				}

				if (m_selectedNode->GetAnimationMixer())
				{
					//m_selectedNode->GetAnimationMixer()->StopAll();
					StopNodeAnimation();
				}
			}

			m_selectedNode = newNode;

			MashGUIWindow *windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("NodeCommonWindow");
			if (m_selectedNode)
			{
				windowElement->SetRenderEnabled(true);

				MashGUIEventDispatch::SetGlobalMute(true);

				MashGUITextBox *tb = (MashGUITextBox*)windowElement->GetElementByName("NodeNameTB");
				tb->SetText(m_selectedNode->GetNodeName());
				tb = (MashGUITextBox*)windowElement->GetElementByName("NodeIDTB");
				tb->SetTextInt(m_selectedNode->GetUserID());
				tb = (MashGUITextBox*)windowElement->GetElementByName("NodePosXTB");
				tb->SetTextFloat(m_selectedNode->GetLocalTransformState().translation.x);
				tb = (MashGUITextBox*)windowElement->GetElementByName("NodePosYTB");
				tb->SetTextFloat(m_selectedNode->GetLocalTransformState().translation.y);
				tb = (MashGUITextBox*)windowElement->GetElementByName("NodePosZTB");
				tb->SetTextFloat(m_selectedNode->GetLocalTransformState().translation.z);
				tb = (MashGUITextBox*)windowElement->GetElementByName("NodeScaleXTB");
				tb->SetTextFloat(m_selectedNode->GetLocalTransformState().scale.x);
				tb = (MashGUITextBox*)windowElement->GetElementByName("NodeScaleYTB");
				tb->SetTextFloat(m_selectedNode->GetLocalTransformState().scale.y);
				tb = (MashGUITextBox*)windowElement->GetElementByName("NodeScaleZTB");
				tb->SetTextFloat(m_selectedNode->GetLocalTransformState().scale.z);

				MashVector3 eulurAngles;
				m_selectedNode->GetLocalTransformState().orientation.ToEulerAngles(eulurAngles);
				//Fix negative radian values
				if (eulurAngles.x < 0.0f)
					eulurAngles.x += math::Pi() * 2.0f;
				if (eulurAngles.y < 0.0f)
					eulurAngles.y += math::Pi() * 2.0f;
				if (eulurAngles.z < 0.0f)
					eulurAngles.z += math::Pi() * 2.0f;

				tb = (MashGUITextBox*)windowElement->GetElementByName("NodeRotXTB");
				tb->SetTextFloat(math::Clamp<int32>(0, 360, math::RadsToDegs(eulurAngles.x)));
				tb = (MashGUITextBox*)windowElement->GetElementByName("NodeRotYTB");
				tb->SetTextFloat(math::Clamp<int32>(0, 360, math::RadsToDegs(eulurAngles.y)));
				tb = (MashGUITextBox*)windowElement->GetElementByName("NodeRotZTB");
				tb->SetTextFloat(math::Clamp<int32>(0, 360, math::RadsToDegs(eulurAngles.z)));

				((MashGUICheckBox*)windowElement->GetElementByName("HasAnimMixerCB"))->SetChecked((bool)m_selectedNode->GetAnimationMixer());
				MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");
				lb->ClearAllItems();

				if (m_selectedNode->GetAnimationMixer())
				{
					MashAnimationMixer *animMixer = m_selectedNode->GetAnimationMixer();
					MashArray<MashStringc> animNames;
					animMixer->GetAnimationNames(animNames);
					for(unsigned int i = 0; i < animNames.Size(); ++i)
						lb->AddItem(animNames[i], 0);
				}

				MashGUIEventDispatch::SetGlobalMute(false);
				
				UpdateSelectedAnimationSettings(windowElement);

				MashGUIEventDispatch::SetGlobalMute(true);
				switch(m_selectedNode->GetNodeType())
				{
				case aNODETYPE_CAMERA:
					{
						windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("CameraSettingsWindow");
						windowElement->SetRenderEnabled(true);

						MashCamera *camera = (MashCamera*)m_selectedNode;
						tb = (MashGUITextBox*)windowElement->GetElementByName("CameraFOVTB");
						tb->SetTextInt(math::RadsToDegs(camera->GetFOV()));
						tb = (MashGUITextBox*)windowElement->GetElementByName("CameraAspectTB");
						tb->SetTextFloat(camera->GetAspect());
						tb = (MashGUITextBox*)windowElement->GetElementByName("CameraZNearTB");
						tb->SetTextFloat(camera->GetNear());
						tb = (MashGUITextBox*)windowElement->GetElementByName("CameraZFarTB");
						tb->SetTextFloat(camera->GetFar());

						MashGUICheckBox *cb = (MashGUICheckBox*)windowElement->GetElementByName("CameraAutoAspectCB");
						cb->SetChecked(camera->GetAutoAspectEnabled());
						cb = (MashGUICheckBox*)windowElement->GetElementByName("CameraIsMainCB");
						cb->SetChecked(camera->IsActiveCamera());

						break;
					}
				case aNODETYPE_ENTITY:
					{
						windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("EntitySettingsWindow");
						windowElement->SetRenderEnabled(true);

						MashEntity *entity = (MashEntity*)m_selectedNode;

						lb = (MashGUIListBox*)windowElement->GetElementByName("EntityLodLB");
						lb->ClearAllItems();

						char buffer[100];
						for(unsigned int i = 0; i < entity->GetLodCount(); ++i)
						{
							memset(buffer, 0, sizeof(buffer));
                            helpers::NumberToString(buffer, sizeof(buffer), i);
							lb->AddItem(buffer, i);
						}

						m_entityData.selectedLod = 0;
						m_entityData.selectedMesh = 0;
						
						lb->SetActiveItemByUserValue(m_entityData.selectedLod);

						lb = (MashGUIListBox*)windowElement->GetElementByName("SubEntityLB");
						lb->ClearAllItems();

						unsigned int lodCount = entity->GetSubEntityCount(m_entityData.selectedLod);
						for(unsigned int j = 0; j < lodCount; ++j)
						{
							memset(buffer, 0, sizeof(buffer));
                            helpers::NumberToString(buffer, sizeof(buffer), j);
							lb->AddItem(buffer, j);
						}

						lb->SetActiveItemByUserValue(m_entityData.selectedMesh);

						UpdateEntiryTriangleColliderGUI(entity, windowElement);
						UpdateSelectedSubEntityGUISettings(entity, windowElement);

						break;
					}
				case aNODETYPE_LIGHT:
					{
						windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("LightSettingsWindow");
						windowElement->SetRenderEnabled(true);

						MashLight *light = (MashLight*)m_selectedNode;
						UpdateLightGUI(light, windowElement);

						m_device->GetGUIManager()->SetGlobalMute(true);
						lb = (MashGUIListBox*)windowElement->GetElementByName("LightTypeLB");
						switch(light->GetLightType())
						{
						case aLIGHT_DIRECTIONAL:
							{
								lb->SetActiveItemByUserValue(0);
								break;
							}
						case aLIGHT_SPOT:
							{
								lb->SetActiveItemByUserValue(1);
								break;
							}
						case aLIGHT_POINT:
							{
								lb->SetActiveItemByUserValue(2);
								break;
							}
						}
						m_device->GetGUIManager()->SetGlobalMute(false);

						break;
					}
				case aNODETYPE_PARTICLE_EMITTER:
					{
						windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("ParticleSettingsWindow");
						windowElement->SetRenderEnabled(true);

						MashParticleSystem *particleSystem = (MashParticleSystem*)m_selectedNode;
						const sParticleSettings *particleSettings = particleSystem->GetParticleSettings();

						windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("ParticleSettingsWindow");
						((MashGUICheckBox*)windowElement->GetElementByName("ParticleIsPlayingCB"))->SetChecked(particleSystem->IsPlaying());
						((MashGUITextBox*)windowElement->GetElementByName("ParticleStartTimeTB"))->SetTextInt(particleSettings->startTime);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleSoftScaleTB"))->SetTextFloat(particleSettings->softParticleScale);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleSoftScaleTB"))->SetTextFloat(particleSettings->softParticleScale);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinStartColATB"))->SetTextInt(math::FloatColourToInt(particleSettings->minStartColour.a));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinStartColBTB"))->SetTextInt(math::FloatColourToInt(particleSettings->minStartColour.b));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinStartColGTB"))->SetTextInt(math::FloatColourToInt(particleSettings->minStartColour.g));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinStartColRTB"))->SetTextInt(math::FloatColourToInt(particleSettings->minStartColour.r));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxStartColATB"))->SetTextInt(math::FloatColourToInt(particleSettings->maxStartColour.a));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxStartColBTB"))->SetTextInt(math::FloatColourToInt(particleSettings->maxStartColour.b));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxStartColGTB"))->SetTextInt(math::FloatColourToInt(particleSettings->maxStartColour.g));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxStartColRTB"))->SetTextInt(math::FloatColourToInt(particleSettings->maxStartColour.r));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinEndColATB"))->SetTextInt(math::FloatColourToInt(particleSettings->minEndColour.a));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinEndColBTB"))->SetTextInt(math::FloatColourToInt(particleSettings->minEndColour.b));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinEndColGTB"))->SetTextInt(math::FloatColourToInt(particleSettings->minEndColour.g));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinEndColRTB"))->SetTextInt(math::FloatColourToInt(particleSettings->minEndColour.r));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxEndColATB"))->SetTextInt(math::FloatColourToInt(particleSettings->maxEndColour.a));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxEndColBTB"))->SetTextInt(math::FloatColourToInt(particleSettings->maxEndColour.b));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxEndColGTB"))->SetTextInt(math::FloatColourToInt(particleSettings->maxEndColour.g));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxEndColRTB"))->SetTextInt(math::FloatColourToInt(particleSettings->maxEndColour.r));
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinVelocityZTB"))->SetTextFloat(particleSettings->minVelocity.z);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinVelocityYTB"))->SetTextFloat(particleSettings->minVelocity.y);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinVelocityXTB"))->SetTextFloat(particleSettings->minVelocity.x);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxVelocityZTB"))->SetTextFloat(particleSettings->maxVelocity.z);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxVelocityYTB"))->SetTextFloat(particleSettings->maxVelocity.y);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxVelocityXTB"))->SetTextFloat(particleSettings->maxVelocity.x);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxCountTB"))->SetTextInt(particleSettings->maxParticleCount);
						((MashGUITextBox*)windowElement->GetElementByName("ParticlePerSecondTB"))->SetTextInt(particleSettings->particlesPerSecond);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinStartSizeTB"))->SetTextInt(particleSettings->minStartSize);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxStartSizeTB"))->SetTextInt(particleSettings->maxStartSize);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinEndSizeTB"))->SetTextInt(particleSettings->minEndSize);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxEndSizeTB"))->SetTextInt(particleSettings->maxEndSize);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinRotateTB"))->SetTextInt(particleSettings->minRotateSpeed);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxRotateTB"))->SetTextInt(particleSettings->maxRotateSpeed);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMinDurationTB"))->SetTextFloat(particleSettings->minDuration);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleMaxDurationTB"))->SetTextFloat(particleSettings->maxDuration);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleGravityXTB"))->SetTextFloat(particleSettings->gravity.x);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleGravityYTB"))->SetTextFloat(particleSettings->gravity.y);
						((MashGUITextBox*)windowElement->GetElementByName("ParticleGravityZTB"))->SetTextFloat(particleSettings->gravity.z);

						MashStringc particleTypeString;
						switch(particleSystem->GetParticleType())
						{
						case aPARTICLE_CPU:
							particleTypeString = "CPU ";
							break;
						case aPARTICLE_GPU:
							particleTypeString = "GPU ";
							break;
						case aPARTICLE_MESH:
							particleTypeString = "Mesh ";
							break;
						case aPARTICLE_CPU_SOFT_DEFERRED:
							particleTypeString = "CPU Soft Deferred";
							break;
						case aPARTICLE_GPU_SOFT_DEFERRED:
							particleTypeString = "CPU Soft Deferred";
							break;
						}

						switch(particleSystem->GetParticleLightingType())
						{
						case aLIGHT_TYPE_AUTO:
							particleTypeString += "Auto Lighting";
							break;
						case aLIGHT_TYPE_DEFERRED:
							particleTypeString += "Deferred Lighting";
							break;
						case aLIGHT_TYPE_PIXEL:
							particleTypeString += "Pixel Lighting";
							break;
						case aLIGHT_TYPE_VERTEX:
							particleTypeString += "Vertex Lighting";
							break;
						case aLIGHT_TYPE_NONE:
							particleTypeString += "No Lighting";
						}

						((MashGUIStaticText*)windowElement->GetElementByName("ParticleTypeST"))->SetText(particleTypeString);

						if (m_loadedNodeData[m_selectedNode].particleData.diffuseTextureName)
							((MashGUITextBox*)windowElement->GetElementByName("ParticleTextureTB"))->SetText(m_loadedNodeData[m_selectedNode].particleData.diffuseTextureName);
						else
							((MashGUITextBox*)windowElement->GetElementByName("ParticleTextureTB"))->SetText("");

						if (particleSystem->GetModel())
							((MashGUITextBox*)windowElement->GetElementByName("ParticleMeshTB"))->SetText(m_loadedNodeData[m_selectedNode].particleData.meshName);
						else
							((MashGUITextBox*)windowElement->GetElementByName("ParticleMeshTB"))->SetText("");

						break;
					}
				case aNODETYPE_DUMMY:
				case aNODETYPE_DECAL:
				case aNODETYPE_BONE:
					{
						break;
					}
				}

				MashGUIEventDispatch::SetGlobalMute(false);
			}
			else
			{
				windowElement->SetRenderEnabled(false);
			}
		}
	}

	void UpdateAnimationRangeInfoGUI(MashGUIListBox *lb)
	{
		int selectedItemUserVal = lb->GetSelectedItemId();
		if (selectedItemUserVal > -1)
		{
			std::map<int, sAnimRange>::const_iterator iter = m_selectedNodeAnimRanges.find(selectedItemUserVal);
			if (iter != m_selectedNodeAnimRanges.end())
			{
				MashGUIEventDispatch::SetGlobalMute(true);

				MashGUITextBox *tb = (MashGUITextBox*)m_animationRangeWindow->GetElementByName("AnimRangeStartFrameTB");
				tb->SetTextInt(iter->second.start);
				tb = (MashGUITextBox*)m_animationRangeWindow->GetElementByName("AnimRangeEndFrameTB");
				tb->SetTextInt(iter->second.end);
				tb = (MashGUITextBox*)m_animationRangeWindow->GetElementByName("AnimRangeNameTB");
				tb->SetText(iter->second.name);

				char textBuffer[256];
				memset(textBuffer, 0, sizeof(textBuffer));
                helpers::PrintToBuffer(textBuffer, sizeof(textBuffer), "%s (%d,%d)", iter->second.name.GetCString(), iter->second.start, iter->second.end);
                
				lb->SetItemText(selectedItemUserVal, MashStringc(textBuffer));

				MashGUIEventDispatch::SetGlobalMute(false);
			}
		}
	}

	void CollectAnimationBufferData(MashSceneNode *node, std::map<MashStringc, sAnimRange> &tempAnimData)
	{
		const MashAnimationBuffer *animBuffer = node->GetAnimationBuffer();
		if (animBuffer)
		{
			std::map<MashStringc, MashArray<MashAnimationBuffer::sController> >::const_iterator iter = animBuffer->GetAnimationKeySets().begin();
			std::map<MashStringc, MashArray<MashAnimationBuffer::sController> >::const_iterator iterEnd = animBuffer->GetAnimationKeySets().end();
			for(; iter != iterEnd; ++iter)
			{
				unsigned int tempStart, tempEnd;
				animBuffer->GetAnimationFrameBounds(iter->first, tempStart, tempEnd);

				if (tempAnimData[iter->first].start > tempStart)
					tempAnimData[iter->first].start = tempStart;
				if (tempAnimData[iter->first].end < tempEnd)
					tempAnimData[iter->first].end = tempEnd;
			}

			m_selectedNodeAnimBuffers.PushBack(node);
		}

		MashList<MashSceneNode*>::ConstIterator nodeIter = node->GetChildren().Begin();
		MashList<MashSceneNode*>::ConstIterator nodeIterEnd = node->GetChildren().End();
		for(; nodeIter != nodeIterEnd; ++nodeIter)
		{
			CollectAnimationBufferData(*nodeIter, tempAnimData);
		}
	}

	void UpdateAnimationEditWindow(MashSceneNode *node)
	{
		m_selectedNodeAnimRanges.clear();
		m_selectedNodeAnimBuffers.Clear();
		std::map<MashStringc, sAnimRange> tempAnimData;
		CollectAnimationBufferData(node, tempAnimData);
		
		MashGUIListBox *lb = (MashGUIListBox*)m_animationRangeWindow->GetElementByName("AnimRangeLB");
		lb->ClearAllItems();

		std::map<MashStringc, sAnimRange>::iterator iter = tempAnimData.begin();
		std::map<MashStringc, sAnimRange>::iterator iterEnd = tempAnimData.end();
		for(; iter != iterEnd; ++iter)
		{
			iter->second.name = iter->first;
			m_selectedNodeAnimRanges.insert(std::make_pair(lb->AddItem(iter->first, 0), iter->second));
		}

		lb->SetMute(true);
		lb->SetActiveItemByUserValue(0);
		lb->SetMute(false);

		UpdateAnimationRangeInfoGUI(lb);
	}

	void RemoveNodesFromApp(MashSceneNode *node)
	{
		RemoveItemFromSceneTree(node);
		m_loadedNodeData.erase(node);

		MashList<MashSceneNode*>::ConstIterator iter = node->GetChildren().Begin();
		MashList<MashSceneNode*>::ConstIterator iterEnd = node->GetChildren().End();
		for(; iter != iterEnd; ++iter)
		{
			RemoveNodesFromApp(*iter);
		}
	}

	void _OnDelete()
	{
		if (m_selectedNode)
		{
			if (m_selectedNode != m_sceneRoot && 
				m_selectedNode != m_defaultCamera &&
				m_selectedNode != m_defaultLight)
			{
				if (m_selectedNode->GetNodeType() == aNODETYPE_LIGHT)
					m_loadedLights.Erase((MashLight*)m_selectedNode);

				MashSceneNode *nodeToDelete = m_selectedNode;
				UpdateSelectedNodeSettings(0);

				RemoveNodesFromApp(nodeToDelete);
				m_device->GetSceneManager()->RemoveSceneNode(nodeToDelete);
				//UpdateSceneTree();
			}
		}
	}

	void OnDelete(const sInputEvent &eventData)
	{
		if (eventData.isPressed == 0)
		{
			if (m_selectedNode)
			{
				MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("ConfirmWindow");
				windowElement->SetRenderEnabled(true);
			}
		}
	}

	void OnNewLogMessageSelected(const sGUIEvent &eventData)
	{
		m_debugLogWindow->SetRenderEnabled(true);
		//MashGUIStaticText *debugText = (MashGUIStaticText*)m_debugLogWindow->GetElementByName("DebugTextST");
		MashGUIListBox *lb = (MashGUIListBox*)m_viewerRoot->GetElementByName("LogMessageLB");
		//debugText->SetText(lb->GetItemText(lb->GetSelectedItemId()));

		lb->SetActiveItem(-1);//removes highlight
	}

	void OnLogMessage(const sLogEvent &eventData)
	{
		if ((eventData.level & MashLog::aERROR_LEVEL_ERROR) || (eventData.level & MashLog::aERROR_LEVEL_WARNING))
		{
			MashGUIListBox *lb = (MashGUIListBox*)m_viewerRoot->GetElementByName("LogMessageLB");

			if (eventData.level & MashLog::aERROR_LEVEL_WARNING)
			{
				lb->SetItemText(0, MashStringc("Warning : ") + eventData.msg);
				MashRectangle2 iconSource(0.0f, 0.0f, 32.0f, 32.0f);
				lb->SetItemIcon(0, m_errorIcons, &iconSource);
			}
			else if (eventData.level & MashLog::aERROR_LEVEL_ERROR)
			{
				lb->SetItemText(0, MashStringc("Error : ") + eventData.msg);
				MashRectangle2 iconSource(32.0f, 0.0f, 64.0f, 32.0f);
				lb->SetItemIcon(0, m_errorIcons, &iconSource);
			}

			lb->SetActiveItem(-1);//removes highlight
		}
	}

	void OnViewportLostFocus(const sGUIEvent &eventData)
	{
		m_freeCameraController->SetInputState(false);
	}

	void OnViewportGainedFocus(const sGUIEvent &eventData)
	{
		if (m_defaultCamera->IsActiveCamera())
			m_freeCameraController->SetInputState(true);
	}

	void OnViewportEvent(const sInputEvent &eventData)
	{
		//on release
		if ((eventData.eventType == sInputEvent::aEVENTTYPE_MOUSE) && 
			(eventData.action == aMOUSEEVENT_B1) &&
			(eventData.isPressed == 0))
		{
			//if (m_device->GetGUIManager()->GetFocusedElement() == m_viewportElement)
			{
				m_viewportElement->SetViewport();

				MashVector2 viewportWidthHeight;
				m_viewportElement->GetViewportWidthHeight(viewportWidthHeight.x, viewportWidthHeight.y);
				//make sure the mouse click occured within the scene region
				//if (m_viewportElement->GetAbsoluteClippedRegion().Intersects(m_device->GetInputManager()->GetCursorPosition()))
				{
					MashScenePick scenePick;
					MashSceneNode *selectedNode = 0;
					float rayLength = 0xFFFFFFFFF;
					MashRay mouseRay;
					m_device->GetSceneManager()->GetActiveCamera()->TransformScreenToWorldPosition(viewportWidthHeight, m_device->GetInputManager()->GetCursorPosition(),
						mouseRay.origin, mouseRay.dir);

					MashArray<MashSceneNode*> collidingNodes;
					scenePick.GetNodesByBounds(m_sceneRoot, mouseRay, aNODETPYE_ALL, collidingNodes);
					//remove the active cam from the list 
					if (collidingNodes.Contains(m_device->GetSceneManager()->GetActiveCamera()))
						collidingNodes.Erase(collidingNodes.Search(m_device->GetSceneManager()->GetActiveCamera()));
					//remove the scene root
					if (collidingNodes.Contains(m_sceneRoot))
						collidingNodes.Erase(collidingNodes.Search(m_sceneRoot));

					if (!collidingNodes.Empty())
					{
						//cycles the selected nodes that overlap
						m_lastSelectedNodeCycle = ++m_lastSelectedNodeCycle % collidingNodes.Size();
						collidingNodes.Sort();
						selectedNode = collidingNodes[m_lastSelectedNodeCycle];
					}

					if (selectedNode)
					{
						if (m_selectedNode != selectedNode)
						{
							int selectedItemId = -1;
							if (selectedNode)
								selectedItemId = selectedNode->GetNodeID();

							//the tree will send a message when an active item is sent. We dont need that to happen else twice the work will be done.
                            MashGUITree *sceneTree = (MashGUITree*)m_sceneTreeWindow->GetElementByName("SceneTree");
							sceneTree->SetMute(true);
							sceneTree->SetActiveItemByUserId(selectedItemId);
							sceneTree->SetMute(false);
							
							UpdateSelectedNodeSettings(selectedNode);
						}
					}
				}

				m_viewportElement->RestoreOriginalViewport();
			}
		}
	}

	void OnMaterialLoad(const sGUIEvent &eventData)
	{
		m_openFileDialog->OpenDialog();
		m_fileDialogUsage = DIALOG_USAGE_LOAD_MATERIAL;
	}

	void OnMaterialLoadAll(const sGUIEvent &eventData)
	{
		m_openFileDialog->OpenDialog();
		m_fileDialogUsage = DIALOG_USAGE_LOAD_MATERIAL_ALL;
	}

	void OnMaterialSelected(const sGUIEvent &eventData)
	{
        if (m_selectedNode && (m_selectedNode->GetNodeType() == aNODETYPE_ENTITY))
        {
            m_materialSelectWindow->SetRenderEnabled(false);
            MashGUIListBox *lb = (MashGUIListBox*)m_materialSelectWindow->GetElementByName("MaterialLB");
            std::map<MashStringc, sMaterial>::iterator materialIter = m_editorMaterials.find(lb->GetItemText(lb->GetSelectedItemId()));
            if (materialIter != m_editorMaterials.end())
            {
                if (m_applyMaterialType == APPLY_MATERIAL_SINGLE)
                    ((MashEntity*)m_selectedNode)->GetSubEntity(m_entityData.selectedMesh, m_entityData.selectedLod)->SetMaterial(materialIter->second.material);
                else
                {
                    MashEntity *entity = (MashEntity*)m_selectedNode;
                    for(unsigned int lod = 0; lod < entity->GetLodCount(); ++lod)
                    {
                        for(unsigned int mesh = 0; mesh < entity->GetSubEntityCount(lod); ++mesh)
                        {
                            entity->GetSubEntity(mesh, lod)->SetMaterial(materialIter->second.material);
                        }
                    }
                }
                
                //update text box
                MashGUIView *elementPropertiesView = (MashGUIView*)m_viewerRoot->GetElementByName("Properties");
                MashGUIWindow *windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("EntitySettingsWindow");
                ((MashGUITextBox*)windowElement->GetElementByName("MaterialNameTB"))->SetText(materialIter->second.material->GetMaterialName());
            }
        }
	}
    
    void OnMaterialReload(const sGUIEvent &eventData)
    {
		MashGUIListBox *lb = (MashGUIListBox*)m_materialSelectWindow->GetElementByName("MaterialLB");
		std::map<MashStringc, sMaterial>::iterator materialIter = m_editorMaterials.find(lb->GetItemText(lb->GetSelectedItemId()));
		if (materialIter != m_editorMaterials.end())
		{
            if (!materialIter->second.fileName.Empty())
                LoadMaterialFile(materialIter->second.filePath, materialIter->second.fileName, true);
        }
    }

	void OnMaterialSelectCancelled(const sGUIEvent &eventData)
	{
		m_materialSelectWindow->SetRenderEnabled(false);
	}

	void OnAnimRangeCancelled(const sGUIEvent &eventData)
	{
		m_animationRangeWindow->SetRenderEnabled(false);
	}

	void OnAnimRangeInsert(const sGUIEvent &eventData)
	{
		sAnimRange newRange;
		newRange.start = 0;
		newRange.end = 0;

		//char nameBuffer[256];
		//unsigned int nameCounter = 0;
		////creates a unique name
		//sprintf_s(nameBuffer, sizeof(nameBuffer), "newAnimation%d", nameCounter++);
		//std::map<std::string, sAnimRange>::const_iterator animIter = m_selectedNodeAnimRanges.begin();
		//std::map<std::string, sAnimRange>::const_iterator animIterEnd = m_selectedNodeAnimRanges.end();
		//do
		//{
		//	sprintf_s(nameBuffer, sizeof(nameBuffer), "newAnimation%d", nameCounter++);
		//}while((strcmp(nameBuffer, animIter->first.c_str()) == 0) && (animIter != animIterEnd) && (nameCounter < 1000));
		newRange.name = ((MashGUITextBox*)m_animationRangeWindow->GetElementByName("AnimRangeNameTB"))->GetText();
		//check for unique name
		std::map<int, sAnimRange>::iterator nameIter = m_selectedNodeAnimRanges.begin();
		std::map<int, sAnimRange>::iterator nameIterEnd = m_selectedNodeAnimRanges.end();
		for(; nameIter != nameIterEnd; ++nameIter)
		{
			if (nameIter->second.name == newRange.name)
			{
				//post error
				return;
			}
		}

		newRange.start = ((MashGUITextBox*)m_animationRangeWindow->GetElementByName("AnimRangeStartFrameTB"))->GetTextAsInt();
		newRange.end = ((MashGUITextBox*)m_animationRangeWindow->GetElementByName("AnimRangeEndFrameTB"))->GetTextAsInt();

		if (newRange.start > newRange.end)
		{
			//post error
			return;
		}	

		MashGUIListBox *lb = (MashGUIListBox*)m_animationRangeWindow->GetElementByName("AnimRangeLB");
		int newItemId = lb->AddItem(newRange.name, 0);
		m_selectedNodeAnimRanges.insert(std::make_pair(newItemId, newRange));
		lb->SetMute(true);
		lb->SetActiveItem(newItemId);
		lb->SetMute(false);

		m_selectedNodeAnimRangeUpdate = ANIM_UPDATE_ALL;

		UpdateAnimationRangeInfoGUI(lb);
	}

	void OnAnimRangeRemove(const sGUIEvent &eventData)
	{
		MashGUIListBox *lb = (MashGUIListBox*)m_animationRangeWindow->GetElementByName("AnimRangeLB");
		int selectedItemUserVal = lb->GetSelectedItemId();
		if (selectedItemUserVal > -1)
		{
			m_selectedNodeAnimRanges.erase(m_selectedNodeAnimRanges.find(selectedItemUserVal));
			lb->RemoveItem(selectedItemUserVal);
			m_selectedNodeAnimRangeUpdate = ANIM_UPDATE_ALL;
		}
	}

	void OnAnimRangeUpdate(const sGUIEvent &eventData)
	{
		MashGUIListBox *lb = (MashGUIListBox*)m_animationRangeWindow->GetElementByName("AnimRangeLB");
		int selectedItemUserVal = lb->GetSelectedItemId();
		if (selectedItemUserVal > -1)
		{
			int startFrame, endFrame;

			std::map<int, sAnimRange>::iterator itemIter = m_selectedNodeAnimRanges.find(selectedItemUserVal);
			MashStringc oldName = itemIter->second.name;
			MashStringc newName = ((MashGUITextBox*)m_animationRangeWindow->GetElementByName("AnimRangeNameTB"))->GetText();

			bool insertNewItem = false;
			if (oldName != newName)
			{
				//this check makes sure that if the user has changed an animation name, that it is unique
				std::map<int, sAnimRange>::iterator nameIter = m_selectedNodeAnimRanges.begin();
				std::map<int, sAnimRange>::iterator nameIterEnd = m_selectedNodeAnimRanges.end();
				for(; nameIter != nameIterEnd; ++nameIter)
				{
					if (nameIter->second.name == newName)
					{
						//post error
						return;
					}
				}

				//name is unique at this point
				insertNewItem = true;
			}
			
			startFrame = ((MashGUITextBox*)m_animationRangeWindow->GetElementByName("AnimRangeStartFrameTB"))->GetTextAsInt();
			endFrame = ((MashGUITextBox*)m_animationRangeWindow->GetElementByName("AnimRangeEndFrameTB"))->GetTextAsInt();

			if (startFrame > endFrame)
			{
				//post error
				return;
			}	

			if (insertNewItem)
			{
				sAnimRange newAnimation;
				newAnimation.name = newName;
				newAnimation.start = startFrame;
				newAnimation.end = endFrame;

				int mapIndex = itemIter->first;

				//erase the old element so we can update the name
				m_selectedNodeAnimRanges.erase(itemIter);
				itemIter = m_selectedNodeAnimRanges.insert(std::make_pair(mapIndex, newAnimation)).first;
			}

			itemIter->second.start = startFrame;
			itemIter->second.end = endFrame;

			m_selectedNodeAnimRangeUpdate = ANIM_UPDATE_ALL;

			UpdateAnimationRangeInfoGUI(lb);
		}
	}

	void OnAnimRangeLoad(const sGUIEvent &eventData)
	{
		m_openFileDialog->OpenDialog();
		m_fileDialogUsage = DIALOG_USAGE_LOAD_ANIM_RANGE;
	}

	void OnAnimRangeSave(const sGUIEvent &eventData)
	{
		if ((m_selectedNodeAnimRangeUpdate != ANIM_UPDATE_NONE) && !m_selectedNodeAnimBuffers.Empty())
		{
			if (m_selectedNodeAnimRangeUpdate & ANIM_UPDATE_ALL)
			{
				MashArray<sAnimationClip> newClips;
				std::map<int, sAnimRange>::const_iterator animIter = m_selectedNodeAnimRanges.begin();
				std::map<int, sAnimRange>::const_iterator animIterEnd = m_selectedNodeAnimRanges.end();
				for(; animIter != animIterEnd; ++animIter)
				{
					sAnimationClip newClip;
					newClip.start = animIter->second.start;
					newClip.end = animIter->second.end;
					newClip.name = animIter->second.name;
					newClips.PushBack(newClip);
				}

				const unsigned int numNodes = m_selectedNodeAnimBuffers.Size();
				for(unsigned int i = 0; i < numNodes; ++i)
				{
					MashAnimationBuffer *originalAnimBuffer = m_loadedNodeData[m_selectedNodeAnimBuffers[i]].originalAnimBuffer;

					MashAnimationBuffer *choppedAnimBuffer = 0;
					if (originalAnimBuffer->ChopBuffer(m_device->GetSceneManager()->GetControllerManager(), newClips, &choppedAnimBuffer) == aMASH_FAILED)
						return;

					if (choppedAnimBuffer)
					{
						m_selectedNodeAnimBuffers[i]->RemoveAnimationBuffer();
						m_selectedNodeAnimBuffers[i]->SetAnimationBuffer(choppedAnimBuffer);
						choppedAnimBuffer->Drop();
					}
				}
			}
			else if (m_selectedNodeAnimRangeUpdate & ANIM_UPDATE_NAME_ONLY)
			{
				MashArray<MashStringc> animNames;
				const unsigned int numNodes = m_selectedNodeAnimBuffers.Size();
				for(unsigned int i = 0; i < numNodes; ++i)
				{
					animNames.Clear();

					MashAnimationBuffer *originalAnimBuffer = m_loadedNodeData[m_selectedNodeAnimBuffers[i]].originalAnimBuffer;
					std::map<MashStringc, MashArray<MashAnimationBuffer::sController> >::const_iterator iter = originalAnimBuffer->GetAnimationKeySets().begin();
					std::map<MashStringc, MashArray<MashAnimationBuffer::sController> >::const_iterator iterEnd = originalAnimBuffer->GetAnimationKeySets().end();
					for(; iter != iterEnd; ++iter)
						animNames.PushBack(iter->first);

					std::map<int, sAnimRange>::const_iterator animIter = m_selectedNodeAnimRanges.begin();
					std::map<int, sAnimRange>::const_iterator animIterEnd = m_selectedNodeAnimRanges.end();
					unsigned int counter = 0;
					for(; animIter != animIterEnd; ++animIter, ++counter)
						originalAnimBuffer->SetAnimationName(animNames[counter], animIter->second.name);
				}
			}

			//UpdateAnimationEditWindow(m_selectedNode);

			MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
			//animation mixer will need to be recreated
			if (m_selectedNode->GetAnimationMixer())
			{
				m_selectedNode->RemoveAnimationMixer();
				MashAnimationMixer *newMixer = m_device->GetSceneManager()->GetControllerManager()->CreateMixer(m_selectedNode);
				//m_device->GetSceneManager()->GetControllerManager()->AddAnimationsToMixer(newMixer, m_selectedNode);

				//refill animation lb
				
				MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");
				lb->ClearAllItems();

				MashArray<MashStringc> animNames;
				newMixer->GetAnimationNames(animNames);
				for(unsigned int i = 0; i < animNames.Size(); ++i)
					lb->AddItem(animNames[i], 0);

				
			}
			
			UpdateSelectedAnimationSettings(windowElement);
		}

		m_selectedNodeAnimRangeUpdate = ANIM_UPDATE_NONE;
		m_animationRangeWindow->SetRenderEnabled(false);
	}

	void OnNodeAnimChange(const sGUIEvent &eventData)
	{
		MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
		UpdateSelectedAnimationSettings(windowElement);
	}

	void OnNodePlayAnimation(const sGUIEvent &eventData)
	{
		MashAnimationMixer *newMixer = m_selectedNode->GetAnimationMixer();
		if (newMixer)
		{
			MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
			MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");

			if (lb->GetSelectedItemId() > -1)
			{
				StopNodeAnimation();

				//lb->SetItemIcon(m_playingAnimtionIndex, 0);
				m_playingAnimtionIndex = lb->GetSelectedItemId();
				lb->SetItemIcon(m_playingAnimtionIndex, m_playIcon);
				
				MashStringc animationName = lb->GetItemText(lb->GetSelectedItemId());
				newMixer->SetWeight(animationName.GetCString(), 1.0f);
				newMixer->Play(animationName.GetCString());
			}
		}
	}

	void StopNodeAnimation()
	{
		MashAnimationMixer *newMixer = m_selectedNode->GetAnimationMixer();
		if (newMixer)
		{
			MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
			MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");

			if (m_playingAnimtionIndex > -1)
			{
				MashStringc animationName = lb->GetItemText(m_playingAnimtionIndex);
				if (newMixer->GetBlendMode(animationName.GetCString()) != aBLEND_ADDITIVE)
					newMixer->SetWeight(animationName.GetCString(), 0.0f);

				newMixer->Stop(animationName.GetCString(), true);

				lb->SetItemIcon(m_playingAnimtionIndex, 0);
				m_playingAnimtionIndex = -1;
			}
		}
	}

	void OnNodeStopAnimation(const sGUIEvent &eventData)
	{
		StopNodeAnimation();
	}

	void OnToggleAnimMixer(const sGUIEvent &eventData)
	{
		MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
		MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");
		lb->ClearAllItems();

		if (m_selectedNode->GetAnimationMixer())
		{
			m_selectedNode->RemoveAnimationMixer();
		}
		else
		{
			MashAnimationMixer *newMixer = m_device->GetSceneManager()->GetControllerManager()->CreateMixer(m_selectedNode);
			//m_device->GetSceneManager()->GetControllerManager()->AddAnimationsToMixer(newMixer, m_selectedNode);
			
			MashArray<MashStringc> animNames;
			newMixer->GetAnimationNames(animNames);
			for(unsigned int i = 0; i < animNames.Size(); ++i)
			{
				lb->AddItem(animNames[i], 0);
				newMixer->SetWrapMode(animNames[i].GetCString(), aWRAP_LOOP);
			}
		}
	}

	void OnChangePreferredLightingSelect(const sGUIEvent &eventData)
	{
		MashGUIListBox *lightTypeLb = (MashGUIListBox*)m_preferredLightingWindow->GetElementByName("PreferredLightingLB");
		if (lightTypeLb->GetSelectedItemId() != -1)
			m_device->GetSceneManager()->SetPreferredLightingMode((eLIGHTING_TYPE)lightTypeLb->GetItemUserValue(lightTypeLb->GetSelectedItemId()));

		m_preferredLightingWindow->SetRenderEnabled(false);
	}

	void OnChangePreferredLightingCancel(const sGUIEvent &eventData)
	{
		m_preferredLightingWindow->SetRenderEnabled(false);
	}

	void OnChangeParentSelect(const sGUIEvent &eventData)
	{
		MashGUITree *sceneTree = (MashGUITree*)m_changeParentWindow->GetElementByName("ChangeParentTree");
		if (sceneTree->GetSelectedItemID() > -1)
		{
			MashSceneNode *newParent = m_device->GetSceneManager()->GetSceneNodeByID(sceneTree->GetItemUserId(sceneTree->GetSelectedItemID()));
			if (newParent && (newParent != m_selectedNode))
			{
				newParent->AddChild(m_selectedNode);

				UpdateItemInSceneTree(m_selectedNode);
			}
		}

		m_changeParentWindow->SetRenderEnabled(false);
	}

	void OnChangeParentCancel(const sGUIEvent &eventData)
	{
		m_changeParentWindow->SetRenderEnabled(false);
	}

	void FillChangeParentTree(MashGUITree *tree, int parent, const MashArray<MashGUITree::sItemData> &items)
	{
		for(unsigned int i = 0; i < items.Size(); ++i)
		{
			int id = tree->AddItem(items[i].text, parent, items[i].id);
			FillChangeParentTree(tree, id, items[i].children);
		}
	}

	void OnNodeChangeParent(const sGUIEvent &eventData)
	{
		MashGUITree *sceneTree = (MashGUITree*)m_sceneTreeWindow->GetElementByName("SceneTree");
		MashArray<MashGUITree::sItemData> items;
		sceneTree->GetItems(items);
		sceneTree = (MashGUITree*)m_changeParentWindow->GetElementByName("ChangeParentTree");
		sceneTree->RemoveAllItems();
		for(unsigned int i = 0; i < items.Size(); ++i)
		{
			int id = sceneTree->AddItem(items[i].text, -1, items[i].id);
			FillChangeParentTree(sceneTree, id, items[i].children);
		}

		m_changeParentWindow->SetRenderEnabled(true);
	}

	void OnAnimRangeSelected(const sGUIEvent &eventData)
	{
		UpdateAnimationRangeInfoGUI((MashGUIListBox*)eventData.component);
	}

	void OnAnimRangeStartFrame(const sGUIEvent &eventData)
	{
		////just checks to see if the data has changed, otherwise there is no need to update
		//MashGUIListBox *lb = (MashGUIListBox*)m_animationRangeWindow->GetElementByName("AnimRangeLB");
		//if (lb->GetSelectedItemId() > -1)
		//{
		//	std::map<int, sAnimRange>::iterator iter = m_selectedNodeAnimRanges.find(lb->GetSelectedItemId());
		//	if (iter != m_selectedNodeAnimRanges.end())
		//	{
		//		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		//		if (tb->GetTextAsInt() != iter->second.start)
		//			m_selectedNodeAnimRangeUpdate = ANIM_UPDATE_ALL;
		//	}
		//}
	}

	void OnAnimRangeEndFrame(const sGUIEvent &eventData)
	{
		////just checks to see if the data has changed, otherwise there is no need to update
		//MashGUIListBox *lb = (MashGUIListBox*)m_animationRangeWindow->GetElementByName("AnimRangeLB");
		//if (lb->GetSelectedItemId() > -1)
		//{
		//	std::map<int, sAnimRange>::iterator iter = m_selectedNodeAnimRanges.find(lb->GetSelectedItemId());
		//	if (iter != m_selectedNodeAnimRanges.end())
		//	{
		//		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		//		if (tb->GetTextAsInt() != iter->second.end)
		//			m_selectedNodeAnimRangeUpdate = ANIM_UPDATE_ALL;
		//	}
		//}
	}

	void OnAnimRangeName(const sGUIEvent &eventData)
	{
		////just checks to see if the data has changed, otherwise there is no need to update
		//MashGUIListBox *lb = (MashGUIListBox*)m_animationRangeWindow->GetElementByName("AnimRangeLB");
		//if (lb->GetSelectedItemId() > -1)
		//{
		//	std::map<int, sAnimRange>::iterator iter = m_selectedNodeAnimRanges.find(lb->GetSelectedItemId());
		//	if (iter != m_selectedNodeAnimRanges.end())
		//	{
		//		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		//		if (tb->GetText() != iter->second.name)
		//			m_selectedNodeAnimRangeUpdate = ANIM_UPDATE_NAME_ONLY;
		//	}
		//}
	}

	void OnSceneTreeSelection(const sGUIEvent &eventData)
	{
		MashGUITree *sceneTree = (MashGUITree*)eventData.component;
		MashSceneNode *selectedNode = m_device->GetSceneManager()->GetSceneNodeByID(sceneTree->GetItemUserId(sceneTree->GetSelectedItemID()));
		UpdateSelectedNodeSettings(selectedNode);
	}

	void OnCancelLoad(const sGUIEvent &eventData)
	{
		m_loadSettingsWindow->SetRenderEnabled(false);
		m_openFileDialog->CloseDialog();
	}

	void FillNodeData(MashSceneNode *node)
	{
		std::map<MashSceneNode*, sSceneNodeData>::iterator iter = m_loadedNodeData.insert(std::make_pair(node, sSceneNodeData())).first;
		
		//store the original anim buffer to undo changes
		iter->second.originalAnimBuffer = node->GetAnimationBuffer();

		if (iter->second.originalAnimBuffer)
			iter->second.originalAnimBuffer->Grab();

		MashVector3 eulerAngles;
		node->GetLocalTransformState().orientation.ToEulerAngles(eulerAngles);
		iter->second.euler.x = eulerAngles.x;
		iter->second.euler.y = eulerAngles.y;
		iter->second.euler.z = eulerAngles.z;
		
		switch(node->GetNodeType())
		{
		case aNODETYPE_PARTICLE_EMITTER:
			{
				iter->second.particleData.diffuseTextureName = 0;
				iter->second.particleData.meshName = 0;
				break;
			}
		case aNODETYPE_LIGHT:
			{
				m_loadedLights.PushBack((MashLight*)node);
				break;
			}
		}

		MashList<MashSceneNode*>::ConstIterator nodeIter = node->GetChildren().Begin();
		MashList<MashSceneNode*>::ConstIterator nodeIterEnd = node->GetChildren().End();
		for(; nodeIter != nodeIterEnd; ++nodeIter)
		{
			FillNodeData(*nodeIter);
		}
	}

	void OnLoad(const sGUIEvent &eventData)
	{
		MashStringc fullPath;
        ConcatenatePaths(m_openFileDialog->GetSelectedFileDirectory(), m_openFileDialog->GetSelectedFileName(), fullPath);

		if (!fullPath.Empty())
		{
			//m_device->GetSceneManager()->_OnBeginUserInitialize();

			//load any materials previously saved
			{
				MashStringc intermPath;
				GetFileName(m_openFileDialog->GetSelectedFileName(), intermPath);
				intermPath += g_sceneViewerXMLFileName;
				intermPath += ".xml";

				//generate file path name
				ConcatenatePaths(m_openFileDialog->GetSelectedFileDirectory(), intermPath.GetCString(), intermPath);

				//look for possible saved file
				if (m_device->GetFileManager()->DoesFileExist(intermPath.GetCString()))
				{
					//file exists so load all materials within
					MashXMLReader *xmlReader = m_device->GetFileManager()->CreateXMLReader(intermPath.GetCString());
					if (xmlReader && xmlReader->MoveToFirstChild())
					{
						MashStringc path, filename;
						do
						{
							GetFilePath(xmlReader->GetAttributeRaw("path"), path);
							GetFileNameAndExtention(xmlReader->GetAttributeRaw("path"), filename);
							LoadMaterialFile(path, filename, false);

							path.Clear();
							filename.Clear();

						}while(xmlReader->MoveToNextSibling());

						xmlReader->PopChild();
					}

					xmlReader->Destroy();
				}
			}

			sLoadSceneSettings loadSettings;
			loadSettings.createRootNode = ((MashGUICheckBox*)m_loadSettingsWindow->GetElementByName("CreateRootNodeCheckBox"))->IsChecked();
			loadSettings.frameRate = ((MashGUITextBox*)m_loadSettingsWindow->GetElementByName("FrameRateTextBox"))->GetTextAsInt();

			/*
				Important!
				This keeps all vertex and index data around so we can apply materials
				after _OnEndUserInitialize().
			*/
			loadSettings.saveGeometryFlags = aSAVE_MESH_DATA_ALL;//save all mesh data for manipulation

			MashList<MashSceneNode*> rootSceneNodes;
			m_device->GetSceneManager()->LoadSceneFile(fullPath, 
				rootSceneNodes, 
				loadSettings);

			MashList<MashSceneNode*>::Iterator iter = rootSceneNodes.Begin();
			MashList<MashSceneNode*>::Iterator iterEnd = rootSceneNodes.End();
			for(; iter != iterEnd; ++iter)
			{
				m_sceneRoot->AddChild(*iter);
				MashAnimationBuffer *animBuffer = (*iter)->GetAnimationBuffer();
				
				FillNodeData(*iter);
			}

			//m_device->GetSceneManager()->_OnEndUserInitialize();
            m_device->GetSceneManager()->CompileAllMaterials(aMATERIAL_COMPILER_NON_COMPILED);

			UpdateSceneTree();
		}

		m_loadSettingsWindow->SetRenderEnabled(false);
	}

	void CollectSceneNodes(MashSceneNode *root, MashArray<MashSceneNode*> &sceneNodes)
	{
		if (root)
			sceneNodes.PushBack(root);

		MashList<MashSceneNode*>::ConstIterator iter = root->GetChildren().Begin();
		MashList<MashSceneNode*>::ConstIterator iterEnd = root->GetChildren().End();
		for(; iter != iterEnd; ++iter)
		{
			CollectSceneNodes(*iter, sceneNodes);
		}
	}

	void OnSave(const sGUIEvent &eventData)
	{
		MashStringc fullPath;
		ConcatenatePaths(m_openFileDialog->GetSelectedFileDirectory(), m_openFileDialog->GetSelectedFileName(), fullPath);

		if (!fullPath.Empty())
		{
			unsigned int cameraParentLocation = 0;
			unsigned int lightParentLocation = 0;

			if (!((MashGUICheckBox*)m_saveSettingsWindow->GetElementByName("SaveDefaultLightCB"))->IsChecked())
			{
				lightParentLocation = m_defaultLight->GetIndexWithinParent();
				m_defaultLight->Grab();
				m_defaultLight->Detach();
			}

			if (!((MashGUICheckBox*)m_saveSettingsWindow->GetElementByName("SaveDefaultCameraCB"))->IsChecked())
			{
				cameraParentLocation = m_defaultCamera->GetIndexWithinParent();
				m_defaultCamera->Grab();
				m_defaultCamera->Detach();
			}

			if (((MashGUICheckBox*)m_saveSettingsWindow->GetElementByName("SaveSceneViewerDataCB"))->IsChecked())
			{
				MashArray<MashSceneNode*> sceneNodes;
				MashArray<MashStringc> usedMaterials;
				CollectSceneNodes(m_sceneRoot, sceneNodes);

				//search through each node and grab material data
				MashArray<MashSceneNode*>::Iterator iter = sceneNodes.Begin();
				MashArray<MashSceneNode*>::Iterator iterEnd = sceneNodes.End();
				for(; iter != iterEnd; ++iter)
				{
					if ((*iter)->GetNodeType() == aNODETYPE_ENTITY)
					{
						MashEntity *entity = (MashEntity*)(*iter);
						uint32 lodCount = entity->GetLodCount();
						for(uint32 lod = 0; lod < lodCount; ++lod)
						{
							uint32 subEntityCount = entity->GetSubEntityCount(lod);
							for(uint32 se = 0; se < subEntityCount; ++se)
							{
								MashSubEntity *subEntity = entity->GetSubEntity(se, lod);
								if (!usedMaterials.Contains(subEntity->GetMaterial()->GetMaterialName()))
									usedMaterials.PushBack(subEntity->GetMaterial()->GetMaterialName());
							}							
						}
					}
				}

				//write used material paths to file for easy loading next time
				if (!usedMaterials.Empty())
				{
					MashStringc intermPath;
					GetFileName(m_openFileDialog->GetSelectedFileName(), intermPath);
					intermPath += g_sceneViewerXMLFileName;
					intermPath += ".xml";

					//generate file path name
					ConcatenatePaths(m_openFileDialog->GetSelectedFileDirectory(), intermPath.GetCString(), intermPath);
					MashXMLWriter *xmlWriter = m_device->GetFileManager()->CreateXMLWriter(intermPath.GetCString(), "root");

					//get each materials abs path and write to file
					MashStringc finalAbsPath;
					MashArray<MashStringc>::Iterator matIter = usedMaterials.Begin();
					MashArray<MashStringc>::Iterator matIterEnd = usedMaterials.End();
					for(; matIter != matIterEnd; ++matIter)
					{
						std::map<MashStringc, sMaterial>::iterator editorMaterialIter = m_editorMaterials.find(*matIter);
						if (editorMaterialIter != m_editorMaterials.end())
						{
							finalAbsPath.Clear();
							ConcatenatePaths(editorMaterialIter->second.filePath.GetCString(), editorMaterialIter->second.fileName.GetCString(), finalAbsPath);
							xmlWriter->WriteChild("material");
							xmlWriter->WriteAttributeString("path", finalAbsPath.GetCString());
							xmlWriter->PopChild();
						}
					}

					xmlWriter->SaveAndDestroy();
				}
			}

			sSaveSceneSettings saveData;
			m_device->GetSceneManager()->SaveSceneFile(fullPath, m_sceneRoot->GetChildren(), saveData);

			//reattch the default nodes
			if (m_defaultLight->GetParent() == 0)
			{
				m_sceneRoot->AddChildAtLocation(m_defaultLight, lightParentLocation);
				m_defaultLight->Drop();
			}
			if (m_defaultCamera->GetParent() == 0)
			{
				m_sceneRoot->AddChildAtLocation(m_defaultCamera, cameraParentLocation);
				m_defaultCamera->Drop();
			}
		}

		m_saveSettingsWindow->SetRenderEnabled(false);
	}

	void OnSaveCancel(const sGUIEvent &eventData)
	{
		m_saveSettingsWindow->SetRenderEnabled(false);
	}

	void OnSaveSetLoadFilePath(const sGUIEvent &eventData)
	{
		m_fileDialogUsage = DIALOG_USAGE_NONE;
		m_openFileDialog->OpenDialog();
	}

	void OnSetLoadFilePath(const sGUIEvent &eventData)
	{
		m_fileDialogUsage = DIALOG_USAGE_NONE;
		m_openFileDialog->OpenDialog();
	}
    
    void LoadMaterialFile(const MashStringc &filePathOnly, const MashStringc &fileName, bool reloadMaterial)
    {
        MashStringc fullPath;
        ConcatenatePaths(filePathOnly.GetCString(), fileName.GetCString(), fullPath);
        
        //Add the materials file path to the file manager so we can find the effect files.
        m_device->GetFileManager()->AddRootPath(filePathOnly.GetCString());
        
        MashArray<MashMaterial*> materialsLoaded;
        m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile(fullPath.GetCString(), 0, 0, &materialsLoaded, reloadMaterial);
        
        m_device->GetSceneManager()->CompileAllMaterials(aMATERIAL_COMPILER_NON_COMPILED);
        for(unsigned int i = 0; i < materialsLoaded.Size(); ++i)
        {
            std::map<MashStringc, sMaterial>::iterator prevMaterialIter = m_editorMaterials.find(materialsLoaded[i]->GetMaterialName());
            /*
				Delete any new materials from the engine that failed to compile.

                Only delete a material if it hasnt been previously loaded. This keeps material pointers
                valid to be reloaded (and edited) later if they have already been set to a model.
            */
            if ((prevMaterialIter == m_editorMaterials.end()) && !materialsLoaded[i]->IsValid())
            {
                m_device->GetRenderer()->GetMaterialManager()->RemoveMaterial(materialsLoaded[i]);
                materialsLoaded.Erase(materialsLoaded.Begin() + i);
            }
        }
        
        if (!materialsLoaded.Empty())
        {
            for(unsigned int i = 0; i < materialsLoaded.Size(); ++i)
            {
                sMaterial newMaterial;
                newMaterial.material = materialsLoaded[i];
                newMaterial.fileName = fileName;
                newMaterial.filePath = filePathOnly;
                m_editorMaterials[materialsLoaded[i]->GetMaterialName()] = newMaterial;
            }
        }
        
        UpdateMaterialSelectionWindow();
    }

	/*
		Some functions wait to process the file location at a later time, others process some data here
	*/
	void OnFileDialogSelection(const sGUIEvent &eventData)
	{
		switch(m_fileDialogUsage)
		{
		case DIALOG_USAGE_LOAD_SHADOWS:
			{
				MashStringc fullPath;
				ConcatenatePaths(m_openFileDialog->GetSelectedFileDirectory(), m_openFileDialog->GetSelectedFileName(), fullPath);
				m_device->GetSceneManager()->LoadShadowCastersFromFile(fullPath);
				_OnResetShadowCaster();
				break;
			}
		case DIALOG_USAGE_SAVE_SHADOWS:
			{
				MashStringc fullPath;
				ConcatenatePaths(m_openFileDialog->GetSelectedFileDirectory(), m_openFileDialog->GetSelectedFileName(), fullPath);
				m_device->GetSceneManager()->SaveShadowCastersToFile(fullPath);
				break;
			}
		case DIALOG_USAGE_LOAD_ANIM_RANGE:
			{
				MashStringc fullPath;
				ConcatenatePaths(m_openFileDialog->GetSelectedFileDirectory(), m_openFileDialog->GetSelectedFileName(), fullPath);

				MashFileStream *fileStream = m_device->GetFileManager()->CreateFileStream();
				if (fileStream->LoadFile(fullPath.GetCString(), aFILE_IO_TEXT) == aMASH_FAILED)
				{
					//post error
					return;
				}

				MashGUIListBox *lb = (MashGUIListBox*)m_animationRangeWindow->GetElementByName("AnimRangeLB");
				lb->ClearAllItems();
				m_selectedNodeAnimRanges.clear();

				unsigned int strLocation = 0;
				const unsigned int strLen = strlen((const char*)fileStream->GetData());
				MashStringc nameStr, startFrameStr, endFrameStr;

				while(strLocation < strLen)
				{
					nameStr.Clear(); startFrameStr.Clear(); endFrameStr.Clear();

                    scriptreader::ReadNextString((const char*)fileStream->GetData(), strLocation, strLen, nameStr);
					scriptreader::ReadNextString((const char*)fileStream->GetData(), strLocation, strLen, startFrameStr);
					scriptreader::ReadNextString((const char*)fileStream->GetData(), strLocation, strLen, endFrameStr);

					sAnimRange newRange;
					newRange.start = atoi(startFrameStr.GetCString());
					newRange.end = atoi(endFrameStr.GetCString());
					newRange.name = nameStr;

					int newItemId = lb->AddItem(newRange.name, 0);
					m_selectedNodeAnimRanges.insert(std::make_pair(newItemId, newRange));
					lb->SetMute(true);
					lb->SetActiveItem(newItemId);
					lb->SetMute(false);
					
					UpdateAnimationRangeInfoGUI(lb);
				}

				m_selectedNodeAnimRangeUpdate = ANIM_UPDATE_ALL;
				

				break;
			}
		case DIALOG_USAGE_LOAD_MATERIAL:
			{
                LoadMaterialFile(m_openFileDialog->GetSelectedFileDirectory(), m_openFileDialog->GetSelectedFileName(), false);
				break;
			}
		case DIALOG_USAGE_LOAD_MATERIAL_ALL:
			{
				//Add the materials file path to the file manager so we can find the effect files.
				m_device->GetFileManager()->AddRootPath(m_openFileDialog->GetSelectedFileDirectory());

				MashArray<MashFileManager::sFileAttributes> fileDir;
				m_device->GetFileManager()->APIGetDirectoryStructure(m_openFileDialog->GetSelectedFileDirectory(), fileDir);
				unsigned int fileCount = fileDir.Size();
				MashStringc fileExt;
				MashArray<sMaterial> materialsLoaded;
				for(unsigned int i = 0; i < fileCount; ++i)
				{
					if (!((fileDir[i].flags & MashFileManager::aFILE_ATTRIB_DIR) ||
						(fileDir[i].flags & MashFileManager::aFILE_ATTRIB_PARENT_DIR)))
					{
						//its a file
						GetFileExtention(fileDir[i].relativeFilePath.GetCString(), fileExt);
						if (fileExt == "mtl")
						{
							//its a material file
                            MashArray<MashMaterial*> newMaterials;
							m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile(fileDir[i].absoluteFilePath.GetCString(), 0, 0, &newMaterials);
                            
                            for(unsigned int j = 0; j < newMaterials.Size(); ++j)
                            {
                                sMaterial newMaterial;
                                newMaterial.material = newMaterials[j];
                                GetFileNameAndExtention(fileDir[i].relativeFilePath.GetCString(), newMaterial.fileName);
                                newMaterial.filePath = m_openFileDialog->GetSelectedFileDirectory();
                                materialsLoaded.PushBack(newMaterial);
                            }
						}
					}
				}
                
                if (!materialsLoaded.Empty())
                {
                    m_device->GetSceneManager()->CompileAllMaterials(aMATERIAL_COMPILER_NON_COMPILED);
                    for(unsigned int i = 0; i < materialsLoaded.Size(); ++i)
                    {
                        std::map<MashStringc, sMaterial>::iterator prevMaterialIter = m_editorMaterials.find(materialsLoaded[i].material->GetMaterialName());
                        /*
                            Only delete a material if it hasnt been previously loaded. This keeps material pointers
                            valid to be reloaded later if they have already been set to a model.
                        */
                        if ((prevMaterialIter == m_editorMaterials.end()) && !materialsLoaded[i].material->IsValid())
                        {
                            m_device->GetRenderer()->GetMaterialManager()->RemoveMaterial(materialsLoaded[i].material);
                            materialsLoaded.Erase(materialsLoaded.Begin() + i);
                        }
                    }
                    
                    for(unsigned int i = 0; i < materialsLoaded.Size(); ++i)
                    {
                        m_editorMaterials[materialsLoaded[i].material->GetMaterialName()] = materialsLoaded[i];
                    }
                }

				UpdateMaterialSelectionWindow();

				break;
			}
		case DIALOG_USAGE_LOAD_PARTICLE_MESH:
			{
				MashStringc fullPath;
                ConcatenatePaths(m_openFileDialog->GetSelectedFileDirectory(), m_openFileDialog->GetSelectedFileName(), fullPath);

				if (!fullPath.Empty())
				{
					//m_device->GetSceneManager()->_OnBeginUserInitialize();

					sLoadSceneSettings loadSettings;
					loadSettings.createRootNode = false;
					loadSettings.saveGeometryFlags = 0;

					MashList<MashSceneNode*> rootSceneNodes;
					m_device->GetSceneManager()->LoadSceneFile(fullPath, 
						rootSceneNodes, 
						loadSettings);
					
                    m_device->GetSceneManager()->CompileAllMaterials(aMATERIAL_COMPILER_NON_COMPILED);
					//m_device->GetSceneManager()->_OnEndUserInitialize();

					MashEntity *meshEntity = 0;
					//find the first node with a mesh
					MashList<MashSceneNode*>::Iterator iter = rootSceneNodes.Begin();
					MashList<MashSceneNode*>::Iterator iterEnd = rootSceneNodes.End();
					for(; iter != iterEnd; ++iter)
					{
						meshEntity = (MashEntity*)(*iter)->GetChildByType(aNODETYPE_ENTITY);
						 if (meshEntity)
							 break;
					}

					if (meshEntity)
					{
						if (((MashParticleSystem*)m_selectedNode)->SetModel(meshEntity->GetModel()) == aMASH_OK)
						{
							std::map<MashSceneNode*, sSceneNodeData>::iterator iter = m_loadedNodeData.find(m_selectedNode);
							if (iter->second.particleData.meshName)
								MASH_FREE((void*)iter->second.particleData.meshName);

							iter->second.particleData.meshName = MASH_ALLOC_T_COMMON(char, strlen(m_openFileDialog->GetSelectedFileName()) + 1);
							strcpy(iter->second.particleData.meshName, m_openFileDialog->GetSelectedFileName());

							MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("ParticleSettingsWindow");
							((MashGUITextBox*)windowElement->GetElementByName("ParticleMeshTB"))->SetText(m_openFileDialog->GetSelectedFileName());
						}

						/*
							TODO : This should be kept alive so other instances reference the same mesh.
							When the scene is saved only the mesh will be kept....so.....can we keep this
							node just hanging? Will it be deleted properly?
						*/
						//meshEntity
					}
				}
				break;
			}
		case DIALOG_USAGE_LOAD_PARTICLE_TEXTURE:
			{
				MashStringc fullPath;
				ConcatenatePaths(m_openFileDialog->GetSelectedFileDirectory(), m_openFileDialog->GetSelectedFileName(), fullPath);

				if (!fullPath.Empty())
				{
					MashTexture *texture = m_device->GetRenderer()->GetTexture(fullPath);
					if (texture)
					{
						MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("ParticleSettingsWindow");
						MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("ParticleTextureWrapModeLB");

						sSamplerState samplerState;
						samplerState.type = aSAMPLER2D;
						samplerState.filter = aFILTER_MIN_MAG_MIP_LINEAR;
						samplerState.uMode = (eTEXTURE_ADDRESS)lb->GetItemUserValue(lb->GetSelectedItemId());
						samplerState.vMode = samplerState.uMode;
						MashTextureState *textureState = m_device->GetRenderer()->AddSamplerState(samplerState);

						if (((MashParticleSystem*)m_selectedNode)->SetDiffuseTexture(texture, textureState) == aMASH_OK)
						{
							std::map<MashSceneNode*, sSceneNodeData>::iterator iter = m_loadedNodeData.find(m_selectedNode);
							if (iter->second.particleData.diffuseTextureName)
								MASH_FREE((void*)iter->second.particleData.diffuseTextureName);

							iter->second.particleData.diffuseTextureName = MASH_ALLOC_T_COMMON(char, strlen(m_openFileDialog->GetSelectedFileName()) + 1);
							strcpy(iter->second.particleData.diffuseTextureName, m_openFileDialog->GetSelectedFileName());
							((MashGUITextBox*)windowElement->GetElementByName("ParticleTextureTB"))->SetText(m_openFileDialog->GetSelectedFileName());
						}
					}
				}
				break;
			}
		}
	}

	void OnMenuBarSelection(const sGUIEvent &eventData)
	{
		MashGUIMenuBar *mb = (MashGUIMenuBar*)eventData.component;
		int selectedMenuItem = mb->GetItemUserValue(mb->GetSelectedItemId());

		MashGUIPopupMenu *popup = mb->GetSelectedSubMenu();
		int popupValue = popup->GetItemUserValue(popup->GetSelectedItemId());

		switch(selectedMenuItem)
		{
		case 0:
			{
				switch(popupValue)
				{
				case 0://save
					{
						//m_openFileDialog->ClearLastFilepath();
						m_saveSettingsWindow->SetRenderEnabled(true);
						break;
					}
				case 1://load
					{
						//m_fileDialogUsage = DIALOG_USAGE_LOAD;
						//m_openFileDialog->ClearLastFilepath();
						m_loadSettingsWindow->SetRenderEnabled(true);
						break;
					}
				case 2://quit
					{
						m_quit = true;
						break;
					}
				//case 3://save standard materials only
				//	{
				//		//m_fileDialogUsage = DIALOG_USAGE_LOAD;
				//		m_openFileDialog->ClearLastFilepath();
				//		m_fileDialogUsage = DIALOG_USAGE_SAVE_STANDARD_MATERIALS;
				//		m_openFileDialog->OpenDialog();
				//		break;
				//	}
				case 4://clear scene
					{
						UpdateSelectedNodeSettings(0);

						std::map<MashSceneNode*, sSceneNodeData>::iterator iter = m_loadedNodeData.begin();
						std::map<MashSceneNode*, sSceneNodeData>::iterator iterEnd = m_loadedNodeData.end();
						for(; iter != iterEnd; ++iter)
						{
							//dont delete the default nodes
							if (iter->first != m_sceneRoot && iter->first != m_defaultCamera && iter->first != m_defaultLight)
								m_device->GetSceneManager()->RemoveSceneNode(iter->first);
						}

						UpdateSceneTree();
						break;
					}
				case 5://save shadows
					{
						m_fileDialogUsage = DIALOG_USAGE_SAVE_SHADOWS;
						//m_openFileDialog->ClearLastFilepath();
						m_openFileDialog->OpenDialog();
						break;
					}
				case 6://load shadows
					{
						m_fileDialogUsage = DIALOG_USAGE_LOAD_SHADOWS;
						//m_openFileDialog->ClearLastFilepath();
						m_openFileDialog->OpenDialog();
						break;
					}
				}
				break;
			}
		case 1:
			{
				switch(popupValue)
				{
				case 0:
					{
						m_debugLogWindow->SetRenderEnabled(true);
						break;
					}
				case 1:
					{
                        m_debugText->SetRenderEnabled(!m_debugText->GetRenderEnabled());
						break;
					}
				case 2:
					{
                        m_fpsText->SetRenderEnabled(!m_fpsText->GetRenderEnabled());
						break;
					}
                case 3:
					{
						m_sceneTreeWindow->SetRenderEnabled(!m_sceneTreeWindow->GetRenderEnabled());
						break;
					}
				}
				break;
			}
		case 2:
			{
				switch(popupValue)
				{
				case 0:
					{
						m_particleCreationWindow->SetRenderEnabled(true);
						m_device->GetGUIManager()->SetGlobalMute(true);
						((MashGUIListBox*)m_particleCreationWindow->GetElementByName("ParticleCreationTypeLB"))->SetActiveItem(0);
						((MashGUIListBox*)m_particleCreationWindow->GetElementByName("ParticleCreationLightTypeLB"))->SetActiveItem(0);
						m_device->GetGUIManager()->SetGlobalMute(false);
						break;
					}
				case 1:
					{
						MashLight *light = m_device->GetSceneManager()->AddLight(m_sceneRoot, "Light", aLIGHT_DIRECTIONAL, aLIGHT_RENDERER_FORWARD, false);
						FillNodeData(light);
						AddItemToSceneTree(light);
						break;
					}
				case 2:
					{
						MashCamera *camera = m_device->GetSceneManager()->AddCamera(m_sceneRoot, "Camera");
						FillNodeData(camera);
						AddItemToSceneTree(camera);
						break;
					}
				}
				break;
			}
		case 3:
			{
				switch(popupValue)
				{
				case 0:
					{
						MashGUIListBox *lightTypeLb = (MashGUIListBox*)m_preferredLightingWindow->GetElementByName("PreferredLightingLB");
						lightTypeLb->SetActiveItemByUserValue(m_device->GetSceneManager()->GetPreferredLightingMode());
						m_preferredLightingWindow->SetRenderEnabled(true);

						break;
					}
                    case 1:
                    {
                        UpdateMaterialSelectionWindow();
                        m_materialSelectWindow->SetRenderEnabled(true);
                        break;
                    }
					case 2:
                    {
                        m_shadowCasterWindow->SetRenderEnabled(true);
                        break;
                    }
				}

				break;
			}
		}
	}

	void OnNodeAnimWeight(const sGUIEvent &eventData)
	{
		MashAnimationMixer *newMixer = m_selectedNode->GetAnimationMixer();
		if (newMixer)
		{
			MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
			MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");

			if (lb->GetSelectedItemId() > -1)
				newMixer->SetWeight(lb->GetItemText(lb->GetSelectedItemId()).GetCString(), ((MashGUITextBox*)eventData.component)->GetTextAsFloat());
		}
	}

	void OnNodeAnimTrack(const sGUIEvent &eventData)
	{
		MashAnimationMixer *newMixer = m_selectedNode->GetAnimationMixer();
		if (newMixer)
		{
			MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
			MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");

			if (lb->GetSelectedItemId() > -1)
			{
				StopNodeAnimation();
				newMixer->SetTrack(lb->GetItemText(lb->GetSelectedItemId()).GetCString(), ((MashGUITextBox*)eventData.component)->GetTextAsInt());
			}
		}
	}

	void OnNodeAnimFPS(const sGUIEvent &eventData)
	{
		MashAnimationMixer *newMixer = m_selectedNode->GetAnimationMixer();
		if (newMixer)
		{
			MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
			MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");

			if (lb->GetSelectedItemId() > -1)
			{
				StopNodeAnimation();
				newMixer->SetFrameRate(((MashGUITextBox*)eventData.component)->GetTextAsInt());
			}
		}
	}

	void OnNodeAnimFrame(const sGUIEvent &eventData)
	{
		MashAnimationMixer *newMixer = m_selectedNode->GetAnimationMixer();
		if (newMixer)
		{
			MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
			MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");

			MashStringc animName = lb->GetItemText(lb->GetSelectedItemId());
			if (lb->GetSelectedItemId() > -1)
			{
				MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
				int frame = tb->GetTextAsInt();
				tb->SetMute(true);
				if (frame < 0)
				{
					frame = 0;
					tb->SetTextInt(frame);
				}
				else if (frame > newMixer->GetFrameLength(animName.GetCString()))
				{
					frame = newMixer->GetFrameLength(animName.GetCString());
					tb->SetTextInt(frame);
				}
				tb->SetMute(false);
				newMixer->SetFrame(animName.GetCString(), frame);
			}
		}
	}

	void OnNodeAnimSpeed(const sGUIEvent &eventData)
	{
		MashAnimationMixer *newMixer = m_selectedNode->GetAnimationMixer();
		if (newMixer)
		{
			MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
			MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");

			if (lb->GetSelectedItemId() > -1)
			{
				MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
				float val = tb->GetTextAsFloat();
				tb->SetMute(true);
				if (val < 0.0f)
				{
					val = 0.0f;
					tb->SetTextFloat(val);
				}
				tb->SetMute(false);

				newMixer->SetSpeed(lb->GetItemText(lb->GetSelectedItemId()).GetCString(), val);
			}
		}
	}

	void OnNodeAnimTransitionTime(const sGUIEvent &eventData)
	{
		/*MashAnimationMixer *newMixer = m_selectedNode->GetAnimationMixer();
		if (newMixer)
		{
			MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
			MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");

			if ((lb->GetSelectedItemId() > -1) && newMixer->IsPlaying())
			{
				newMixer->SetTransitionTime(((MashGUITextBox*)eventData.component)->GetTextAsInt());
			}
		}*/
	}

	void OnNodeAnimTransition(const sGUIEvent &eventData)
	{
		MashAnimationMixer *mixer = m_selectedNode->GetAnimationMixer();
		if (mixer && (m_playingAnimtionIndex > -1))
		{
			MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
			MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");
			MashGUITextBox *tb = (MashGUITextBox*)m_viewerRoot->GetElementByName("NodeAnimTransitionTB");
			float transitionTime = tb->GetTextAsFloat();

			if (((MashGUIButton*)eventData.component)->IsButtonDown())
			{
				if (lb->GetSelectedItemId() > -1)
					mixer->Transition(lb->GetItemText(lb->GetSelectedItemId()).GetCString(), transitionTime);
			}
			else
			{
				mixer->Transition(lb->GetItemText(m_playingAnimtionIndex).GetCString(), transitionTime);
			}
		}
	}

	void OnNodeAnimWrapMode(const sGUIEvent &eventData)
	{
		MashAnimationMixer *newMixer = m_selectedNode->GetAnimationMixer();
		if (newMixer)
		{
			MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
			MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");

			if (lb->GetSelectedItemId() > -1)
			{
				eANIMATION_WRAP_MODE wrapMode;
				MashGUIListBox *wrapLb = (MashGUIListBox*)eventData.component;
				switch(wrapLb->GetItemUserValue(wrapLb->GetSelectedItemId()))
				{
				case 0:
					wrapMode = aWRAP_PLAYONCE;
					break;
				case 1:
					wrapMode = aWRAP_CLAMP;
					break;
				case 2:
					wrapMode = aWRAP_LOOP;
					break;
				case 3:
					wrapMode = aWRAP_BOUNCE;
					break;
				}

				StopNodeAnimation();
				newMixer->SetWrapMode(lb->GetItemText(lb->GetSelectedItemId()).GetCString(), wrapMode);
			}
		}
	}

	void OnNodeAnimBlendMode(const sGUIEvent &eventData)
	{
		MashAnimationMixer *newMixer = m_selectedNode->GetAnimationMixer();
		if (newMixer)
		{
			MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("NodeCommonWindow");
			MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("NodeAnimLB");

			if (lb->GetSelectedItemId() > -1)
			{
				eANIMATION_BLEND_MODE blendMode;
				MashGUIListBox *blendLb = (MashGUIListBox*)eventData.component;
				switch(blendLb->GetItemUserValue(blendLb->GetSelectedItemId()))
				{
				case 0:
					{
						blendMode = aBLEND_BLEND;
						newMixer->SetWeight(lb->GetItemText(lb->GetSelectedItemId()).GetCString(), 0.0f);
						break;
					}
				case 1:
					{
						blendMode = aBLEND_ADDITIVE;
						newMixer->SetWeight(lb->GetItemText(lb->GetSelectedItemId()).GetCString(), 1.0f);
						break;
					}
				}

				StopNodeAnimation();
				newMixer->SetBlendMode(lb->GetItemText(lb->GetSelectedItemId()).GetCString(), blendMode);
			}
		}
	}

	void OnNodeIDChange(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		m_selectedNode->SetUserID(tb->GetTextAsInt());
	}

	void OnEditNodeAnimation(const sGUIEvent &eventData)
	{
		UpdateAnimationEditWindow(m_selectedNode);
		m_animationRangeWindow->SetRenderEnabled(true);
		m_animationRangeWindow->SendToFront();
	}

	void OnNodeScaleZ(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashVector3 vec = m_selectedNode->GetLocalTransformState().scale;
		vec.z = tb->GetTextAsFloat();
		m_selectedNode->SetScale(vec, true);
	}

	void OnNodeScaleY(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashVector3 vec = m_selectedNode->GetLocalTransformState().scale;
		vec.y = tb->GetTextAsFloat();
		m_selectedNode->SetScale(vec, true);
	}

	void OnNodeScaleX(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashVector3 vec = m_selectedNode->GetLocalTransformState().scale;
		vec.x = tb->GetTextAsFloat();
		m_selectedNode->SetScale(vec, true);
	}

	void OnNodeRotZ(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		/*MashVector3 vec;
		m_selectedNode->GetLocalTransformState().orientation.ToEulerAngles(vec);
		vec.z = DegsToRads(tb->GetTextAsFloat());*/
		std::map<MashSceneNode*, sSceneNodeData>::iterator iter = m_loadedNodeData.find(m_selectedNode);
		iter->second.euler.z = math::DegsToRads(tb->GetTextAsFloat());
		MashQuaternion quat;
		quat.SetEuler(iter->second.euler);
		m_selectedNode->SetOrientation(quat, true);
	}

	void OnNodeRotY(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		//MashVector3 vec;
		//m_selectedNode->GetLocalTransformState().orientation.ToEulerAngles(vec);
		std::map<MashSceneNode*, sSceneNodeData>::iterator iter = m_loadedNodeData.find(m_selectedNode);
		iter->second.euler.y = math::DegsToRads(tb->GetTextAsFloat());
		MashQuaternion quat;
		quat.SetEuler(iter->second.euler);
		m_selectedNode->SetOrientation(quat, true);
	}

	void OnNodeRotX(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		/*MashVector3 vec;
		m_selectedNode->GetLocalTransformState().orientation.ToEulerAngles(vec);
		vec.x = DegsToRads(tb->GetTextAsFloat());*/
		std::map<MashSceneNode*, sSceneNodeData>::iterator iter = m_loadedNodeData.find(m_selectedNode);
		iter->second.euler.x = math::DegsToRads(tb->GetTextAsFloat());
		MashQuaternion quat;
		quat.SetEuler(iter->second.euler);
		m_selectedNode->SetOrientation(quat, true);
	}

	void OnNodePosZ(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashVector3 vec = m_selectedNode->GetLocalTransformState().translation;
		vec.z = tb->GetTextAsFloat();
		m_selectedNode->SetPosition(vec, true);
	}

	void OnNodePosY(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashVector3 vec = m_selectedNode->GetLocalTransformState().translation;
		vec.y = tb->GetTextAsFloat();
		m_selectedNode->SetPosition(vec, true);
	}

	void OnNodePosX(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashVector3 vec = m_selectedNode->GetLocalTransformState().translation;
		vec.x = tb->GetTextAsFloat();
		m_selectedNode->SetPosition(vec, true);
	}

	void OnNodeName(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		if (tb->GetText().Empty())
		{
			tb->SetMute(true);
			tb->SetText("NoName");
			tb->SetMute(false);
		}

		m_selectedNode->SetNodeName(tb->GetText());

		MashGUITree *sceneTree = (MashGUITree*)m_sceneTreeWindow->GetElementByName("SceneTree");
		std::map<MashSceneNode*, sSceneNodeData>::iterator nodeIter = m_loadedNodeData.find(m_selectedNode);
		sceneTree->SetItemText(nodeIter->second.treeId, m_selectedNode->GetNodeName());
	}

	void OnParticleSoftScale(const sGUIEvent &eventData)
	{
		((MashParticleSystem*)m_selectedNode)->SetSoftParticleScale(((MashGUITextBox*)eventData.component)->GetTextAsFloat());
	}

	void OnParticleStartTime(const sGUIEvent &eventData)
	{
		((MashParticleSystem*)m_selectedNode)->SetStartTime(((MashGUITextBox*)eventData.component)->GetTextAsInt());
	}

	void OnParticleIsPlaying(const sGUIEvent &eventData)
	{
		MashGUICheckBox *cb = (MashGUICheckBox*)eventData.component;
		if (cb->IsChecked())
			((MashParticleSystem*)m_selectedNode)->PlayEmitter();
		else
			((MashParticleSystem*)m_selectedNode)->StopEmitter();
	}

	void OnParticleMinStartColA(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->minStartColour;
		col.a = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMinStartColour(col);
	}

	void OnParticleMinStartColB(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->minStartColour;
		col.b = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMinStartColour(col);
	}

	void OnParticleMinStartColG(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->minStartColour;
		col.g = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMinStartColour(col);
	}

	void OnParticleMinStartColR(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->minStartColour;
		col.r = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMinStartColour(col);
	}

	void OnParticleMaxStartColA(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->maxStartColour;
		col.a = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMaxStartColour(col);
	}

	void OnParticleMaxStartColB(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->maxStartColour;
		col.b = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMaxStartColour(col);
	}

	void OnParticleMaxStartColG(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->maxStartColour;
		col.g = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMaxStartColour(col);
	}

	void OnParticleMaxStartColR(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->maxStartColour;
		col.r = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMaxStartColour(col);
	}

	void OnParticleMinEndColA(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->minEndColour;
		col.a = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMinEndColour(col);
	}

	void OnParticleMinEndColB(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->minEndColour;
		col.b = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMinEndColour(col);
	}

	void OnParticleMinEndColG(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->minEndColour;
		col.g = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMinEndColour(col);
	}

	void OnParticleMinEndColR(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->minEndColour;
		col.r = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMinEndColour(col);
	}

	void OnParticleMaxEndColA(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->maxEndColour;
		col.a = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMaxEndColour(col);
	}

	void OnParticleMaxEndColB(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->maxEndColour;
		col.b = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMaxEndColour(col);
	}

	void OnParticleMaxEndColG(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->maxEndColour;
		col.g = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMaxEndColour(col);
	}

	void OnParticleMaxEndColR(const sGUIEvent &eventData)
	{
		sMashColour4 col = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->maxEndColour;
		col.r = math::IntColourToFloat(((MashGUITextBox*)eventData.component)->GetTextAsInt());
		((MashParticleSystem*)m_selectedNode)->SetMaxEndColour(col);
	}

	void OnParticleMinVelZ(const sGUIEvent &eventData)
	{
		MashVector3 vel = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->minVelocity;
		vel.z = ((MashGUITextBox*)eventData.component)->GetTextAsFloat();
		((MashParticleSystem*)m_selectedNode)->SetMinStartVelocity(vel);
	}

	void OnParticleMinVelY(const sGUIEvent &eventData)
	{
		MashVector3 vel = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->minVelocity;
		vel.y = ((MashGUITextBox*)eventData.component)->GetTextAsFloat();
		((MashParticleSystem*)m_selectedNode)->SetMinStartVelocity(vel);
	}

	void OnParticleMinVelX(const sGUIEvent &eventData)
	{
		MashVector3 vel = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->minVelocity;
		vel.x = ((MashGUITextBox*)eventData.component)->GetTextAsFloat();
		((MashParticleSystem*)m_selectedNode)->SetMinStartVelocity(vel);
	}

	void OnParticleMaxVelZ(const sGUIEvent &eventData)
	{
		MashVector3 vel = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->maxVelocity;
		vel.z = ((MashGUITextBox*)eventData.component)->GetTextAsFloat();
		((MashParticleSystem*)m_selectedNode)->SetMaxStartVelocity(vel);
	}

	void OnParticleMaxVelY(const sGUIEvent &eventData)
	{
		MashVector3 vel = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->maxVelocity;
		vel.y = ((MashGUITextBox*)eventData.component)->GetTextAsFloat();
		((MashParticleSystem*)m_selectedNode)->SetMaxStartVelocity(vel);
	}

	void OnParticleMaxVelX(const sGUIEvent &eventData)
	{
		MashVector3 vel = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->maxVelocity;
		vel.x = ((MashGUITextBox*)eventData.component)->GetTextAsFloat();
		((MashParticleSystem*)m_selectedNode)->SetMaxStartVelocity(vel);
	}

	void OnParticleMaxCount(const sGUIEvent &eventData)
	{
		((MashParticleSystem*)m_selectedNode)->SetMaxParticleCount(((MashGUITextBox*)eventData.component)->GetTextAsInt());
	}

	void OnParticlePerSecond(const sGUIEvent &eventData)
	{
		((MashParticleSystem*)m_selectedNode)->SetParticlesPerSecond(((MashGUITextBox*)eventData.component)->GetTextAsInt());
	}

	void OnParticleMinStartSize(const sGUIEvent &eventData)
	{
		((MashParticleSystem*)m_selectedNode)->SetMinStartSize(((MashGUITextBox*)eventData.component)->GetTextAsFloat());
	}

	void OnParticleMaxStartSize(const sGUIEvent &eventData)
	{
		((MashParticleSystem*)m_selectedNode)->SetMaxStartSize(((MashGUITextBox*)eventData.component)->GetTextAsFloat());
	}

	void OnParticleMinEndSize(const sGUIEvent &eventData)
	{
		((MashParticleSystem*)m_selectedNode)->SetMinEndSize(((MashGUITextBox*)eventData.component)->GetTextAsFloat());
	}

	void OnParticleMaxEndSize(const sGUIEvent &eventData)
	{
		((MashParticleSystem*)m_selectedNode)->SetMaxEndSize(((MashGUITextBox*)eventData.component)->GetTextAsFloat());
	}

	void OnParticleMinRotate(const sGUIEvent &eventData)
	{
		((MashParticleSystem*)m_selectedNode)->SetMinRotateSpeed(((MashGUITextBox*)eventData.component)->GetTextAsFloat());
	}

	void OnParticleMaxRotate(const sGUIEvent &eventData)
	{
		((MashParticleSystem*)m_selectedNode)->SetMaxRotateSpeed(((MashGUITextBox*)eventData.component)->GetTextAsFloat());
	}

	void OnParticleMinDuration(const sGUIEvent &eventData)
	{
		((MashParticleSystem*)m_selectedNode)->SetMinDuration(((MashGUITextBox*)eventData.component)->GetTextAsFloat());
	}

	void OnParticleMaxDuration(const sGUIEvent &eventData)
	{
		((MashParticleSystem*)m_selectedNode)->SetMaxDuration(((MashGUITextBox*)eventData.component)->GetTextAsFloat());
	}

	void OnParticleGravityX(const sGUIEvent &eventData)
	{
		MashVector3 gravity = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->gravity;
		gravity.x = ((MashGUITextBox*)eventData.component)->GetTextAsFloat();
		((MashParticleSystem*)m_selectedNode)->SetGravity(gravity);
	}

	void OnParticleGravityY(const sGUIEvent &eventData)
	{
		MashVector3 gravity = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->gravity;
		gravity.y = ((MashGUITextBox*)eventData.component)->GetTextAsFloat();
		((MashParticleSystem*)m_selectedNode)->SetGravity(gravity);
	}

	void OnParticleGravityZ(const sGUIEvent &eventData)
	{
		MashVector3 gravity = ((MashParticleSystem*)m_selectedNode)->GetParticleSettings()->gravity;
		gravity.z = ((MashGUITextBox*)eventData.component)->GetTextAsFloat();
		((MashParticleSystem*)m_selectedNode)->SetGravity(gravity);
	}

	void OnParticleTextureLoad(const sGUIEvent &eventData)
	{
		//m_openFileDialog->ClearLastFilepath();
		m_openFileDialog->OpenDialog();
		m_fileDialogUsage = DIALOG_USAGE_LOAD_PARTICLE_TEXTURE;
	}

	void OnParticleMeshLoad(const sGUIEvent &eventData)
	{
		//m_openFileDialog->ClearLastFilepath();
		m_openFileDialog->OpenDialog();
		m_fileDialogUsage = DIALOG_USAGE_LOAD_PARTICLE_MESH;
	}

	void OnLightIsEnabled(const sGUIEvent &eventData)
	{
		MashGUICheckBox *cb = (MashGUICheckBox*)eventData.component;
		MashLight *light = ((MashLight*)m_selectedNode);
		light->SetEnableLight(cb->IsChecked());
	}

	void OnLightIsMain(const sGUIEvent &eventData)
	{
		MashGUICheckBox *cb = (MashGUICheckBox*)eventData.component;
		MashLight *light = ((MashLight*)m_selectedNode);
        
        light->SetLightRendererType(light->IsForwardRenderedLight()?aLIGHT_RENDERER_FORWARD:aLIGHT_RENDERER_DEFERRED, cb->IsChecked());
        
		if (!cb->IsChecked())
		{

			/*
				If this is the only light then it will always be the main light.
				This just makes sure the cb reflects that.
			*/
			cb->SetMute(true);
			cb->SetChecked(light->IsMainForwardRenderedLight());
			cb->SetMute(false);
		}
	}

	void OnLightDiffuseR(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		sMashColour4 col = lightNode->GetLightData()->diffuse;
		col.r = math::IntColourToFloat(tb->GetTextAsInt());
		lightNode->SetDiffuse(col);
	}

	void OnLightDiffuseG(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		sMashColour4 col = lightNode->GetLightData()->diffuse;
		col.g = math::IntColourToFloat(tb->GetTextAsInt());
		lightNode->SetDiffuse(col);
	}

	void OnLightDiffuseB(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		sMashColour4 col = lightNode->GetLightData()->diffuse;
		col.b = math::IntColourToFloat(tb->GetTextAsInt());
		lightNode->SetDiffuse(col);
	}

	void OnLightAmbientR(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		sMashColour4 col = lightNode->GetLightData()->ambient;
		col.r = math::IntColourToFloat(tb->GetTextAsInt());
		lightNode->SetAmbient(col);
	}

	void OnLightAmbientG(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		sMashColour4 col = lightNode->GetLightData()->ambient;
		col.g = math::IntColourToFloat(tb->GetTextAsInt());
		lightNode->SetAmbient(col);
	}

	void OnLightAmbientB(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		sMashColour4 col = lightNode->GetLightData()->ambient;
		col.b = math::IntColourToFloat(tb->GetTextAsInt());
		lightNode->SetAmbient(col);
	}

	void OnLightSpecularR(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		sMashColour4 col = lightNode->GetLightData()->specular;
		col.r = math::IntColourToFloat(tb->GetTextAsInt());
		lightNode->SetSpecular(col);
	}

	void OnLightSpecularG(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		sMashColour4 col = lightNode->GetLightData()->specular;
		col.g = math::IntColourToFloat(tb->GetTextAsInt());
		lightNode->SetSpecular(col);
	}

	void OnLightSpecularB(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		sMashColour4 col = lightNode->GetLightData()->specular;
		col.b = math::IntColourToFloat(tb->GetTextAsInt());
		lightNode->SetSpecular(col);
	}

	void OnLightRange(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		lightNode->SetRange(tb->GetTextAsFloat());
	}

	void OnLightFalloff(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		lightNode->SetFalloff(tb->GetTextAsFloat());
	}

	void OnLightInnerCone(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		lightNode->SetInnerCone(math::DegsToRads(tb->GetTextAsFloat()));
	}

	void OnLightOuterCone(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		lightNode->SetOuterCone(math::DegsToRads(tb->GetTextAsFloat()));
	}

	void OnLightShadows(const sGUIEvent &eventData)
	{
		MashGUICheckBox *cb = (MashGUICheckBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		lightNode->SetShadowsEnabled(cb->IsChecked());
	}

	void OnLightAtten0(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		MashVector3 atten = lightNode->GetLightData()->atten;
		atten.x = tb->GetTextAsFloat();
		lightNode->SetAttenuation(atten.x, atten.y, atten.z);
	}

	void OnLightAtten1(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		MashVector3 atten = lightNode->GetLightData()->atten;
		atten.y = tb->GetTextAsFloat();
		lightNode->SetAttenuation(atten.x, atten.y, atten.z);
	}

	void OnLightAtten2(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		MashVector3 atten = lightNode->GetLightData()->atten;
		atten.z = tb->GetTextAsFloat();
		lightNode->SetAttenuation(atten.x, atten.y, atten.z);
	}

	void OnForwardRenderedLight(const sGUIEvent &eventData)
	{
		MashGUICheckBox *cb = (MashGUICheckBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		lightNode->SetLightRendererType(cb->IsChecked()?aLIGHT_RENDERER_FORWARD:aLIGHT_RENDERER_DEFERRED, lightNode->IsMainForwardRenderedLight());
	}

	void OnLightType(const sGUIEvent &eventData)
	{
		MashGUIListBox *lb = (MashGUIListBox*)eventData.component;
		MashLight *lightNode = (MashLight*)m_selectedNode;
		sMashLight lightData = *lightNode->GetLightData();
		int itemValue = lb->GetItemUserValue(lb->GetSelectedItemId());
		switch(itemValue)
		{
		case 0://directional
			{
				lightNode->SetLightType(aLIGHT_DIRECTIONAL);
				/*
					Set the default settings so the light just works when changed.
					This is more important for spot and point lights where things
					like attenuation are used.
				*/
				lightNode->SetDefaultDirectionalLightSettings(lightData.direction);
				break;
			}
		case 1://spot
			{
				lightNode->SetLightType(aLIGHT_SPOT);
				lightNode->SetDefaultSpotLightSettings(lightData.direction);
				break;
			}
		case 2://point
			{
				lightNode->SetLightType(aLIGHT_POINT);
				lightNode->SetDefaultPointLightSettings();
				break;
			}
		};

		//copy over some old values because the default settings will override these
		lightNode->SetDiffuse(lightData.diffuse);
		lightNode->SetSpecular(lightData.specular);
		lightNode->SetAmbient(lightData.ambient);

		MashGUIView *elementPropertiesView = (MashGUIView*)m_viewerRoot->GetElementByName("Properties");
		MashGUIWindow *windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("LightSettingsWindow");
		UpdateLightGUI(lightNode, windowElement);
	}

	void OnCameraIsMain(const sGUIEvent &eventData)
	{
		MashCamera *cameraNode = (MashCamera*)m_selectedNode;
		MashGUICheckBox *cb = (MashGUICheckBox*)eventData.component;
		if (cb->IsChecked())
		{
			m_device->GetSceneManager()->SetActiveCamera(cameraNode);
		}
		else
		{
			
			/*
				This stops the user from disabling the current active camera.
				Make sure we mute to stop recursion.
			*/
			cb->SetMute(true);
			cb->SetChecked(true);
			cb->SetMute(false);
		}

		if (!m_defaultCamera->IsActiveCamera())
			m_freeCameraController->SetInputState(false);
		else
			m_freeCameraController->SetInputState(true);
	}

	void OnCameraZFar(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashCamera *cameraNode = (MashCamera*)m_selectedNode;
		cameraNode->SetZFar(tb->GetTextAsFloat());
	}

	void OnCameraZNear(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashCamera *cameraNode = (MashCamera*)m_selectedNode;
		cameraNode->SetZNear(tb->GetTextAsFloat());
	}

	void OnCameraAspect(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashCamera *cameraNode = (MashCamera*)m_selectedNode;
		cameraNode->SetAspect(cameraNode->GetAutoAspectEnabled(), tb->GetTextAsFloat());
	}

	void OnCameraFOV(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashCamera *cameraNode = (MashCamera*)m_selectedNode;
		cameraNode->SetFOV(math::DegsToRads(tb->GetTextAsFloat()));
	}

	void OnCameraAutoAspect(const sGUIEvent &eventData)
	{
		MashGUICheckBox *cb = (MashGUICheckBox*)eventData.component;
		MashCamera *cameraNode = (MashCamera*)m_selectedNode;
		cameraNode->SetAspect(cb->IsChecked(), cameraNode->GetAspect());
	}

	void OnEntityLodDistance(const sGUIEvent &eventData)
	{
		MashGUITextBox *tb = (MashGUITextBox*)eventData.component;
		MashEntity *entityNode = (MashEntity*)m_selectedNode;
		entityNode->SetLodDistance(m_entityData.selectedLod, tb->GetTextAsInt());
	}

	void OnSubEntityEnabled(const sGUIEvent &eventData)
	{
		MashGUICheckBox *cb = (MashGUICheckBox*)eventData.component;
		MashEntity *entityNode = (MashEntity*)m_selectedNode;
		MashSubEntity *subEntity = entityNode->GetSubEntity(m_entityData.selectedMesh, m_entityData.selectedLod);
		if (subEntity)
			subEntity->SetIsActive(cb->IsChecked());
	}

	void OnApplyMaterialAll(const sGUIEvent &eventData)
	{
		m_materialSelectWindow->SetRenderEnabled(true);
		UpdateMaterialSelectionWindow();

		m_applyMaterialType = APPLY_MATERIAL_ALL;
	}

	void OnApplyMaterial(const sGUIEvent &eventData)
	{
		m_materialSelectWindow->SetRenderEnabled(true);
		UpdateMaterialSelectionWindow();

		m_applyMaterialType = APPLY_MATERIAL_SINGLE;
	}

	void OnConfirmYes(const sGUIEvent &eventData)
	{
		_OnDelete();
		MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("ConfirmWindow");
		windowElement->SetRenderEnabled(false);
	}

	void OnConfirmNo(const sGUIEvent &eventData)
	{
		MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("ConfirmWindow");
		windowElement->SetRenderEnabled(false);
	}

	void OnTriColliderCreate(const sGUIEvent &eventData)
	{
		MashGUIListBox *lb = (MashGUIListBox*)m_triColliderWindow->GetElementByName("TriColliderLodLB");
		unsigned int selectedLod = lb->GetItemUserValue(lb->GetSelectedItemId());

		lb = (MashGUIListBox*)m_triColliderWindow->GetElementByName("TriColliderTypeLB");
		eTRIANGLE_COLLIDER_TYPE colliderType = (eTRIANGLE_COLLIDER_TYPE)lb->GetItemUserValue(lb->GetSelectedItemId());

		MashModel *model = ((MashEntity*)m_selectedNode)->GetModel();
		MashTriangleCollider *newCollider = m_device->GetSceneManager()->CreateTriangleCollider(model, selectedLod, colliderType, true);
		model->SetTriangleCollider(newCollider);
		newCollider->Drop();

		/*
			Remove all buffers from this model so we don't end up saving data we don't need.
		*/
		//model->DropAllTriangleBuffers();

		MashGUIView *elementPropertiesView = (MashGUIView*)m_viewerRoot->GetElementByName("Properties");
		MashGUIWindow *windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("EntitySettingsWindow");
		UpdateEntiryTriangleColliderGUI(((MashEntity*)m_selectedNode), windowElement);

		m_triColliderWindow->SetRenderEnabled(false);
	}

	void OnParticleCreationConfirm(const sGUIEvent &eventData)
	{
		MashGUIListBox *particleTypeLb = (MashGUIListBox*)m_particleCreationWindow->GetElementByName("ParticleCreationTypeLB");
		MashGUIListBox *lightTypeLb = (MashGUIListBox*)m_particleCreationWindow->GetElementByName("ParticleCreationLightTypeLB");

		//m_device->GetSceneManager()->_OnBeginUserInitialize();

		sParticleSettings defaultSettings;
		MashParticleSystem *particleSystem = m_device->GetSceneManager()->AddParticleSystem(m_sceneRoot, 
			"ParticleSystem", 
			defaultSettings,
			(ePARTICLE_TYPE)particleTypeLb->GetItemUserValue(particleTypeLb->GetSelectedItemId()),
			(eLIGHTING_TYPE)lightTypeLb->GetItemUserValue(lightTypeLb->GetSelectedItemId()),
			true, 0);

		//create emitter
		particleSystem->CreatePointEmitter();
		//zero out texture data because the particle system may have been instanced internally
		particleSystem->SetDiffuseTexture(0, 0);

        m_device->GetSceneManager()->CompileAllMaterials(aMATERIAL_COMPILER_NON_COMPILED);
		//m_device->GetSceneManager()->_OnEndUserInitialize();

		if (particleSystem)
		{
			FillNodeData(particleSystem);
			AddItemToSceneTree(particleSystem);
			//UpdateSceneTree();
		}

		m_particleCreationWindow->SetRenderEnabled(false);
	}

	void OnParticleWrapMode(const sGUIEvent &eventData)
	{
		MashGUIWindow *windowElement = (MashGUIWindow*)m_viewerRoot->GetElementByName("ParticleSettingsWindow");
		MashGUIListBox *lb = (MashGUIListBox*)windowElement->GetElementByName("ParticleTextureWrapModeLB");

		sSamplerState samplerState;
		samplerState.type = aSAMPLER2D;
		samplerState.filter = aFILTER_MIN_MAG_MIP_LINEAR;
		samplerState.uMode = (eTEXTURE_ADDRESS)lb->GetItemUserValue(lb->GetSelectedItemId());
		samplerState.vMode = samplerState.uMode;
		MashTextureState *textureState = m_device->GetRenderer()->AddSamplerState(samplerState);

		MashTexture *texture = ((MashParticleSystem*)m_selectedNode)->GetDiffuseTexture()->texture;
		((MashParticleSystem*)m_selectedNode)->SetDiffuseTexture(texture, textureState);
	}

	void OnParticleCreationCancel(const sGUIEvent &eventData)
	{
		m_particleCreationWindow->SetRenderEnabled(false);
	}

	void OnTriColliderCancel(const sGUIEvent &eventData)
	{
		m_triColliderWindow->SetRenderEnabled(false);
	}

	void OnEntityTriColliderRemove(const sGUIEvent &eventData)
	{
		((MashEntity*)m_selectedNode)->GetModel()->SetTriangleCollider(0);
		((MashEntity*)m_selectedNode)->GetModel()->DropAllTriangleBuffers();

		MashGUIView *elementPropertiesView = (MashGUIView*)m_viewerRoot->GetElementByName("Properties");
		MashGUIWindow *windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("EntitySettingsWindow");
		UpdateEntiryTriangleColliderGUI(((MashEntity*)m_selectedNode), windowElement);
	}

	void OnEntityTriColliderModify(const sGUIEvent &eventData)
	{
		MashGUIListBox *lb = (MashGUIListBox*)m_triColliderWindow->GetElementByName("TriColliderLodLB");
		lb->ClearAllItems();

		char buffer[100];
		unsigned int lodCount = ((MashEntity*)m_selectedNode)->GetLodCount();
		for(unsigned int i = 0; i < lodCount; ++i)
		{
			memset(buffer, 0, sizeof(buffer));
            helpers::NumberToString(buffer, sizeof(buffer), i);
			lb->AddItem(buffer, i);
		}

		if (lodCount > 0)
			lb->SetActiveItemByUserValue(0);	

		lb = (MashGUIListBox*)m_triColliderWindow->GetElementByName("TriColliderTypeLB");
		lb->SetActiveItemByUserValue(0);

		m_triColliderWindow->SetRenderEnabled(true);
	}

	void OnSelectActiveEntityLod(const sGUIEvent &eventData)
	{
		MashGUIListBox *lb = (MashGUIListBox*)eventData.component;
		m_entityData.selectedLod = lb->GetItemUserValue(lb->GetSelectedItemId());

		//on selection of a lod we need to fill the available meshes for this lod
		MashGUIView *elementPropertiesView = (MashGUIView*)m_viewerRoot->GetElementByName("Properties");
		MashGUIWindow *windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("EntitySettingsWindow");
		lb = (MashGUIListBox*)windowElement->GetElementByName("SubEntityLB");
		lb->ClearAllItems();

		char buffer[100];
		unsigned int lodCount = ((MashEntity*)m_selectedNode)->GetSubEntityCount(m_entityData.selectedLod);
		for(unsigned int j = 0; j < lodCount; ++j)
		{
			memset(buffer, 0, sizeof(buffer));            
            helpers::NumberToString(buffer, sizeof(buffer), j);
            
			lb->AddItem(buffer, j);
		}

		m_entityData.selectedMesh = 0;
		lb->SetActiveItemByUserValue(m_entityData.selectedMesh);

		if (lodCount == 0)//force call to clear data
			UpdateSelectedSubEntityGUISettings((MashEntity*)m_selectedNode, windowElement);
	}

	void OnSelectActiveSubEntity(const sGUIEvent &eventData)
	{
		MashGUIListBox *lb = (MashGUIListBox*)eventData.component;
		m_entityData.selectedMesh = lb->GetItemUserValue(lb->GetSelectedItemId());

		MashGUIView *elementPropertiesView = (MashGUIView*)m_viewerRoot->GetElementByName("Properties");
		MashGUIWindow *windowElement = (MashGUIWindow*)elementPropertiesView->GetElementByName("EntitySettingsWindow");
		UpdateSelectedSubEntityGUISettings((MashEntity*)m_selectedNode, windowElement);
	}

	void OnChangeShadowCasterLightType(const sGUIEvent &eventData)
	{
		MashGUIListBox *lb = (MashGUIListBox*)eventData.component;
		eLIGHTTYPE lightType = (eLIGHTTYPE)lb->GetItemUserValue(lb->GetSelectedItemId());
		switch(lightType)
		{
		case aLIGHT_DIRECTIONAL:
			{
				m_shadowCasterWindow->GetElementByName("DirectionalCasterWindow", false)->SetRenderEnabled(true);
				m_shadowCasterWindow->GetElementByName("SpotCasterWindow", false)->SetRenderEnabled(false);
				m_shadowCasterWindow->GetElementByName("PointCasterWindow", false)->SetRenderEnabled(false);

				lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("ShadowCasterCasterTypeLB",false);
				lb->ClearAllItems();
				lb->AddItem("No Caster", -1);
				lb->AddItem("Standard Filtering", aSHADOW_CASTER_DIRECTIONAL_CASCADE_STANDARD);
				lb->AddItem("ESM", aSHADOW_CASTER_DIRECTIONAL_CASCADE_ESM);
				lb->SetActiveItemByUserValue(-1);
				break;
			}
		case aLIGHT_POINT:
			{
				m_shadowCasterWindow->GetElementByName("DirectionalCasterWindow", false)->SetRenderEnabled(false);
				m_shadowCasterWindow->GetElementByName("SpotCasterWindow", false)->SetRenderEnabled(false);
				m_shadowCasterWindow->GetElementByName("PointCasterWindow", false)->SetRenderEnabled(true);

				lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("ShadowCasterCasterTypeLB",false);
				lb->ClearAllItems();
				lb->AddItem("No Caster", -1);
				lb->AddItem("No Filtering", aSHADOW_CASTER_POINT_STANDARD);
				lb->AddItem("Filterd", aSHADOW_CASTER_POINT_STANDARD_FILTERED);
				lb->AddItem("ESM", aSHADOW_CASTER_POINT_ESM);
				break;
			}
		case aLIGHT_SPOT:
			{
				m_shadowCasterWindow->GetElementByName("DirectionalCasterWindow", false)->SetRenderEnabled(false);
				m_shadowCasterWindow->GetElementByName("SpotCasterWindow", false)->SetRenderEnabled(true);
				m_shadowCasterWindow->GetElementByName("PointCasterWindow", false)->SetRenderEnabled(false);

				lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("ShadowCasterCasterTypeLB",false);
				lb->ClearAllItems();
				lb->AddItem("No Caster", -1);
				lb->AddItem("Standard Filtering", aSHADOW_CASTER_SPOT_STANDARD);
				lb->AddItem("ESM", aSHADOW_CASTER_SPOT_ESM);
				lb->SetActiveItemByUserValue(-1);
				break;
			}
		}

		_OnResetShadowCaster();
	}

	void UpdateDirCascadeCaster(eLIGHTTYPE lightType, int32 casterType)
	{
		//deleted current caster
		if (casterType == -1)
		{
			m_device->GetSceneManager()->SetShadowCaster(lightType, 0);
			_OnResetShadowCaster();
			return;
		}

		MashShadowCaster *tempCaster = m_device->GetSceneManager()->GetShadowCaster(lightType);
		MashDirectionalShadowCascadeCaster *caster = 0;
		if (!tempCaster || (tempCaster->GetShadowCasterType() != casterType))
		{
			MashDirectionalShadowCascadeCaster::eCASTER_TYPE ct;
			switch(casterType)
			{
			case aSHADOW_CASTER_DIRECTIONAL_CASCADE_STANDARD:
				ct = MashDirectionalShadowCascadeCaster::aCASTER_TYPE_STANDARD;
				break;
			case aSHADOW_CASTER_DIRECTIONAL_CASCADE_ESM:
				ct = MashDirectionalShadowCascadeCaster::aCASTER_TYPE_ESM;
				break;
			}

			caster = m_device->GetSceneManager()->CreateDirectionalCascadeShadowCaster(ct);
			m_device->GetSceneManager()->SetShadowCaster(lightType, caster);
		}
		else
		{
			caster = (MashDirectionalShadowCascadeCaster*)tempCaster;
		}

		MashGUIListBox *lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("DirShadowTexFormatLB");
		caster->SetTextureFormat((eSHADOW_MAP_FORMAT)lb->GetItemUserValue(lb->GetSelectedItemId()));
		lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("DirShadowTexSizeLB");
		caster->SetTextureSize(lb->GetItemUserValue(lb->GetSelectedItemId()));
		MashGUITextBox *tb = (MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowBiasTB");
		caster->SetBias(tb->GetTextAsFloat());
		tb = (MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowSampleSizeTB");
		caster->SetSampleSize(tb->GetTextAsFloat());
		lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("DirShadowSamplesLB");
		caster->SetSamples((eSHADOW_SAMPLES)lb->GetItemUserValue(lb->GetSelectedItemId()));
		tb = (MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowCascadeCountTB");
		caster->SetCascadeCount(tb->GetTextAsInt());
		tb = (MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowCascdeSpacingTB");
		caster->SetCascadeDivider(tb->GetTextAsFloat());
		tb = (MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowCascadeBlendTB");
		caster->SetCascadeEdgeBlendDistance(tb->GetTextAsFloat());
		tb = (MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowDistanceTB");
		MashGUICheckBox *cb = (MashGUICheckBox*)m_shadowCasterWindow->GetElementByName("DirShadowDistanceCB");
		caster->SetFixedShadowDistance(cb->IsChecked(), tb->GetTextAsFloat());
		tb = (MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowESMDarkeningTB");
		caster->SetESMDarkeningFactor(tb->GetTextAsFloat());
		cb = (MashGUICheckBox*)m_shadowCasterWindow->GetElementByName("DirShadowBackfaceCB");
		caster->UseBackfaceGeometry(cb->IsChecked());
	}

	void UpdateSpotCascadeCaster(eLIGHTTYPE lightType, int32 casterType)
	{
		//deleted current caster
		if (casterType == -1)
		{
			m_device->GetSceneManager()->SetShadowCaster(lightType, 0);
			_OnResetShadowCaster();
			return;
		}

		MashShadowCaster *tempCaster = m_device->GetSceneManager()->GetShadowCaster(lightType);
		MashSpotShadowCaster *caster = 0;
		if (!tempCaster || (tempCaster->GetShadowCasterType() != casterType))
		{
			MashSpotShadowCaster::eCASTER_TYPE ct;
			switch(casterType)
			{
			case aSHADOW_CASTER_SPOT_STANDARD:
				ct = MashSpotShadowCaster::aCASTER_TYPE_STANDARD;
				break;
			case aSHADOW_CASTER_SPOT_ESM:
				ct = MashSpotShadowCaster::aCASTER_TYPE_ESM;
				break;
			}

			caster = m_device->GetSceneManager()->CreateSpotShadowCaster(ct);
			m_device->GetSceneManager()->SetShadowCaster(lightType, caster);
		}
		else
		{
			caster = (MashSpotShadowCaster*)tempCaster;
		}

		MashGUIListBox *lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("SpotShadowTexFormatLB");
		caster->SetTextureFormat((eSHADOW_MAP_FORMAT)lb->GetItemUserValue(lb->GetSelectedItemId()));
		lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("SpotShadowTexSizeLB");
		caster->SetTextureSize(lb->GetItemUserValue(lb->GetSelectedItemId()));
		MashGUITextBox *tb = (MashGUITextBox*)m_shadowCasterWindow->GetElementByName("SpotShadowBiasTB");
		caster->SetBias(tb->GetTextAsFloat());
		tb = (MashGUITextBox*)m_shadowCasterWindow->GetElementByName("SpotShadowSampleSizeTB");
		caster->SetSampleSize(tb->GetTextAsFloat());
		lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("SpotShadowSamplesLB");
		caster->SetSamples((eSHADOW_SAMPLES)lb->GetItemUserValue(lb->GetSelectedItemId()));
		tb = (MashGUITextBox*)m_shadowCasterWindow->GetElementByName("SpotShadowESMDarkeningTB");
		caster->SetESMDarkeningFactor(tb->GetTextAsFloat());
		MashGUICheckBox *cb = (MashGUICheckBox*)m_shadowCasterWindow->GetElementByName("SpotShadowBackfaceCB");
		caster->UseBackfaceGeometry(cb->IsChecked());
	}

	void UpdatePointCascadeCaster(eLIGHTTYPE lightType, int32 casterType)
	{
		//deleted current caster
		if (casterType == -1)
		{
			m_device->GetSceneManager()->SetShadowCaster(lightType, 0);
			_OnResetShadowCaster();
			return;
		}

		MashShadowCaster *tempCaster = m_device->GetSceneManager()->GetShadowCaster(lightType);
		MashPointShadowCaster *caster = 0;
		if (!tempCaster || (tempCaster->GetShadowCasterType() != casterType))
		{
			MashPointShadowCaster::eCASTER_TYPE ct;
			switch(casterType)
			{
			case aSHADOW_CASTER_POINT_STANDARD:
				ct = MashPointShadowCaster::aCASTER_TYPE_STANDARD;
				break;
			case aSHADOW_CASTER_POINT_STANDARD_FILTERED:
				ct = MashPointShadowCaster::aCASTER_TYPE_STANDARD_FILTERED;
				break;
			case aSHADOW_CASTER_POINT_ESM:
				ct = MashPointShadowCaster::aCASTER_TYPE_ESM;
				break;
			}
			caster = m_device->GetSceneManager()->CreatePointShadowCaster(ct);
			m_device->GetSceneManager()->SetShadowCaster(lightType, caster);
		}
		else
		{
			caster = (MashPointShadowCaster*)tempCaster;
		}

		MashGUIListBox *lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("PointShadowTexFormatLB");
		caster->SetTextureFormat((eSHADOW_MAP_FORMAT)lb->GetItemUserValue(lb->GetSelectedItemId()));
		lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("PointShadowTexSizeLB");
		caster->SetTextureSize(lb->GetItemUserValue(lb->GetSelectedItemId()));
		MashGUITextBox *tb = (MashGUITextBox*)m_shadowCasterWindow->GetElementByName("PointShadowBiasTB");
		caster->SetBias(tb->GetTextAsFloat());
		tb = (MashGUITextBox*)m_shadowCasterWindow->GetElementByName("PointShadowESMDarkeningTB");
		caster->SetESMDarkeningFactor(tb->GetTextAsFloat());
		MashGUICheckBox *cb = (MashGUICheckBox*)m_shadowCasterWindow->GetElementByName("PointShadowBackfaceCB");
		caster->UseBackfaceGeometry(cb->IsChecked());
	}

	void OnUpdateShadowCaster(const sGUIEvent &eventData)
	{
		MashGUIListBox *lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("ShadowCasterLightTypeLB", false);
		eLIGHTTYPE lightType = (eLIGHTTYPE)lb->GetItemUserValue(lb->GetSelectedItemId());
		switch(lightType)
		{
		case aLIGHT_DIRECTIONAL:
			{
				lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("ShadowCasterCasterTypeLB", false);
				int32 casterType = lb->GetItemUserValue(lb->GetSelectedItemId());
				UpdateDirCascadeCaster(lightType, casterType);
				break;
			}
		case aLIGHT_POINT:
			{
				lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("ShadowCasterCasterTypeLB", false);
				int32 casterType = lb->GetItemUserValue(lb->GetSelectedItemId());
				UpdatePointCascadeCaster(lightType, casterType);
				break;
			}
		case aLIGHT_SPOT:
			{
				lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("ShadowCasterCasterTypeLB", false);
				int32 casterType = lb->GetItemUserValue(lb->GetSelectedItemId());
				UpdateSpotCascadeCaster(lightType, casterType);
				break;
			}
		}
	}

	void OnResetShadowCaster(const sGUIEvent &eventData)
	{
		_OnResetShadowCaster();
	}

	void _OnResetShadowCaster()
	{
		m_device->GetGUIManager()->SetGlobalMute(true);

		MashGUIListBox *lb = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("ShadowCasterLightTypeLB", false);
		eLIGHTTYPE lightType = (eLIGHTTYPE)lb->GetItemUserValue(lb->GetSelectedItemId());
		switch(lightType)
		{
		case aLIGHT_DIRECTIONAL:
			{
				MashGUIListBox *casterTypeLB = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("ShadowCasterCasterTypeLB", false);
				int32 casterType = lb->GetItemUserValue(lb->GetSelectedItemId());

				MashShadowCaster *tempCaster = m_device->GetSceneManager()->GetShadowCaster(lightType);
				MashDirectionalShadowCascadeCaster *caster = 0;
				if (!tempCaster)
				{
					casterTypeLB->SetActiveItemByUserValue(-1);

					//set lb defaults so shadow caster updates go smoothly
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("DirShadowTexFormatLB"))->SetActiveItemByUserValue(aSHADOW_FORMAT_16);
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("DirShadowSamplesLB"))->SetActiveItemByUserValue(eSHADOW_SAMPLES_5);
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("DirShadowTexSizeLB"))->SetActiveItemByUserValue(512);
				}
				else
				{
					switch(tempCaster->GetShadowCasterType())
					{
					case aSHADOW_CASTER_DIRECTIONAL_CASCADE_STANDARD:
						casterTypeLB->SetActiveItemByUserValue(aSHADOW_CASTER_DIRECTIONAL_CASCADE_STANDARD);
						break;
					case aSHADOW_CASTER_DIRECTIONAL_CASCADE_ESM:
						casterTypeLB->SetActiveItemByUserValue(aSHADOW_CASTER_DIRECTIONAL_CASCADE_ESM);
						break;
					}

					caster = (MashDirectionalShadowCascadeCaster*)tempCaster;
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("DirShadowTexFormatLB"))->SetActiveItemByUserValue(caster->GetTextureFormat());
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("DirShadowTexSizeLB"))->SetActiveItemByUserValue(caster->GetTextureSize());
					((MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowBiasTB"))->SetTextFloat(caster->GetBias());
					((MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowSampleSizeTB"))->SetTextFloat(caster->GetSampleSize());
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("DirShadowSamplesLB"))->SetActiveItemByUserValue(caster->GetSampleCount());
					((MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowESMDarkeningTB"))->SetTextFloat(caster->GetESMDarkeningFactor());
					((MashGUICheckBox*)m_shadowCasterWindow->GetElementByName("DirShadowBackfaceCB"))->SetChecked(caster->GetBackfaceRenderingEnabled());

					((MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowCascadeCountTB"))->SetTextInt(caster->GetCascadeCount());
					((MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowCascdeSpacingTB"))->SetTextFloat(caster->GetCascadeDivider());
					((MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowCascadeBlendTB"))->SetTextFloat(caster->GetCascadeEdgeBlendDistance());
					((MashGUITextBox*)m_shadowCasterWindow->GetElementByName("DirShadowDistanceTB"))->SetTextFloat(caster->GetFixedShadowDistance());
					((MashGUICheckBox*)m_shadowCasterWindow->GetElementByName("DirShadowDistanceCB"))->SetChecked(caster->IsFixedShadowDistanceEnabled());
				}

				break;
			}
		case aLIGHT_POINT:
			{
				MashGUIListBox *casterTypeLB = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("ShadowCasterCasterTypeLB", false);
				int32 casterType = lb->GetItemUserValue(lb->GetSelectedItemId());

				MashShadowCaster *tempCaster = m_device->GetSceneManager()->GetShadowCaster(lightType);
				MashPointShadowCaster *caster = 0;
				if (!tempCaster)
				{
					casterTypeLB->SetActiveItemByUserValue(-1);

					//set lb defaults so shadow caster updates go smoothly
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("PointShadowTexFormatLB"))->SetActiveItemByUserValue(aSHADOW_FORMAT_16);
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("PointShadowTexSizeLB"))->SetActiveItemByUserValue(512);
				}
				else
				{
					switch(tempCaster->GetShadowCasterType())
					{
					case aSHADOW_CASTER_POINT_STANDARD:
						casterTypeLB->SetActiveItemByUserValue(aSHADOW_CASTER_SPOT_STANDARD);
						break;
					case aSHADOW_CASTER_POINT_STANDARD_FILTERED:
						casterTypeLB->SetActiveItemByUserValue(aSHADOW_CASTER_POINT_STANDARD_FILTERED);
						break;
					case aSHADOW_CASTER_POINT_ESM:
						casterTypeLB->SetActiveItemByUserValue(aSHADOW_CASTER_SPOT_ESM);
						break;
					}

					caster = (MashPointShadowCaster*)tempCaster;
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("PointShadowTexFormatLB"))->SetActiveItemByUserValue(caster->GetTextureFormat());
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("PointShadowTexSizeLB"))->SetActiveItemByUserValue(caster->GetTextureSize());
					((MashGUITextBox*)m_shadowCasterWindow->GetElementByName("PointShadowBiasTB"))->SetTextFloat(caster->GetBias());
					((MashGUITextBox*)m_shadowCasterWindow->GetElementByName("PointShadowESMDarkeningTB"))->SetTextFloat(caster->GetESMDarkeningFactor());
					((MashGUICheckBox*)m_shadowCasterWindow->GetElementByName("PointShadowBackfaceCB"))->SetChecked(caster->GetBackfaceRenderingEnabled());
				}
				break;
			}
		case aLIGHT_SPOT:
			{
				MashGUIListBox *casterTypeLB = (MashGUIListBox*)m_shadowCasterWindow->GetElementByName("ShadowCasterCasterTypeLB", false);
				int32 casterType = lb->GetItemUserValue(lb->GetSelectedItemId());

				MashShadowCaster *tempCaster = m_device->GetSceneManager()->GetShadowCaster(lightType);
				MashSpotShadowCaster *caster = 0;
				if (!tempCaster)
				{
					casterTypeLB->SetActiveItemByUserValue(-1);

					//set lb defaults so shadow caster updates go smoothly
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("SpotShadowTexFormatLB"))->SetActiveItemByUserValue(aSHADOW_FORMAT_16);
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("SpotShadowSamplesLB"))->SetActiveItemByUserValue(eSHADOW_SAMPLES_5);
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("SpotShadowTexSizeLB"))->SetActiveItemByUserValue(512);
				}
				else
				{
					switch(tempCaster->GetShadowCasterType())
					{
					case aSHADOW_CASTER_SPOT_STANDARD:
						casterTypeLB->SetActiveItemByUserValue(aSHADOW_CASTER_SPOT_STANDARD);
						break;
					case aSHADOW_CASTER_SPOT_ESM:
						casterTypeLB->SetActiveItemByUserValue(aSHADOW_CASTER_SPOT_ESM);
						break;
					}

					caster = (MashSpotShadowCaster*)tempCaster;
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("SpotShadowTexFormatLB"))->SetActiveItemByUserValue(caster->GetTextureFormat());
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("SpotShadowTexSizeLB"))->SetActiveItemByUserValue(caster->GetTextureSize());
					((MashGUITextBox*)m_shadowCasterWindow->GetElementByName("SpotShadowBiasTB"))->SetTextFloat(caster->GetBias());
					((MashGUITextBox*)m_shadowCasterWindow->GetElementByName("SpotShadowSampleSizeTB"))->SetTextFloat(caster->GetSampleSize());
					((MashGUIListBox*)m_shadowCasterWindow->GetElementByName("SpotShadowSamplesLB"))->SetActiveItemByUserValue(caster->GetSampleCount());
					((MashGUITextBox*)m_shadowCasterWindow->GetElementByName("SpotShadowESMDarkeningTB"))->SetTextFloat(caster->GetESMDarkeningFactor());
					((MashGUICheckBox*)m_shadowCasterWindow->GetElementByName("SpotShadowBackfaceCB"))->SetChecked(caster->GetBackfaceRenderingEnabled());
				}
				break;
			}
		}

		m_device->GetGUIManager()->SetGlobalMute(false);
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
	deviceSettings.preferredLightingMode = aLIGHT_TYPE_PIXEL;
	deviceSettings.antiAliasType = aANTIALIAS_TYPE_NONE;//needs to be none because deferred rendering can be selected
	deviceSettings.enableDeferredRender = true;

    deviceSettings.rootPaths.PushBack("../../../../../Media/GUI");
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");
    deviceSettings.rootPaths.PushBack("../../../../DemoMedia/SceneViewer");

	//Creates the device
	MashDevice *device = CreateDevice(deviceSettings);

	if (!device)
		return 1;

	device->SetWindowCaption("Scene Viewer");

	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();

	device->Drop();
    
    return 0;
}
