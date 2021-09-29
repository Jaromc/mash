//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#include "MashInclude.h"

#include "MemoryAllocator/MashDefaultMemoryAllocator.h"
#include "D3D10/MashD3D10Creation.h"
#include "OpenGL3/MashOpenGL3Creation.h"

#if defined (MASH_WINDOWS) && !defined(__MINGW32__)
    #define USE_DIRECTX
#endif

using namespace mash;

enum eEXTRA_GUI_ENUM
{
	EXTRA_GUI_OPEN_FILE_DIALOG = aGUI_TYPE_COUNT + 1
};
class MainLoop : public MashGameLoop
{
private:
	MashDevice *m_device;
	MashDummy *m_root;
	uint32 m_playerControl;

	MashGUIPopupMenu *m_rightClickMenu;
	MashGUIOpenFileDialog *m_openFileDialog;

	f32 m_alphaMaskDelta;
	MashGUISprite *m_alphaMaskSprite;

	MashRenderSurface *m_playerNameSurface;
	MashGUIStaticText *m_playerNameGUIText;
	MashEntity *m_playerEntity;
	MashMaterial *m_playerNameBillboardMaterial;
	MashVector3 m_playerMovement;
	bool m_playerNameInitialized;
public:
	MainLoop(MashDevice *device):m_device(device), m_alphaMaskDelta(0.0f), m_playerNameSurface(0), m_playerNameInitialized(false){}
	virtual ~MainLoop()
	{
		m_openFileDialog->Drop();
		m_rightClickMenu->Destroy();
		m_playerNameSurface->Drop();
	}

	bool Initialise()
	{
		m_playerControl = m_device->GetInputManager()->CreatePlayer();
		const MashStringc inputContext = "play";
		m_device->GetInputManager()->SetCurrentPlayerContext(m_playerControl, inputContext);
		m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 0);//keyboard/mouse
		
