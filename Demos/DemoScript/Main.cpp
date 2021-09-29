//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#include "MashInclude.h"

#include "MemoryAllocator/MashDefaultMemoryAllocator.h"
#include "D3D10/MashD3D10Creation.h"
#include "OpenGL3/MashOpenGL3Creation.h"
#include <time.h>

#if defined (MASH_WINDOWS) && !defined(__MINGW32__)
    #define USE_DIRECTX
#endif

using namespace mash;

class MainLoop : public MashGameLoop
{
private:
	static MashDevice *m_device;

	MashSceneNode *m_root;
	uint32 m_playerControl;
	bool m_quit;

	MashGUIStaticText *m_fpsText;
public:
	MainLoop(MashDevice *device): m_quit(false)
	{
		m_device = device;
	}

	virtual ~MainLoop(){}

	static int RotateVectorToNodeLuaCallback(MashLuaScript *script)
	{
		MashVector3 vec;

		/*
			The Get() methods will return each parameter in our function from lua.
			In this function, there were 4 params. So we call it 4 times. Calling
			Get() < 4 times or > 4 times may result in errors.
		*/
		uint32 node = script->GetInt(1);
		vec.x = script->GetFloat(2);
		vec.y = script->GetFloat(3);
		vec.z = script->GetFloat(4);

		/*
			This method will check the active script scene node before searching all
			the nodes. So it is fast if always operating with the current node.
		*/
		MashSceneNode *currentNode = m_device->GetSceneManager()->GetSceneNodeByID(node);
		vec = currentNode->GetWorldTransformState().TransformRotation(vec);

		/*
			This is similar to the Get() method. Our lua function expects 3 float values
			to be returned. So we push 3 objects onto the stack.
		*/
		script->PushFloat(vec.x);
		script->PushFloat(vec.y);
		script->PushFloat(vec.z);

		//We must state how many objects are being returned. 3 in this case.
		return 3;
	}

	bool Initialise()
	{
		srand(time(NULL));

		m_playerControl = m_device->GetInputManager()->CreatePlayer();
		const MashStringc inputContext = "play";
		m_device->GetInputManager()->SetCurrentPlayerContext(m_playerControl, inputContext);
		m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 0);//keyboard/mouse

		if (m_device->GetInputManager()->GetControllerCount() > 1)
        {
			m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 1);//joystick

