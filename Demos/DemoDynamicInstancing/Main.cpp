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

enum eBATCH_MODE
{
	BMODE_NO_BATCH,
	BMODE_BONE_ARRAY,
	BMODE_TEXTURE
};

eBATCH_MODE g_batchMode = BMODE_NO_BATCH;
const uint32 g_maxApplicationInstanceCount = 200;
//these values match those found in the shader files for this technique
const uint32 g_boneCount = 26;
const uint32 g_instanceIncrementCount = 5;
uint32 g_currentInstanceCount = 1;
uint32 g_drawCalls = 0;
uint32 g_currentBoneBufferInstanceCount = 0;

//for bone texture technique
MashVector2 g_boneTextureDim;

//for bone array technique
const uint32 g_maxBoneArrayInstanceCount = 5;//This value matches a define found in the skinning shader
MashMatrix4 *g_instanceBoneArray = 0;

void ApplyMaterialToAllNodes(MashSceneNode *node, MashMaterial *material)
{
	if (node->GetNodeType() & aNODETYPE_ENTITY)
		((MashEntity*)node)->SetMaterialToAllSubEntities(material);

	MashList<MashSceneNode*>::ConstIterator iter = node->GetChildren().Begin();
	MashList<MashSceneNode*>::ConstIterator end = node->GetChildren().End();
	for(; iter != end; ++iter)
		ApplyMaterialToAllNodes(*iter, material);
}

class BatchBonePaletteEffectAuto : public MashAutoEffectParameter
{
public:
	BatchBonePaletteEffectAuto():MashAutoEffectParameter(){}
	~BatchBonePaletteEffectAuto(){}

	void OnSet(const MashRenderInfo *renderInfo, 
		MashEffect *effect, 
		MashEffectParamHandle *parameter,
		uint32 index)
	{
		effect->SetMatrix(parameter, g_instanceBoneArray, g_currentBoneBufferInstanceCount * g_boneCount);
	}

	const int8* GetParameterName()const{return "batchedBonePalette";}
};

class BoneTextureDimEffectAuto : public MashAutoEffectParameter
{
public:
	BoneTextureDimEffectAuto():MashAutoEffectParameter(){}
	~BoneTextureDimEffectAuto(){}

	void OnSet(const MashRenderInfo *renderInfo, 
		MashEffect *effect, 
		MashEffectParamHandle *parameter,
		uint32 index)
	{
		effect->SetVector2(parameter, &g_boneTextureDim);
	}

	const int8* GetParameterName()const{return "boneTextureDim";}
};

//bit twiddling hacks
uint32 GetNextPowerOfTwo(uint32 i)
{
	--i;
	i |= i >> 1;
	i |= i >> 2;
	i |= i >> 4;
	i |= i >> 8;
	i |= i >> 16;
	i++;
	return i;
}


/*
	Note these batches assume the same mesh will pass through them each time.
	For an entity that has multiple meshes for lods and multisubs then you
	would need to handle this differently.
*/
class BoneArrayBatchRenderPath : public MashCustomRenderPath
{
private:
	MashDevice *m_device;
	MashRenderInfo *m_renderInfo;

	MashMaterial *m_material;
	MashMesh *m_originalEntityMesh;
	MashMeshBuffer *m_batchedBuffer;
	f32 *m_instanceBufferData;
public:
	BoneArrayBatchRenderPath(MashDevice *device, MashMeshBuffer *batchedBuffer, MashMesh *originalEntityMesh, MashMaterial *material):m_device(device), m_batchedBuffer(batchedBuffer), 
		m_material(material), m_originalEntityMesh(originalEntityMesh), m_instanceBufferData(0)
	{
		m_batchedBuffer->Grab();

		//array used for bones
		g_instanceBoneArray = MASH_ALLOC_T_COMMON(MashMatrix4, g_maxBoneArrayInstanceCount * g_boneCount);
		//setup custom bone palette effect auto
        BatchBonePaletteEffectAuto *effAuto = MASH_NEW_COMMON BatchBonePaletteEffectAuto();
		device->GetRenderer()->GetMaterialManager()->RegisterAutoParameterHandler(effAuto);
        effAuto->Drop();

		m_renderInfo = m_device->GetRenderer()->GetRenderInfo();
	}