		if (m_device->GetInputManager()->GetControllerCount() > 1)
			m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 1);//joystick

		/*
			TODO : Get gamepad to emulate the keyboard
		*/
		MashArray<eINPUT_EVENT> actions;
		actions.PushBack(aKEYEVENT_LEFT);
		actions.PushBack(aKEYEVENT_RIGHT);
		m_device->GetInputManager()->SetEventValueSign(aKEYEVENT_LEFT, true);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION, actions, MashInputEventFunctor(&MainLoop::OnAlphaMaskChange, this));

		actions.Clear();
		actions.PushBack(aMOUSEEVENT_B2);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION+1, actions, MashInputEventFunctor(&MainLoop::OnRightClick, this));

		actions.Clear();
		actions.PushBack(aKEYEVENT_W);
		actions.PushBack(aKEYEVENT_S);
		m_device->GetInputManager()->SetEventValueSign(aKEYEVENT_S, true);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION+2, actions, MashInputEventFunctor(&MainLoop::OnMoveForward, this));

		actions.Clear();
		actions.PushBack(aKEYEVENT_A);
		actions.PushBack(aKEYEVENT_D);
		m_device->GetInputManager()->SetEventValueSign(aKEYEVENT_A, true);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION+3, actions, MashInputEventFunctor(&MainLoop::OnMoveHorizontal, this));

		m_root = m_device->GetSceneManager()->AddDummy(0, "Root");
		MashCamera *camera = (MashCamera*)m_device->GetSceneManager()->AddCamera(m_root, "Camera01");
		camera->SetZFar(1000);
		camera->SetZNear(1.0f);
		camera->SetPosition(MashVector3(10, 10, -50));
		
		MashLight *light = (MashLight*)m_device->GetSceneManager()->AddLight(m_root, "Light01", aLIGHT_DIRECTIONAL, aLIGHT_RENDERER_FORWARD, true);
		light->SetDefaultDirectionalLightSettings(MashVector3(-1,0,0));

		MashGUIPopupMenu *addGUIElementMenu = m_device->GetGUIManager()->CreatePopupMenu();
		addGUIElementMenu->AddItem("Button", aGUI_BUTTON);
		addGUIElementMenu->AddItem("CheckBox", aGUI_CHECK_BOX);
		addGUIElementMenu->AddItem("TextBox", aGUI_TEXT_BOX);
		addGUIElementMenu->AddItem("ListBox", aGUI_LISTBOX);
		addGUIElementMenu->AddItem("Tree", aGUI_TREE);
		addGUIElementMenu->AddItem("MenuBar", aGUI_MENUBAR);
		addGUIElementMenu->AddItem("Window", aGUI_WINDOW);
		addGUIElementMenu->AddItem("OpenFileDialog", EXTRA_GUI_OPEN_FILE_DIALOG);
		
		addGUIElementMenu->RegisterReceiver(aGUIEVENT_POPUP_SELECTION, MashGUIEventFunctor(&MainLoop::OnAddItem, this));
		
		m_rightClickMenu = m_device->GetGUIManager()->CreatePopupMenu();
		m_rightClickMenu->AddItem("Add Element", 0, addGUIElementMenu);

		MashGUIRect spriteRegion(MashGUIUnit(0.0f, 300.0f), MashGUIUnit(0.0f, 300.0f),
			MashGUIUnit(0.0f, 500.0f), MashGUIUnit(0.0f, 500.0f));
		m_alphaMaskSprite = m_device->GetGUIManager()->AddSprite(spriteRegion, 0);
		MashGUISkin *alphaTextureSkin = m_alphaMaskSprite->GetSkin();
		alphaTextureSkin->SetTexture(m_device->GetRenderer()->GetTexture("AlphaMaskTexture.png"));
		alphaTextureSkin->ScaleSourceToTexture();
		alphaTextureSkin->isTransparent = false;
		alphaTextureSkin->alphaMaskThreshold = 0.0f;

		/*
			There only ever needs to be one openfile dialog. We just open it when needed.
		*/
		MashGUIRect dialogDest(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 500), MashGUIUnit(0.0f, 400));
		m_openFileDialog = m_device->GetGUIManager()->CreateOpenFileDialog(dialogDest);

		MashGUIRect textRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 200.0f),
			MashGUIUnit(0.0f, 300.0f), MashGUIUnit(0.0f, 300.0f));
		MashGUIStaticText *staticText = m_device->GetGUIManager()->AddStaticText(textRegion, 0);

		MashStringc text= "\Controls :\nRight click to open popup menu\n\
				Left/Right arrows to change alpha mask\n\
				A/D to move player left/right\n\
				W/S to move player forward/backward";

		staticText->SetText(text);

		/*
			The following creates a moveable player and a billboard that will sit above the player.
		*/
		uint32 stringAreaX, stringAreaY;
		MashStringc playerName = "Name";
		m_device->GetGUIManager()->GetActiveGUIStyle()->GetFont()->GetBoundingAreaForString(playerName, 200, stringAreaX, stringAreaY);
		MashGUIRect playerNameDest(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, stringAreaX), MashGUIUnit(0.0f, stringAreaY));
		
		m_playerNameGUIText = m_device->GetGUIManager()->AddStaticText(playerNameDest, 0);
		m_playerNameGUIText->SetText(playerName);

		eFORMAT renderTargetFormat = aFORMAT_RGBA16_FLOAT;
		m_playerNameSurface = m_device->GetRenderer()->CreateRenderSurface(stringAreaX, stringAreaY, &renderTargetFormat, 1, 1, aDEPTH_OPTION_NO_DEPTH);

		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("StaticMeshMaterial.mtl");
		MashMaterial *staticMeshMaterial = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("StaticMeshMaterial");
		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("BillboardMaterial.mtl");
		m_playerNameBillboardMaterial = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("BillboardMaterial");

		MashMesh *playerMesh = m_device->GetSceneManager()->CreateStaticMesh();
		m_device->GetSceneManager()->GetMeshBuilder()->CreateCube(playerMesh, 5, 5, 5, staticMeshMaterial->GetVertexDeclaration());
		MashModel *playerModel = m_device->GetSceneManager()->CreateModel();
		playerModel->Append(&playerMesh);
		playerMesh->Drop();

		m_playerEntity = m_device->GetSceneManager()->AddEntity(m_root, playerModel, "Player");
		playerModel->Drop();
		m_playerEntity->SetMaterialToAllSubEntities(staticMeshMaterial);

		int32 planeWidth = stringAreaX*0.2f;
		int32 planeHeight = stringAreaY*0.2f;
		MashMesh *playerNameMesh = m_device->GetSceneManager()->CreateStaticMesh();
		m_device->GetSceneManager()->GetMeshBuilder()->CreatePlane(playerNameMesh, 1, planeWidth, planeHeight, 
			m_playerNameBillboardMaterial->GetVertexDeclaration(), MashVector3(0,0,-1));
		MashModel *playerNameModel = m_device->GetSceneManager()->CreateModel();
		playerNameModel->Append(&playerNameMesh);
		playerNameMesh->Drop();

		MashEntity *m_playerNameBillboard = m_device->GetSceneManager()->AddEntity(m_playerEntity, playerNameModel, "PlayerName");
		playerNameModel->Drop();
		m_playerNameBillboard->SetMaterialToAllSubEntities(m_playerNameBillboardMaterial);

		m_playerNameBillboard->SetPosition(MashVector3(0.0f, 7.0f, 0.0f));
		m_playerNameBillboard->SetInheritTranslationOnly(true);

		return false;
	}

	bool Update(f32 dt)
	{
		m_device->GetSceneManager()->UpdateScene(dt, m_root);

		MashGUISkin *alphaTextureSkin = m_alphaMaskSprite->GetSkin();
		alphaTextureSkin->alphaMaskThreshold += m_alphaMaskDelta * dt;
		alphaTextureSkin->alphaMaskThreshold = math::Clamp<f32>(0.0f, 1.0f, alphaTextureSkin->alphaMaskThreshold);

		if (!m_playerMovement.IsZero())
		{
			m_playerEntity->AddPosition(m_playerMovement * 10.0f * dt);

			MashQuaternion rotation;
			rotation.AddScaledVector(m_playerMovement,dt);
			m_playerEntity->AddOrientation(rotation);
		}
		
		return false;
	}

	void Render()
	{
		/*
			CullScene must still be called on the camera so that it gets its final update.
		*/
		m_device->GetSceneManager()->CullScene(m_root);

		if (!m_playerNameInitialized)
		{
			/*
				This only needs to be done once.
			*/
			m_device->GetRenderer()->SetRenderTarget(m_playerNameSurface);
            m_device->GetRenderer()->ClearTarget(aCLEAR_TARGET | aCLEAR_DEPTH, sMashColour4(0.0f, 0.0f, 0.0f, 0.0f), 1.0f);
			m_device->GetGUIManager()->BeginDraw();
			m_device->GetGUIManager()->DrawComponent(m_playerNameGUIText);
			m_device->GetGUIManager()->EndDraw();

			m_playerNameGUIText->Destroy();
			m_playerNameGUIText = 0;

			m_playerNameBillboardMaterial->SetTexture(0, m_playerNameSurface->GetTexture(0));

			m_playerNameInitialized = true;
		}
		
		m_device->GetRenderer()->SetRenderTargetDefault();
		
		m_device->GetSceneManager()->DrawScene();

		m_device->GetGUIManager()->BeginDraw();
		m_device->GetGUIManager()->DrawAll();
		m_device->GetGUIManager()->EndDraw();

	}

	void OnAddItem(const sGUIEvent &e)
	{
		MashGUIPopupMenu *popup = (MashGUIPopupMenu*)e.component;
		int32 selectedElement = popup->GetItemUserValue(popup->GetSelectedItemId());

		MashVector2 topLeftStartPosition(m_rightClickMenu->GetAbsoluteRegion().left, m_rightClickMenu->GetAbsoluteRegion().top);

		/*
			If no parent is returned then all objects are parented
			to the root window by default.
		*/
		MashGUIView *parent = m_device->GetGUIManager()->GetRootWindow()->GetClosestParentableObject(topLeftStartPosition);
		if (parent)
		{
			//move the new element into parent space
			topLeftStartPosition.x -= parent->GetAbsoluteRegion().left;
			topLeftStartPosition.y -= parent->GetAbsoluteRegion().top;
		}

		switch(selectedElement)
		{
		case aGUI_BUTTON:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 50), MashGUIUnit(0.0f, topLeftStartPosition.y + 20));

				MashGUIComponent *newElement = m_device->GetGUIManager()->AddButton(rect, parent);
				break;
			}
		case aGUI_CHECK_BOX:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 20), MashGUIUnit(0.0f, topLeftStartPosition.y + 20));

				MashGUIComponent *newElement = m_device->GetGUIManager()->AddCheckBox(rect, parent);
				break;
			}
		case aGUI_WINDOW:
				{
					MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
						MashGUIUnit(0.0f, topLeftStartPosition.x + 300), MashGUIUnit(0.0f, topLeftStartPosition.y + 300));

					MashGUIWindow *newElement = m_device->GetGUIManager()->AddWindow(rect, parent);
					newElement->EnableCloseButton(true);
					newElement->EnableMinimizeButton(true);
					break;
				}
		case aGUI_TEXT_BOX:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 100), MashGUIUnit(0.0f, topLeftStartPosition.y + 50));

				MashGUITextBox *texbox = m_device->GetGUIManager()->AddTextBox(rect, parent);
				//texbox->SetWordWrap(false);
				texbox->AddString("Add Text");
				break;
			}
		case aGUI_LISTBOX:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 300), MashGUIUnit(0.0f, topLeftStartPosition.y + 200));

				MashTexture *icon = m_device->GetRenderer()->GetTexture("ListboxIcon.png");
				MashGUIListBox *listBox = m_device->GetGUIManager()->AddListBox(rect, parent);
				listBox->EnableIcons(true, true);
				listBox->AddItem("Item0", 0, icon);
				listBox->AddItem("Item1", 1, icon);
				listBox->AddItem("Item2", 2, icon);
				listBox->AddItem("Item3", 3, icon);
				listBox->AddItem("Item4", 4, icon);
				listBox->AddItem("Item5", 5, icon);
				listBox->AddItem("Item6", 6, icon);
				listBox->AddItem("Item7", 7, icon);
				listBox->AddItem("Item8", 8, icon);
				listBox->AddItem("Item9", 9, icon);
				break;
			}
		case aGUI_TREE:
			{
				MashGUIRect rect(MashGUIUnit(0.0f, topLeftStartPosition.x), MashGUIUnit(0.0f, topLeftStartPosition.y),
					MashGUIUnit(0.0f, topLeftStartPosition.x + 200), MashGUIUnit(0.0f, topLeftStartPosition.y + 200));

				MashGUITree *tree = m_device->GetGUIManager()->AddTree(rect, parent);
				tree->AddItem("node0");
				tree->AddItem("node1");
				int32 p0 = tree->AddItem("node4");
				int32 p1 = tree->AddItem("node3", p0);
				tree->AddItem("node5", p1);
				tree->AddItem("node2", p0);
				break;
			}
		case aGUI_MENUBAR:
			{
				MashGUIMenuBar *menuBar = m_device->GetGUIManager()->AddMenuBar(parent);

				MashGUIPopupMenu *newSubMenu = m_device->GetGUIManager()->CreatePopupMenu();
				newSubMenu->AddItem("Texture");
				newSubMenu->AddItem("Mesh");
				newSubMenu->AddItem("Document");

				MashGUIPopupMenu *fileSubMenu = m_device->GetGUIManager()->CreatePopupMenu();
				fileSubMenu->AddItem("New", 0, newSubMenu);
				fileSubMenu->AddItem("Load");
				fileSubMenu->AddItem("Save");
				fileSubMenu->AddItem("Quit");

				MashGUIPopupMenu *viewSubMenu = m_device->GetGUIManager()->CreatePopupMenu();
				viewSubMenu->AddItem("View0");
				viewSubMenu->AddItem("View1");
				viewSubMenu->AddItem("View2");
				viewSubMenu->AddItem("View3");
				viewSubMenu->AddItem("View4");
				viewSubMenu->AddItem("View5");

				menuBar->AddItem("File", fileSubMenu);
				menuBar->AddItem("View", viewSubMenu);
				break;
			}
		case EXTRA_GUI_OPEN_FILE_DIALOG:
			{
				m_openFileDialog->OpenDialog();
				break;
			}
		}
	}

	void OnRightClick(const sInputEvent &e)
	{
		if (e.isPressed == 1)
		{
			m_rightClickMenu->Activate(m_device->GetInputManager()->GetCursorPosition());
		}
	}

	void OnAlphaMaskChange(const sInputEvent &e)
	{
		m_alphaMaskDelta = e.value * 0.5f;
	}

	void OnMoveForward(const sInputEvent &e)
	{
		if (e.isPressed == 1)
			m_playerMovement.z = e.value;
		else
			m_playerMovement.z = 0.0f;
	}

	void OnMoveHorizontal(const sInputEvent &e)
	{
		if (e.isPressed == 1)
			m_playerMovement.x = e.value;
		else
			m_playerMovement.x = 0.0f;
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
	deviceSettings.screenWidth = 800;
	deviceSettings.screenHeight = 600;
	deviceSettings.enableVSync = false;
	deviceSettings.preferredLightingMode = aLIGHT_TYPE_PIXEL;
	deviceSettings.antiAliasType = aANTIALIAS_TYPE_X4;

    deviceSettings.rootPaths.PushBack("../../../../../Media/GUI");
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");
    deviceSettings.rootPaths.PushBack("../../../../DemoMedia/GUIDemo");

	MashDevice *device = CreateDevice(deviceSettings);

	if (!device)
		return 1;

	device->SetWindowCaption("GUI Demo");

	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();

	device->Drop();
    
    return 0;
}