            //set a small deadzone to the controllers axis
            sJoystickThreshold controllerThreshold;
            controllerThreshold.axis1 = 0.2f;
            controllerThreshold.axis2 = 0.2f;
            controllerThreshold.throttle1 = 0.05f;
            controllerThreshold.throttle2 = 0.05f;
            m_device->GetInputManager()->SetControllerThresholds(1, controllerThreshold);
        }

		//We invert and turn down the sensitivity of the mouse
		sControllerSensitivity constrollerSensitivity = *m_device->GetInputManager()->GetControllerSensitivity(0);
		constrollerSensitivity.mouseAxisX = 0.5f;
		constrollerSensitivity.mouseAxisY = -0.5f;
		m_device->GetInputManager()->SetControllerSensitivity(0, constrollerSensitivity);

		/*
			These are the action values for our input system. They are also used
			to define custom keymapping for our lua script.
		*/
		const uint32 moveXAxis = aINPUTMAP_USER_REGION;
		const uint32 moveYAxis = moveXAxis + 1;
		const uint32 lookXAxis = moveYAxis + 1;
		const uint32 lookYAxis = lookXAxis + 1;
		const uint32 quitAPP = lookYAxis + 1;

		MashArray<eINPUT_EVENT> actions;
		actions.PushBack(aKEYEVENT_W);
		actions.PushBack(aKEYEVENT_S);
		actions.PushBack(aJOYEVENT_AXIS_1_Y);
        actions.PushBack(aJOYEVENT_POVUP);
        actions.PushBack(aJOYEVENT_POVDOWN);
        //these keys will return a positive value, but to emulate an axis we want negative values instead
		m_device->GetInputManager()->SetEventValueSign(aKEYEVENT_S, true);
        m_device->GetInputManager()->SetEventValueSign(aJOYEVENT_POVDOWN, true);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, moveYAxis, actions);

		actions.Clear();
		actions.PushBack(aKEYEVENT_A);
		actions.PushBack(aKEYEVENT_D);
		actions.PushBack(aJOYEVENT_AXIS_1_X);
        actions.PushBack(aJOYEVENT_POVRIGHT);
        actions.PushBack(aJOYEVENT_POVLEFT);
        m_device->GetInputManager()->SetEventValueSign(aKEYEVENT_A, true);
        m_device->GetInputManager()->SetEventValueSign(aJOYEVENT_POVLEFT, true);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, moveXAxis, actions);

		actions.Clear();
		actions.PushBack(aMOUSEEVENT_AXISX);
		actions.PushBack(aJOYEVENT_AXIS_2_X);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, lookXAxis, actions);

		actions.Clear();
		actions.PushBack(aMOUSEEVENT_AXISY);
		actions.PushBack(aJOYEVENT_AXIS_2_Y);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, lookYAxis, actions);

		actions.Clear();
		actions.PushBack(aKEYEVENT_ESCAPE);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, quitAPP, actions, MashInputEventFunctor(&MainLoop::QuitApp, this));

		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("BaseMaterial.mtl");
		MashMaterial *baseMaterial = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("BaseMaterial");
		if (!baseMaterial)
			return true;

		m_root = m_device->GetSceneManager()->AddDummy(0, "SceneRoot");

		MashCamera *camera = (MashCamera*)m_device->GetSceneManager()->AddCamera(m_root, "Camera01");
		camera->SetZFar(1000);
		camera->SetZNear(0.1f);
		camera->SetPosition(MashVector3(0, 10, 0));

		MashLight *light = (MashLight*)m_device->GetSceneManager()->AddLight(m_root, "Light", aLIGHT_DIRECTIONAL, aLIGHT_RENDERER_FORWARD, true);
		light->SetDefaultDirectionalLightSettings(MashVector3(-1.0f,-0.0f,0.0f));
		light->SetAmbient(sMashColour4(0.5f, 0.5f, 0.5f, 1.0f));
		light->SetRange(1000);
		light->SetShadowsEnabled(false);

		//Create some scene objects
		MashMesh *cubeMesh = m_device->GetSceneManager()->CreateStaticMesh();
		m_device->GetSceneManager()->GetMeshBuilder()->CreateCube(cubeMesh, 5, 5, 5, baseMaterial->GetVertexDeclaration());
		MashModel *cubeModel = m_device->GetSceneManager()->CreateModel();
		cubeModel->Append(&cubeMesh);
		cubeMesh->Drop();
		MashEntity *mainCharacter = m_device->GetSceneManager()->AddEntity(m_root, cubeModel, "CubeEntity");
		cubeModel->Drop();
		mainCharacter->SetMaterialToAllSubEntities(baseMaterial);

		for(uint32 i = 0; i < 40; ++i)
		{
			//instance some random scene objects
			MashEntity *newInstance = (MashEntity*)m_device->GetSceneManager()->AddInstance(mainCharacter, m_root, "Instance");
            
			int32 areaHalfWidth = 200;
			int32 areaHalfLength = 200;
			newInstance->SetPosition(MashVector3(math::RandomInt(-areaHalfWidth, areaHalfWidth), 
				0, math::RandomInt(-areaHalfLength, areaHalfLength)));
		}

		/*
			This will be the keymapping used in our script.
			The string is what we will use in the script to access the key value.
			The int is the input action we created above.
		*/
		sMashLuaKeyValue luaInputVals[] = {
			{"moveXAxis", moveXAxis},
			{"moveYAxis", moveYAxis},
			{"lookXAxis", lookXAxis},
			{"lookYAxis", lookYAxis},
			{0, 0}
		};
		m_device->GetScriptManager()->SetLibInputValues(luaInputVals);

		/*
			This creates a custom function for our script.
			The string is what we will use in the script to call the function.
			And the function pointer is the C callback function. 
		*/
		sMashLuaUserFunction luaExtraFunctions[] = {
			{"rotateVectorToNode", RotateVectorToNodeLuaCallback},
			{0, 0}
		};
		m_device->GetScriptManager()->SetLibUserFunctions(luaExtraFunctions);

		/*
			Loading a script is easy, Just load it from a file and assign it to a node.

			The same script can be assigned to multiple nodes. However, if the script holds/loads
			unique node data then you will need to load the script again and assign it to the next node.
		*/
		MashLuaScriptWrapper *newScript = m_device->GetScriptManager()->CreateLuaScript("CameraMovementScript.lua");
		MashSceneNodeScriptHandler *scriptHandler = m_device->GetScriptManager()->CreateSceneNodeScriptHandler(newScript);
		camera->AddCallback(scriptHandler);
		/*
			Drop our copy, the scene node now owns it.
			We don't need to drop the script because that's done for us in the manager.
		*/
		scriptHandler->Drop();

		//Sets a global value in the script called "playerControllerId" to a given value
		newScript->GetScript()->SetGlobalInt("playerControllerId", m_playerControl);

		//set up gui
		MashGUIRect fpsRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f),
			MashGUIUnit(0.0f, 100.0f), MashGUIUnit(0.0f, 20.0f));
		m_fpsText = m_device->GetGUIManager()->AddStaticText(fpsRegion, 0);

		MashGUIRect textRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 20.0f),
			MashGUIUnit(0.0f, 300.0f), MashGUIUnit(0.0f, 120.0f));
		MashGUIStaticText *staticText = m_device->GetGUIManager()->AddStaticText(textRegion, 0);

		MashStringc text = "\Controls :\n'W' or 'joy axis 1 Y' walk forward\n\
'S' or 'joy axis 1 Y' walk backwards\n\
'A' or 'joy axis 1 X' strafe left\n\
'D' 'joy axis 1 X' strafe right\n\
'mouse axis' or 'joy axis 2' look around";

		staticText->SetText(text);

		return false;
	}

	bool Update(f32 dt)
	{
		m_device->GetSceneManager()->UpdateScene(dt, m_root);
		return m_quit;
	}

	void Render()
	{
		m_device->GetRenderer()->SetRenderTargetDefault();

		m_device->GetSceneManager()->CullScene(m_root);
		m_device->GetSceneManager()->DrawScene();

		int8 buffer[256];
        helpers::PrintToBuffer(buffer, sizeof(buffer), "FPS : %d", m_device->GetFPS());
		m_fpsText->SetText(buffer);

		m_device->GetGUIManager()->BeginDraw();
		m_device->GetGUIManager()->DrawAll();
        m_device->GetGUIManager()->EndDraw();
	}

	void QuitApp(const sInputEvent &e)
    {
		m_quit = true;
	}
};