	~BoneArrayBatchRenderPath()
	{
		if (m_batchedBuffer)
		{
			m_batchedBuffer->Drop();
			m_batchedBuffer = 0;
		}

		if (g_instanceBoneArray)
		{
			MASH_FREE(g_instanceBoneArray);
			g_instanceBoneArray = 0;
		}
	}

	void AddObject(MashRenderable *pRenderable)
	{
		//force a flush if we hit the max instance count
		if (g_currentBoneBufferInstanceCount == g_maxBoneArrayInstanceCount)
			Flush();

		//copy instance bone data into our big batched bone array
		uint32 boneStart = g_currentBoneBufferInstanceCount * g_boneCount;
		//renderInfo stores all the insformation about the current scene. So we can get the bone array from there.
		memcpy(&g_instanceBoneArray[boneStart], m_renderInfo->GetBonePalette(), sizeof(MashMatrix4) * m_renderInfo->GetCurrentBoneCount());
		//set the instance id
		m_instanceBufferData[g_currentBoneBufferInstanceCount] = g_currentBoneBufferInstanceCount;
		++g_currentBoneBufferInstanceCount;
	}

	void Flush()
	{
		//unlock the buffer so we can render
		UnlockBuffers();

		if (m_material->OnSet() == aMASH_OK)
		{
			m_device->GetRenderer()->DrawIndexedInstancedList(m_batchedBuffer, 
				m_originalEntityMesh->GetVertexCount(),
				m_originalEntityMesh->GetIndexCount(),
				m_originalEntityMesh->GetPrimitiveCount(),
				m_originalEntityMesh->GetPrimitiveType(),
				g_currentBoneBufferInstanceCount);

			++g_drawCalls;
		}

		//lock the buffers for writing again
		LockBuffers();
	}

	void LockBuffers()
	{
		if (!m_instanceBufferData)
		{
			//lock the buffers for writing
			m_batchedBuffer->GetVertexBuffer(1)->Lock(aLOCK_WRITE_DISCARD, (void**)&m_instanceBufferData);
			g_currentBoneBufferInstanceCount = 0;
		}
	}

	void UnlockBuffers()
	{
		if (m_instanceBufferData)
		{
			//unlock the buffer so we can render
			m_batchedBuffer->GetVertexBuffer(1)->Unlock();
			m_instanceBufferData = 0;
		}
	}
};

class TextureBatchRenderPath : public MashCustomRenderPath
{
private:
	MashDevice *m_device;
	MashRenderInfo *m_renderInfo;

	MashMaterial *m_material;
	MashMesh *m_originalEntityMesh;
	MashMeshBuffer *m_batchedBuffer;
	f32 *m_instanceBufferData;
	uint32 m_currentInstancesInTheBuffer;

	//for texture fetch tehnique
	MashMatrix4 *m_boneTextureData;
	MashTexture *m_boneTexture;
public:
	TextureBatchRenderPath(MashDevice *device, MashMeshBuffer *batchedBuffer, MashMesh *originalEntityMesh, MashMaterial *material):m_device(device), m_batchedBuffer(batchedBuffer), 
		m_material(material), m_originalEntityMesh(originalEntityMesh), m_currentInstancesInTheBuffer(0), m_boneTextureData(0), m_instanceBufferData(0)
	{
		m_batchedBuffer->Grab();

		/*
			Each row in this texture represents one instances bone data.
			(width) = instance bone count (height) = instance count

			Each pixel is 1 float4. So 4 pixels equals 1 matrix.
		*/
		g_boneTextureDim.x = (f32)GetNextPowerOfTwo(g_boneCount * 4);
		g_boneTextureDim.y = (f32)GetNextPowerOfTwo(g_maxApplicationInstanceCount);

		//setup bone texture technique
		m_boneTexture = device->GetRenderer()->AddTexture("BoneTexture", 
			(uint32)g_boneTextureDim.x, 
			(uint32)g_boneTextureDim.y, 
			false, 
			aUSAGE_DYNAMIC, 
			aFORMAT_RGBA32_FLOAT);

		/*
			Assign the texture to the texture slot that aligns with the one used in the shader. 
			We could have instead created a custom effect auto for this texture...Either
			way is fine, both methods work.
		*/
		m_material->SetTexture(1, m_boneTexture);

		//setup custom bone texture dim effect auto
        BoneTextureDimEffectAuto *effAuto = MASH_NEW_COMMON BoneTextureDimEffectAuto();
		device->GetRenderer()->GetMaterialManager()->RegisterAutoParameterHandler(effAuto);
        effAuto->Drop();

		m_renderInfo = m_device->GetRenderer()->GetRenderInfo();
	}

