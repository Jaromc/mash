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

/*
 The following snippet shows how to chop a scene graphs animtion buffer
 manually. You might do this if your models were exported from max with
 all animations in one timeline. Ie, from 0 - 500 contains, walk, run, punch, etc...
 
 An entire scene graph can be passed into the chop function.
 This graph may be an entity and all its bones. Or you can just pass it a node
 at a time if needed.
 
 Once a buffer has been chopped, the original cannot be recovered.
 */
/*
 MashSceneNode *m_root = graph of some entity/scene including its bones
 
 sAnimationClip idle;
 idle.name = "idle";
 idle.start = 297;
 idle.end = 520;
 
 sAnimationClip walk;
 walk.name = "walk";
 walk.start = 10;
 walk.end = 104;
 
 sAnimationClip leanleft;
 leanleft.name = "leanleft";
 leanleft.start = 160;
 leanleft.end = 200;
 
 sAnimationClip leanright;
 leanright.name = "leanright";
 leanright.start = 115;
 leanright.end = 155;
 
 std::vector<sAnimationClip> clips;
 clips.push_back(idle);
 clips.push_back(leanleft);
 clips.push_back(leanright);
 clips.push_back(walk);
 
 //must be called before the mixer is created
 m_device->GetSceneManager()->GetControllerManager()->ChopAnimationBuffers(m_root, clips);

 
 m_device->GetSceneManager()->GetControllerManager()->CreateMixer(m_root);
 m_animationMixer = m_root->GetAnimationMixer();
 */
/*
 Note, in this demo, the entity and its bones are children of a root node.
 This root node is what needs to be transformed if transformation is needed.
 The entity and bones need to move together otherwise the mesh will deform in
 the wrong way.
 
 Mesh Lodding is fairly easy. This mesh in this demo has been created with 2 lods.
 1st lod is the main, high poly mesh. 2nd lod is a lower poly mesh.
 All we do here is set at what distances from the camera the lods start.
 The first lod always starts at a distance of 0.
 Eg, m_animatedEntity->SetLodDistance(1, 50). So this sets the second lod to start
 at a distance of 50 units from the camera.
 
 Lodding, animation, material selection, and lighting have already be applied offline
 in the scene editor.	
 */
class MainLoop : public MashGameLoop
{
private:
	MashDevice *m_device;
    
	MashSceneNode *m_root;
	MashEntity *m_animatedEntity;
	MashSceneNode *m_batAttachPoint;
	MashAnimationMixer *m_animationMixer;
	uint32 m_playerControl;
	f32 m_leanFrame;
	int32 m_leanAnimationLength;
	f32 m_addToCameraPosition;
    
	MashCamera *m_camera;
    
	MashGUIStaticText *m_staticText;
	MashGUIStaticText *m_fpsText;
	MashGUIStaticText *m_footStepText;
	f32 m_footStepSoundTimer;
public:
	MainLoop(MashDevice *device):m_device(device), m_leanFrame(0), m_footStepSoundTimer(0.0f), m_addToCameraPosition(0.0f){}
	virtual ~MainLoop(){}
    
	bool Initialise()
	{
		m_playerControl = m_device->GetInputManager()->CreatePlayer();
		const MashStringc inputContext = "play";
		m_device->GetInputManager()->SetCurrentPlayerContext(m_playerControl, inputContext);
		m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 0);//keyboard/mouse
		