MashDevice *MainLoop::m_device = 0;

int main()
{
	sMashDeviceSettings deviceSettings;
#ifdef USE_DIRECTX
	deviceSettings.rendererFunctPtr = CreateMashD3D10Device;
#else
    deviceSettings.rendererFunctPtr = CreateMashOpenGL3Device;
#endif
	deviceSettings.guiManagerFunctPtr = CreateMashGUI;
	deviceSettings.scriptManagerFunctPtr = CreateMashScriptManager;

    deviceSettings.fullScreen = false;
    deviceSettings.screenWidth = 800;
    deviceSettings.screenHeight = 600;
    deviceSettings.enableVSync = false;
	deviceSettings.preferredLightingMode = aLIGHT_TYPE_PIXEL; 
    deviceSettings.antiAliasType = aANTIALIAS_TYPE_X4;

    deviceSettings.rootPaths.PushBack("../../../../../Media/GUI");
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");
    deviceSettings.rootPaths.PushBack("../../../../DemoMedia/ScriptDemo");

	MashDevice *device = CreateDevice(deviceSettings);

	if (!device)
		return 1;

	/*
		Normally you would want to do this for most application and just
		render your own cursor using the GUI library.
	*/
    //device->LockMouseToScreenCenter(true);
    //device->HideMouseCursor(true);

	device->SetWindowCaption("Script Demo");

	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();

	device->Drop();
    
    return 0;
}