	~TextureBatchRenderPath()
	{
		if (m_batchedBuffer)
		{
			m_batchedBuffer->Drop();
			m_batchedBuffer = 0;
		}
	}

	void AddObject(MashRenderable *pRenderable)
	{
		/*
			Copy instance bone data into the texture.
			Remember each row is a new instance.
		*/
		uint32 boneStart = (g_boneTextureDim.x / 4.0f) * m_currentInstancesInTheBuffer;
		//renderInfo stores all the insformation about the current scene. So we can get the bone array from there.
		memcpy(&m_boneTextureData[boneStart], m_renderInfo->GetBonePalette(), sizeof(MashMatrix4) * m_renderInfo->GetCurrentBoneCount());
		//set the instance id
		m_instanceBufferData[m_currentInstancesInTheBuffer] = m_currentInstancesInTheBuffer;
		++m_currentInstancesInTheBuffer;
	}

	void Flush()
	{
		//unlock the buffers so we can render
		UnlockBuffers();

		if (m_material->OnSet() == aMASH_OK)
		{
			m_device->GetRenderer()->DrawIndexedInstancedList(m_batchedBuffer, 
				m_originalEntityMesh->GetVertexCount(),
				m_originalEntityMesh->GetIndexCount(),
				m_originalEntityMesh->GetPrimitiveCount(),
				m_originalEntityMesh->GetPrimitiveType(),
				m_currentInstancesInTheBuffer);

			++g_drawCalls;
		}

		//lock the buffers for writing again
		LockBuffers();
	}

	void LockBuffers()
	{
		//lock the buffer for writing
		if (!m_instanceBufferData)
			m_batchedBuffer->GetVertexBuffer(1)->Lock(aLOCK_WRITE_DISCARD, (void**)&m_instanceBufferData);
		//lock the bone texture for writing
		if (!m_boneTextureData)
			m_boneTexture->Lock(aLOCK_WRITE_DISCARD, (void**)&m_boneTextureData);

		m_currentInstancesInTheBuffer = 0;
	}

	void UnlockBuffers()
	{
		//unlock the buffer so we can render
		if (m_instanceBufferData)
		{
			m_batchedBuffer->GetVertexBuffer(1)->Unlock();
			m_instanceBufferData = 0;
		}

		//unlock the bone texture
		if (m_boneTextureData)
		{
			m_boneTexture->Unlock();
			m_boneTextureData = 0;
		}
	}


	void OnDestroy(){}
};

class MainLoop : public MashGameLoop
{
private:
	MashDevice *m_device;

	MashSceneNode *m_root;
	MashAnimationMixer *m_animationMixer;
	uint32 m_playerControl;

	//this node hold the animated entity and the bone structure
	MashSceneNode *m_baseCharacterRoot;

	MashMaterial *m_standardSkinningMtrl;
	MashMaterial *m_boneArraySkinningMtrl;
	MashMaterial *m_textureFetchSkinningMtrl;

	TextureBatchRenderPath *m_textureFetchRenderPath;
	BoneArrayBatchRenderPath *m_boneArrayRenderPath;

	MashGUIStaticText *m_staticText;
	MashGUIStaticText *m_fpsText;
public:
	MainLoop(MashDevice *device):m_device(device), m_boneArrayRenderPath(0), m_textureFetchRenderPath(0){}
	virtual ~MainLoop()
    {
    }

	void UpdateText()
	{
		MashStringc text;

		switch(g_batchMode)
		{
		case BMODE_NO_BATCH:
			{
				text = "Mode : Standard (no batching)\n";
				break;
			}
		case BMODE_BONE_ARRAY:
			{
				char buffer[256];
                helpers::PrintToBuffer(buffer, sizeof(buffer), "Mode : HW instance + Bone array. %d instances per batch\n", g_maxBoneArrayInstanceCount);
				text = buffer;
				break;
			}
		case BMODE_TEXTURE:
			{
				text = "Mode : HW instance + Texture fetch\n";
				break;
			}
		}

		int8 buffer[256];
        helpers::PrintToBuffer(buffer, sizeof(buffer), "\Instance count : %d\n", g_currentInstanceCount);
		text += buffer;
        helpers::PrintToBuffer(buffer, sizeof(buffer), "\Bones per instance : %d\n", g_boneCount);
		text += buffer;

		text += "\nPress Z to decrease instance count\n\
Press X to increase instance count\n\
Press C to change rendering type";

		m_staticText->SetText(text);
	}

