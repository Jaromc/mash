
#include "MashInclude.h"

#include "MemoryAllocator/MashDefaultMemoryAllocator.h"
#include "D3D10/MashD3D10Creation.h"
#include "OpenGL3/MashOpenGL3Creation.h"

#if defined (MASH_WINDOWS) && !defined(__MINGW32__)
    #define USE_DIRECTX
#endif

using namespace mash;

/*
	Triangle colliders can be created using SceneManager::CreateTriangleCollection
*/
class MainLoop : public MashGameLoop
{
private:
	MashDevice *m_device;

	MashSceneNode *m_root;
	uint32 m_playerControl;
	MashCamera *m_camera;
	MashDecal *m_staticDecalBatch;
	MashDecal *m_skinnedDecalBatch;
    
    MashGUIStaticText *m_staticText;
	MashGUIStaticText *m_fpsText;
public:
	MainLoop(MashDevice *device):m_device(device){}
	virtual ~MainLoop(){}

	bool Initialise()
	{
		m_playerControl = m_device->GetInputManager()->CreatePlayer();
		const MashStringc inputContext = "play";
		m_device->GetInputManager()->CreateDefaultActionMap(m_playerControl,inputContext);
		m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 0);

		m_device->GetInputManager()->SetPlayerActionCallback(m_playerControl, inputContext, aINPUTMAP_FIRE, 
			MashInputEventFunctor(&MainLoop::FireDecal, this));

		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("StaticMeshMaterial.mtl");
		MashMaterial *staticMeshMtrl = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("StaticMeshMaterial");
		if (!staticMeshMtrl)
			return true;

		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("SkinnedMeshMaterial.mtl");
		MashMaterial *skinnedMeshMtrl = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("SkinnedMeshMaterial");
		if (!skinnedMeshMtrl)
			return true;

		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("SkinnedDecalMaterial.mtl");
		MashMaterial *skinnedDecalMtrl = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("SkinnedDecalMaterial");
		if (!skinnedDecalMtrl)
			return true;

		MashList<mash::MashSceneNode*> rootNodeList;
		sLoadSceneSettings loadSettings;
		loadSettings.createRootNode = false;
		loadSettings.saveGeometryFlags = 0;
		if (m_device->GetSceneManager()->LoadSceneFile("DecalScene.nss", rootNodeList, loadSettings) == aMASH_FAILED)
			return true;

		//hold onto this for later
		MashSceneNode *sceneMeshOnly = rootNodeList.Front();

		m_root = m_device->GetSceneManager()->AddDummy(0, "SceneRoot");
		m_root->AddChild(sceneMeshOnly);	

		rootNodeList.Clear();
		if (m_device->GetSceneManager()->LoadSceneFile("AnimatedCharacter.nss", rootNodeList, loadSettings) == aMASH_FAILED)
			return true;
		
		MashSceneNode *animatedCharacterScene = rootNodeList.Front();
		animatedCharacterScene->SetScale(MashVector3(5.0f, 5.0f, 5.0f), true);
		m_root->AddChild(animatedCharacterScene);
		MashEntity *animatedCharacterEntity = (MashEntity*)animatedCharacterScene->GetChildByName("Character");
		
		MashAnimationMixer *mixer = animatedCharacterScene->GetAnimationMixer();
		if (mixer)
		{
			mixer->Transition("idle");
			mixer->SetWrapMode("idle", aWRAP_LOOP);
		}

		m_camera = (MashCamera*)m_device->GetSceneManager()->AddCamera(m_root, "Camera");
		m_camera->SetZFar(4500);
		m_camera->SetZNear(1.0f);
		m_camera->SetPosition(MashVector3(0, 100, -500));
		m_camera->SetLookAtDirection(MashVector3(-1, -0.5, 0));
        