		if (m_device->GetInputManager()->GetControllerCount() > 1)
			m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 1);//joystick
        
		MashArray<eINPUT_EVENT> actions;
		actions.PushBack(aKEYEVENT_W);
		actions.PushBack(aKEYEVENT_S);
		actions.PushBack(aJOYEVENT_AXIS_1_Y);
		m_device->GetInputManager()->SetEventValueSign(aKEYEVENT_S, true);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION, actions, MashInputEventFunctor(&MainLoop::BlendIntoWalk, this));
        
		actions.Clear();
		actions.PushBack(aKEYEVENT_SPACE);
		actions.PushBack(aJOYEVENT_B6);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION+1, actions, MashInputEventFunctor(&MainLoop::AttachBat, this));
        
		/*
         Here we set the lean controls.
         We emulate joystick controls with the keyboard by setting the 'c' key
         to return a negative value when pressed, and the 'v' key will return a
         positive value by default. Remember keys are either on or off (0 or 1)
         so we won't get a smooth lean transition using keys like we will using
         a joystick or gamepad.
         */
		actions.Clear();
		actions.PushBack(aKEYEVENT_Q);
		actions.PushBack(aKEYEVENT_E);
		actions.PushBack(aJOYEVENT_AXIS_2_X);
		m_device->GetInputManager()->SetEventValueSign(aKEYEVENT_Q, true);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION+2, actions, MashInputEventFunctor(&MainLoop::LeanCharacter, this));
        
		actions.Clear();
		actions.PushBack(aKEYEVENT_Z);
		actions.PushBack(aKEYEVENT_X);
		actions.PushBack(aJOYEVENT_THROTTLE_1);
		actions.PushBack(aJOYEVENT_THROTTLE_2);
		m_device->GetInputManager()->SetEventValueSign(aKEYEVENT_Z, true);
		m_device->GetInputManager()->SetEventValueSign(aJOYEVENT_THROTTLE_1, true);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION+3, actions, MashInputEventFunctor(&MainLoop::MoveCamera, this));
		
		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("BaseMaterial.mtl");
		MashMaterial *baseMtrl = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("BaseMaterial");
		if (!baseMtrl)
			return true;
        
		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("SkinningMaterial.mtl");
		MashMaterial *skinningMtrl = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("SkinningMaterial");
		if (!skinningMtrl)
			return true;
        
        m_root = m_device->GetSceneManager()->AddDummy(0, "RootNode");
        
		MashList<mash::MashSceneNode*> rootNodeList;
		sLoadSceneSettings loadSettings;
		loadSettings.createRootNode = false;
		loadSettings.saveGeometryFlags = 0;
		m_device->GetSceneManager()->LoadSceneFile("AnimatedLodCharacter.nss", rootNodeList, loadSettings);
		MashSceneNode *chararacterRoot = rootNodeList.Front();
        m_root->AddChild(chararacterRoot);	
        
		m_animatedEntity = (MashEntity*)m_root->GetChildByName("Character");
		//Note the animation mixer is not within the entity, but its parent.
		m_animationMixer = chararacterRoot->GetAnimationMixer();
        
		m_animationMixer->SetWrapMode("idle", aWRAP_LOOP);
		m_animationMixer->SetBlendMode("idle", aBLEND_BLEND);
		m_animationMixer->Transition("idle", 0.0f);
        
		m_animationMixer->SetWrapMode("walk", aWRAP_LOOP);
		m_animationMixer->SetBlendMode("walk", aBLEND_BLEND);
        
		m_animationMixer->SetWrapMode("leanleft", aWRAP_CLAMP);
		m_animationMixer->SetBlendMode("leanleft", aBLEND_ADDITIVE);
		m_animationMixer->SetTrack("leanleft", 1);
		m_animationMixer->SetWeight("leanleft", 1.0f);
		
		m_animationMixer->SetWrapMode("leanright", aWRAP_CLAMP);
		m_animationMixer->SetBlendMode("leanright", aBLEND_ADDITIVE);
		m_animationMixer->SetTrack("leanright", 1);
		m_animationMixer->SetWeight("leanright", 1.0f);
        
		rootNodeList.Clear();
		m_device->GetSceneManager()->LoadSceneFile("CharacterBat.nss", rootNodeList, loadSettings);
        
		m_batAttachPoint = rootNodeList.Front()->GetChildByName("BatAttachPoint");
		m_root->GetChildByName("RightHandAttachPoint")->AddChild(m_batAttachPoint);
        
		m_camera = (MashCamera*)m_device->GetSceneManager()->AddCamera(m_root, "Camera");
		m_camera->SetZFar(500);
		m_camera->SetZNear(1.0f);
		m_camera->SetPosition(MashVector3(30, 0, -50));
		m_camera->SetLookAtNode(true, m_animatedEntity);
        
		/*
			Load shadow caster data from a xml file. Alternatively you can create shadow casters in
			code using:
			
			MashDirectionalShadowCascadeCaster *shadowCaster = m_device->GetSceneManager()->CreateDirectionalCascadeShadowCaster();
			m_device->GetSceneManager()->SetShadowCaster(aLIGHT_DIRECTIONAL, shadowCaster);
			shadowCaster->Drop();//the manager now owns it
		*/
		//m_device->GetSceneManager()->LoadShadowCastersFromFile("ShadowCasterData.xml");
        
		MashLight *light = (MashLight*)m_device->GetSceneManager()->AddLight(m_root, "Light", aLIGHT_DIRECTIONAL, aLIGHT_RENDERER_FORWARD, true);
		light->SetDefaultDirectionalLightSettings(MashVector3 (-1.0f,0.0f,0.0f));
		light->SetRange(1000);
		light->SetShadowsEnabled(false);
        
		//lean left and right are the same length
		m_leanAnimationLength = m_animationMixer->GetFrameLength("leanright");
		//set animation callbacks for walking sound
		m_animationMixer->SetCallbackHandler(MashAnimationEventFunctor(&MainLoop::AnimationFrameCallback, this));
		//these are the frames the feet hit the ground when walking forward
		m_animationMixer->SetCallbackTrigger("walk", 25, 0);
		m_animationMixer->SetCallbackTrigger("walk", 65, 0);
        
		//create a plane so the model has something to cast a shadow on
		MashMesh *planeMesh = m_device->GetSceneManager()->CreateStaticMesh();
		m_device->GetSceneManager()->GetMeshBuilder()->CreatePlane(planeMesh, 1, 100, 100, baseMtrl->GetVertexDeclaration(), 
                                                                   MashVector3(1,0,0), MashVector3(-30,0,0));
		MashModel *planeModel = m_device->GetSceneManager()->CreateModel();
		planeModel->Append(&planeMesh);
		planeMesh->Drop();
		MashEntity *planeEntity = m_device->GetSceneManager()->AddEntity(m_root, planeModel, "PlaneEntity");
		planeModel->Drop();
		planeEntity->SetMaterialToAllSubEntities(baseMtrl);
        
		//set up gui
		MashGUIRect fpsRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f),
                             MashGUIUnit(0.0f, 100.0f), MashGUIUnit(0.0f, 20.0f));
		m_fpsText = m_device->GetGUIManager()->AddStaticText(fpsRegion, 0);
        
		MashGUIRect textRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 20.0f),
                              MashGUIUnit(0.0f, 300.0f), MashGUIUnit(0.0f, 120.0f));
		m_staticText = m_device->GetGUIManager()->AddStaticText(textRegion, 0);
        
		MashStringc text= "Controls :\n'W,S' or 'joy axis 1' walk forward/backward\n\
'Q,E' or 'joy axis 2' lean left/right\n\
'Z,X' 'joy throttles' Zoom in/out\n\
'SPACE' or 'button A' pickup/drop weapon";
        
		m_staticText->SetText(text);
        
		MashGUIRect tapTextRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 120.0f),
                                 MashGUIUnit(0.0f, 100.0f), MashGUIUnit(0.0f, 140.0f));
		m_footStepText = m_device->GetGUIManager()->AddStaticText(tapTextRegion, 0);
		m_footStepText->SetText("Tap");
        
		return false;
	}
    
	bool Update(f32 dt)
	{
		if (m_addToCameraPosition != 0.0f)
		{
			m_camera->AddPosition(MashVector3(0.0f, 0.0f, 100.0f * m_addToCameraPosition * dt));
		}	

		m_device->GetSceneManager()->UpdateScene(dt, m_root);

		//only show the tab text for a short period of time to simulate sound
		if (m_footStepText->GetRenderEnabled() == true)
		{
			m_footStepSoundTimer -= dt;
			if (m_footStepSoundTimer < 0.0f)
				m_footStepText->SetRenderEnabled(false);
		}
		
		return false;
	}
    
	void Render()
	{
		m_device->GetRenderer()->SetRenderTargetDefault();
        
		m_device->GetSceneManager()->CullScene(m_root);
		m_device->GetSceneManager()->DrawScene();
        
		m_device->GetSceneManager()->DrawAABB(m_animatedEntity->GetWorldBoundingBox(), sMashColour(255,255,255,255));
        
		int8 buffer[256];
        helpers::PrintToBuffer(buffer, sizeof(buffer), "FPS : %d", m_device->GetFPS());
		m_fpsText->SetText(buffer);
        
		m_device->GetGUIManager()->BeginDraw();
		m_device->GetGUIManager()->DrawAll();
		m_device->GetGUIManager()->EndDraw();
        
	}
    
	void BlendIntoWalk(const sInputEvent &e)
	{
		f32 threshold = 0.01f;
		if ((e.value <= threshold) && (e.value >= -e.value))
		{
			m_animationMixer->Transition("idle");
		}
		else
		{
			m_animationMixer->Transition("walk");
            
			if (e.value > threshold)
				m_animationMixer->SetReverse("walk", false);
			else
				m_animationMixer->SetReverse("walk", true);
			
		}
	}
    
	void AttachBat(const sInputEvent &e)
	{
		/*
         Attaches the bat to the characters hand.
         The character has a bone in the center of its hand which we use to
         attach the bat to.
         
         The bat is parented by a dummy node at the handle which is the attachment point.
         */
		if (e.isPressed)
		{
			//simple check to see what the bat is currently attached to
			if (m_batAttachPoint->GetParent() != m_root)
			{
				//drop the bat to the ground
				m_batAttachPoint->Detach();
				m_batAttachPoint->SetPosition(MashVector3(-13.0f, -14.0f, 0.0f), true);
				//need to attach it to the root node so it still gets updated and rendered
				m_root->AddChild(m_batAttachPoint);
			}
			else
			{
				//send the bat back to the origin so the bat moves with the hand correctly
				m_batAttachPoint->Detach();//from root node
				m_batAttachPoint->SetPosition(MashVector3(0.0f, 0.0f, 0.0f), true);
				m_root->GetChildByName("RightHandAttachPoint")->AddChild(m_batAttachPoint);
			}
		}
	}
    
	void LeanCharacter(const sInputEvent &e)
	{
		m_leanFrame = e.value * m_leanAnimationLength;
		m_animationMixer->SetFrame("leanleft", -m_leanFrame);
		m_animationMixer->SetFrame("leanright", m_leanFrame);
	}
    
	void AnimationFrameCallback(const sAnimationEvent &e)
	{
		//pseudocide example use
		/*
            if walking on wood then
                play wood noise
            else if walking on sand then
                play sand noise
        */
		m_footStepText->SetRenderEnabled(true);
		m_footStepSoundTimer = 0.5f;
	}
    
	void MoveCamera(const sInputEvent &e)
	{
		m_addToCameraPosition = e.value;
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
	deviceSettings.fixedTimeStep = 1.0/30.0;
    
    deviceSettings.rootPaths.PushBack("../../../../../Media/GUI");
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");
    deviceSettings.rootPaths.PushBack("../../../../DemoMedia/AnimationDemo");
    
	MashDevice *device = CreateDevice(deviceSettings);
    
	if (!device)
		return 1;
    
	device->SetWindowCaption("Animation Demo");
    
	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();
    
	device->Drop();
    
    return 0;
}