	void OnRenderModeChange()
	{
		//Unlock any buffers before moving on to the next render path.
		switch(g_batchMode)
		{
		case BMODE_BONE_ARRAY:
				m_boneArrayRenderPath->UnlockBuffers();
				break;
		case BMODE_TEXTURE:
				m_textureFetchRenderPath->UnlockBuffers();
				break;
		}

		static uint32 renderingTypeCounter = 0;
		g_batchMode = eBATCH_MODE(renderingTypeCounter++ % 3);

		/*
			Apply new materials to the entities and lock the buffers for rendering.

			In this case it is safe to change the materials at runtime because we know
			the vertex format at stream 0 (geometry stream) are the same for all the materials,
			and the mesh will not be forced to reformat its vertex buffer each time we call this.
		*/
		switch(g_batchMode)
		{
		case BMODE_NO_BATCH:
			{
				ApplyMaterialToAllNodes(m_root, m_standardSkinningMtrl);
				break;
			}
		case BMODE_BONE_ARRAY:
			{
				ApplyMaterialToAllNodes(m_root, m_boneArraySkinningMtrl);
				m_boneArrayRenderPath->LockBuffers();
				break;
			}
		case BMODE_TEXTURE:
			{
				ApplyMaterialToAllNodes(m_root, m_textureFetchSkinningMtrl);
				m_textureFetchRenderPath->LockBuffers();
				break;
			}
		}
	}

	bool Initialise()
	{
		srand(time(NULL));

		m_playerControl = m_device->GetInputManager()->CreatePlayer();
		const MashStringc inputContext = "play";
		m_device->GetInputManager()->AssignPlayerToController(m_playerControl, 0);//keyboard/mouse

		MashArray<eINPUT_EVENT> actions;

		actions.PushBack(aKEYEVENT_X);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, "pause", 50, actions, MashInputEventFunctor(&MainLoop::IncreaseInstanceCount, this));