		//Add some extra lighting to the scene other than just the spot light
        MashLight *light = (MashLight*)m_device->GetSceneManager()->AddLight(m_root, "Light", aLIGHT_DIRECTIONAL, aLIGHT_RENDERER_FORWARD, false);
		MashVector3 lightDir(-1.0f,-0.5f,0.0f);
		lightDir.Normalize();
		light->SetDefaultDirectionalLightSettings(lightDir);
		light->SetAmbient(sMashColour4(0.0f,0.0f,0.0f,1.0f));
		light->SetDiffuse(sMashColour4(0.5f,0.5f,0.5f,1.0f));
		light->SetSpecular(sMashColour4(0.0f,0.0f,0.0f,1.0f));
		light->SetRange(5000);
        light->SetShadowsEnabled(false);

		/*
			Load shadow caster data from a xml file. Alternatively you can create shadow casters in
			code using:
			
			MashSpotShadowCaster *shadowCaster = m_device->GetSceneManager()->CreateSpotShadowCaster();
			m_device->GetSceneManager()->SetShadowCaster(aLIGHT_SPOT, shadowCaster);
			shadowCaster->Drop();//the manager now owns it
		*/
		m_device->GetSceneManager()->LoadShadowCastersFromFile("ShadowCasterData.xml");

		MashLight *spotLight = (MashLight*)m_device->GetSceneManager()->AddLight(m_camera, "SpotLight", aLIGHT_SPOT, aLIGHT_RENDERER_FORWARD, true);
		lightDir = MashVector3(0.0f,0.0f,1.0f);
		lightDir.Normalize();
        spotLight->SetPosition(MashVector3(200,-100,0));
		spotLight->SetDefaultSpotLightSettings(lightDir);
		spotLight->SetRange(5000);
		spotLight->SetOuterCone(math::DegsToRads(30.0f));
		spotLight->SetInnerCone(math::DegsToRads(20.0f));
		spotLight->SetAttenuation(0.0, 0.00, 1);
        spotLight->SetShadowsEnabled(true);

		//only collide against the scene, not the animated character
		MashEllipsoidColliderController *colliderController = m_device->GetSceneManager()->CreateEllipsoidColliderController(m_camera, sceneMeshOnly, MashVector3(100.0f, 100.0f, 100.0f), MashVector3(0.0f, 0.0f, 0.0f));
		MashFreeMovementController *freeCameraController = m_device->GetSceneManager()->CreateFreeMovementController(m_playerControl, inputContext, 0.1f, 500.0f);

		/*
			Order of controllers can be important depending on the desired result. For example, in this case we
			process any input and movement before handling the collisions.
		*/
		m_camera->AddCallback(colliderController, 0);
        m_camera->AddCallback(freeCameraController, 1);
		
		//drop the colliders because the node now owns them
        freeCameraController->Drop();
		colliderController->Drop();

		/*
			Mash has builtin materials to handle non-skinned decals. Decals made easy!
			For decals requiring skinning, custom decal materials must be created.
		*/
		m_staticDecalBatch = m_device->GetSceneManager()->AddDecal(m_root, "StaticDecalBatch", aDECAL_STANDARD, 20, aLIGHT_TYPE_PIXEL);
        m_staticDecalBatch->GetMaterial()->SetTexture(0, m_device->GetRenderer()->GetTexture("DecalTexture.DDS"));

		/*
			Skinned decals are not transformed into world space on creation.
			They are transformed per frame on the gpu. Therefore the decal needs to be
			attached to its owner so it has the correct transform.

			It also ensures the decals are culled correctly.
		*/
		m_skinnedDecalBatch = m_device->GetSceneManager()->AddDecalCustom(animatedCharacterEntity, "SkinnedDecalBatch", skinnedDecalMtrl, 20, animatedCharacterEntity->GetSkin());
        