		actions.Clear();
		actions.PushBack(aKEYEVENT_X);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION, actions, MashInputEventFunctor(&MainLoop::IncreaseInstanceCount, this));

		actions.Clear();
		actions.PushBack(aKEYEVENT_Z);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION+1, actions, MashInputEventFunctor(&MainLoop::DecreaseInstanceCount, this));

		actions.Clear();
		actions.PushBack(aKEYEVENT_C);
		m_device->GetInputManager()->SetPlayerActionMap(m_playerControl, inputContext, aINPUTMAP_USER_REGION+2, actions, MashInputEventFunctor(&MainLoop::ChangeRenderTechnique, this));

		m_device->GetRenderer()->GetMaterialManager()->LoadMaterialFile("InstancingDemoMaterials.mtl");
		m_boneArraySkinningMtrl = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("BoneArraySkinningMaterial");
		m_standardSkinningMtrl = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("StandardSkinningMaterial");
		m_textureFetchSkinningMtrl = m_device->GetRenderer()->GetMaterialManager()->FindMaterial("TextureFetchSkinningMaterial");

		MashList<MashSceneNode*> rootNodes;
		sLoadSceneSettings loadSettings;
		loadSettings.createRootNode = false;
		loadSettings.saveGeometryFlags = 0;
		m_device->GetSceneManager()->LoadSceneFile("AnimatedCharacter.nss", rootNodes, loadSettings);

		/*
			This is our character + bone structure that will be instanced.
		*/
		m_baseCharacterRoot = rootNodes.Front();

		m_root = m_device->GetSceneManager()->AddDummy(0, "SceneRoot");
		m_root->AddChild(m_baseCharacterRoot);

		MashCamera *camera = (MashCamera*)m_device->GetSceneManager()->AddCamera(m_root, "Camera");
		camera->SetZFar(1000);
		camera->SetZNear(1.0f);
		camera->SetPosition(MashVector3(10, 10, -50), true);
        
        MashFreeMovementController *freeCameraController = m_device->GetSceneManager()->CreateFreeMovementController(m_playerControl, inputContext, 0.05f, 50.0f);
        camera->AddCallback(freeCameraController);
        freeCameraController->Drop();

		MashLight *light = (MashLight*)m_device->GetSceneManager()->AddLight(m_root, "Light", aLIGHT_DIRECTIONAL, aLIGHT_RENDERER_FORWARD, true);
		light->SetDefaultDirectionalLightSettings(MashVector3(-1.0f,0.0f,0.0f));
		light->SetRange(1000);
		light->SetShadowsEnabled(false);

		m_animationMixer = m_baseCharacterRoot->GetAnimationMixer();
		if (m_animationMixer)
		{
			m_animationMixer->SetWrapMode("idle", aWRAP_LOOP);
			m_animationMixer->SetBlendMode("idle", aBLEND_BLEND);
			m_animationMixer->Transition("idle");
		}
		
		/*
			This function will apply the active material to the mesh.

			It's important the material is applied before we setup the new mesh
			buffer because the original meshes vertex buffer data is likley to
			change format if the materials vertex declaration at stream 0 is
			different to the original mesh.
		*/
		OnRenderModeChange();

		//both batching techniques have the save vertex decl, so they can use the same mesh buffer
		MashEntity *baseAnimatedCharacter = (MashEntity*)m_baseCharacterRoot->GetChildByName("Character");
		MashMesh *originalEntityMesh = baseAnimatedCharacter->GetModel()->GetMesh(0);
		sVertexStreamInit meshBufferInit[2];
		meshBufferInit[0].data = originalEntityMesh->GetRawVertices().Pointer();
		meshBufferInit[0].dataSizeInBytes = originalEntityMesh->GetRawVertices().GetCurrentSize();
		meshBufferInit[0].usage = aUSAGE_STATIC;
		meshBufferInit[1].data = 0;
		meshBufferInit[1].dataSizeInBytes = sizeof(uint32) * g_maxApplicationInstanceCount;
		meshBufferInit[1].usage = aUSAGE_DYNAMIC; 

		MashMeshBuffer *meshBufferForBatching = m_device->GetRenderer()->CreateMeshBuffer(meshBufferInit, 
			2, 
			m_boneArraySkinningMtrl->GetVertexDeclaration(), //any matching vertex decl will do
			originalEntityMesh->GetRawIndices().Pointer(), 
			originalEntityMesh->GetIndexCount(),
			originalEntityMesh->GetIndexFormat(), 
			aUSAGE_STATIC);
		
		//create the render paths
		m_boneArrayRenderPath = MASH_NEW_COMMON BoneArrayBatchRenderPath(m_device, meshBufferForBatching, originalEntityMesh, m_boneArraySkinningMtrl);
		m_boneArraySkinningMtrl->SetCustomRenderPath(m_boneArrayRenderPath);
		m_boneArrayRenderPath->Drop();

		m_textureFetchRenderPath = MASH_NEW_COMMON TextureBatchRenderPath(m_device, meshBufferForBatching, originalEntityMesh, m_textureFetchSkinningMtrl);
		m_textureFetchSkinningMtrl->SetCustomRenderPath(m_textureFetchRenderPath);
		m_textureFetchRenderPath->Drop();

		meshBufferForBatching->Drop();

		//set up gui
		MashGUIRect fpsRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 0.0f),
			MashGUIUnit(0.0f, 170.0f), MashGUIUnit(0.0f, 40.0f));
		m_fpsText = m_device->GetGUIManager()->AddStaticText(fpsRegion, 0);

		MashGUIRect textRegion(MashGUIUnit(0.0f, 0.0f), MashGUIUnit(0.0f, 40.0f),
			MashGUIUnit(0.0f, 400.0f), MashGUIUnit(0.0f, 170.0f));
		m_staticText = m_device->GetGUIManager()->AddStaticText(textRegion, 0);
		
		UpdateText();

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

		if (g_batchMode == BMODE_NO_BATCH)
			//we can use SceneManager::GetCurrentSceneRenderInfo() to get render counts from this frames cull
			g_drawCalls = m_device->GetSceneManager()->GetCurrentSceneRenderInfo()->forwardRenderedSolidObjectCount;

		MashStringc perFrameText;
		int8 buffer[256];
        helpers::PrintToBuffer(buffer, sizeof(buffer), "FPS : %d\n", m_device->GetFPS());
		perFrameText = buffer;
        helpers::PrintToBuffer(buffer, sizeof(buffer), "Scene draw calls : %d", g_drawCalls);
		perFrameText += buffer;
		m_fpsText->SetText(perFrameText);

		m_device->GetGUIManager()->BeginDraw();
		m_device->GetGUIManager()->DrawAll();
		m_device->GetGUIManager()->EndDraw();

		g_drawCalls = 0;
	}

	bool Pause()
	{
		m_device->GetSceneManager()->CullScene(m_root);
		m_device->GetSceneManager()->DrawScene();

		MashStringc perFrameText;
		int8 buffer[256];
        helpers::PrintToBuffer(buffer, sizeof(buffer), "FPS : %d\n", m_device->GetFPS());
		perFrameText = buffer;
        helpers::PrintToBuffer(buffer, sizeof(buffer), "Scene draw calls : %d", g_drawCalls);
		perFrameText += buffer;
		m_fpsText->SetText(perFrameText);

		m_device->GetGUIManager()->BeginDraw();
		m_device->GetGUIManager()->DrawAll();
		m_device->GetGUIManager()->EndDraw();

		g_drawCalls = 0;
		return false;
	}

	void IncreaseInstanceCount(const sInputEvent &e)
	{
		if (e.isPressed == 0)
		{
			for(uint32 i = 0; i < g_instanceIncrementCount; ++i)
			{
				if (g_currentInstanceCount < g_maxApplicationInstanceCount)
				{
					++g_currentInstanceCount;

                    /*
                        Create a new instance of all the nodes associated with the entity.
                        Root node, entity, and bones. The entity will also have a new skin
                        created for it filled with the newly instanced bones.
                     
                        The animation mixer still needs to be created though...
                    */
                    MashSceneNode *newInstanceRoot = m_device->GetSceneManager()->AddInstance(m_baseCharacterRoot, m_root, "Instance", true);
                    
					//create animation mixer for the new instance
					MashAnimationMixer *newInstanceMixer = m_device->GetSceneManager()->GetControllerManager()->CreateMixer(newInstanceRoot);

					//set some animation settings
					newInstanceMixer->SetWrapMode("idle", aWRAP_LOOP);
					newInstanceMixer->Transition("idle", 0.0f);

					/*
						Give the new instance a random position. Important to note here that we are moving the root
						node, not the entity directly! Depending on your skinning code and scene layout moving a skinned
						entity directly may not be desirable because the bones will not follow the entity through
						a parent/child relationship.
					*/
					int32 areaHalfWidth = 100;
					int32 areaHalfLength = 100;
					newInstanceRoot->SetPosition(MashVector3(mash::math::RandomInt(-areaHalfWidth, areaHalfWidth), 0, math::RandomInt(-areaHalfLength, areaHalfLength)), true);
					newInstanceRoot->SetScale(MashVector3(0.5f, 0.5f, 0.5f), true);
				}
			}

			UpdateText();
		}

		
	}

	void DecreaseInstanceCount(const sInputEvent &e)
	{
		if (e.isPressed == 0)
		{
			for(uint32 i = 0; i < g_instanceIncrementCount; ++i)
			{
				if (g_currentInstanceCount > 1)
				{
					//remove the first instance found
					m_device->GetSceneManager()->RemoveSceneNode(m_root->GetChildByName("Instance"));
					--g_currentInstanceCount;
				}
			}
			
			UpdateText();
		}
	}

	void ChangeRenderTechnique(const sInputEvent &e)
	{
		if (e.isPressed == 0)
		{
			OnRenderModeChange();
			UpdateText();
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
	deviceSettings.fixedTimeStep = 1.0/30.0;

    deviceSettings.rootPaths.PushBack("../../../../../Media/GUI");
    deviceSettings.rootPaths.PushBack("../../../../../Media/Materials");
    deviceSettings.rootPaths.PushBack("../../../../DemoMedia/DynamicInstancingDemo");

	MashDevice *device = CreateDevice(deviceSettings);

	if (!device)
		return 1;

	device->SetWindowCaption("Dynamic Instancing Demo");

	MainLoop *mainLoop = MASH_NEW_COMMON MainLoop(device);
	device->SetGameLoop(mainLoop);
	mainLoop->Drop();

	device->Drop();
    
    return 0;
}