        //set up gui
		MashGUIRect fpsRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f),
                             MashGUIUnit(0.0f, 100.0f), MashGUIUnit(0.0f, 20.0f));
		m_fpsText = m_device->GetGUIManager()->AddStaticText(fpsRegion, 0);
        
		MashGUIRect textRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 20.0f),
                              MashGUIUnit(0.0f, 400.0f), MashGUIUnit(0.0f, 120.0f));

		MashGUIStaticText *staticText = m_device->GetGUIManager()->AddStaticText(textRegion, 0);
        
		MashStringc text= "\Controls :\nW,S move forward/backward\n\
A,D move left/right\n\
MB 1 shoot\n\
Hold MB 2 then move the mouse to look around";
        
		staticText->SetText(text);

		return false;
	}

	bool Update(f32 dt)
	{
		m_device->GetSceneManager()->UpdateScene(dt, m_root);
		return false;
	}

	void Render()
	{
		m_device->GetSceneManager()->CullScene(m_root);
		m_device->GetSceneManager()->DrawScene();
        
        int8 buffer[256];
        mash::helpers::PrintToBuffer(buffer, sizeof(buffer), "FPS : %d", m_device->GetFPS());
		m_fpsText->SetText(buffer);
        
		m_device->GetGUIManager()->BeginDraw();
		m_device->GetGUIManager()->DrawAll();
		m_device->GetGUIManager()->EndDraw();
	}

	void FireDecal(const sInputEvent &e)
	{
		if (e.isPressed == 0)
		{
			MashRay cameraRay(m_camera->GetRenderTransformState().translation, m_camera->GetTarget() - m_camera->GetRenderTransformState().translation);
			uint32 nodesToSelect = aNODETYPE_ENTITY;

			sTriPickResult pickResult;
			MashScenePick scenePick;

			if (scenePick.GetClosestTriFromScene(m_root,
				cameraRay,
				nodesToSelect,
				false,
				pickResult))
			{
				f32 decalRotation = math::RandomFloat(0.0f, 6.0f);
				
				/*
					This code assumes nodes arn't moving in the scene.

					If you want to add decals to nodes that can move freely around a scene,
					Decals will need to be transformed per frame based on the node that added it.
					It's most likley there will be one decal batch per node, similar to the skinned example.
					However with some thought it may be possible to batch many moveable nodes decals into a 
					single batch. Possibly via a matrix array on the GPU?
				*/
				if (((MashEntity*)pickResult.node)->GetSkin())
				{
					MashVector2 decalSize(2,2);
					/*
						This can only handle decals for one skeleton.
					*/
					m_skinnedDecalBatch->AppendVertices(pickResult.node->GetTriangleCollider(),
						pickResult, decalSize, decalRotation, 0);
				}
				else
				{
					/*
						The static scene is scaled down greatly. So the decal size needs to be big.
						This sort of problem can be handled many different ways. This is the easy option.
					*/
					MashVector2 decalSize(50,50);

					/*
						This can handle all static objects in the scene. This is the most efficent way of handling decals.
					*/
                    MashMatrix4 collidedModelTransform = pickResult.node->GetWorldTransformState().ToMatrix();
					m_staticDecalBatch->AppendVertices(pickResult.node->GetTriangleCollider(),
						pickResult, decalSize, decalRotation, &collidedModelTransform);
				}
			}
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
	deviceSettings.screenWidth = 800;
	deviceSettings.screenHeight = 600;
	deviceSettings.enableVSync = false;
    deviceSettings.preferredLightingMode = aLIGHT_TYPE_PIXEL;
    deviceSettings.antiAliasType = aANTIALIAS_TYPE_X4;

    deviceSettings.rootPaths.PushBack("../../../../../Media/GUI");
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");
    deviceSettings.rootPaths.PushBack("../../../../DemoMedia/DecalDemo");

	MashDevice *device = CreateDevice(deviceSettings);

	if (!device)
		return 1;

	device->SetWindowCaption("Decal Demo");

	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();

	device->Drop();
    
    return 0;
}
