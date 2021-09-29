//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef _C_MASH_SCENE_MANAGER_H_
#define _C_MASH_SCENE_MANAGER_H_

#include "MashSceneManager.h"
#include "MashSceneNode.h"
#include "CMashMeshBuilder.h"
#include "MashInputManager.h"
#include "MashList.h"
#include <map>
#include "MashString.h"
#include <algorithm>
#include "MashMaterial.h"
#include "MashMeshBuilder.h"
#include "MashTechniqueInstance.h"
#include "MashRenderable.h"
#include "MashCamera.h"
#include "MashGeometryBatch.h"

namespace mash
{
	class MashVideo;
	
	class MashShadowCaster;
	class MashRenderSurface;
	class CMashCamera;
	class CMashEntity;
	class CMashLight;
	class MashDummy;
	
	class CMashModelLoader;
	class MashDecal;

	const uint32 g_gbufferLightingType = 3;
	
	class CMashModel;
	class MashModel;
	class MashGeometryBatch;

	class CMashSceneManager : public MashSceneManager
	{
	private:

		struct sSolidRenderable
		{
			mash::MashRenderable *pRenderable;
            uint32 renderKey;

			bool operator<(const sSolidRenderable &other)const
			{
                return renderKey < other.renderKey;
			}

            //TODO : Perhaps also store the material so it doesnt hae to be accesed when sorted.
			sSolidRenderable(mash::MashRenderable *_pRenderable):pRenderable(_pRenderable),
                renderKey(pRenderable->GetMaterial()->GetActiveTechnique()->GetRenderKey())
			{
			}

			sSolidRenderable():pRenderable(0), renderKey(0){}
		};

		struct sTransparentRenderable
		{
			mash::MashRenderable *pRenderable;

			/*
				The render key for a transparent object is made up from the render
				key from the current technique and the objects depth.
				The key is made up from,
				First 32bits = current depth in view space
				Second 32bits = technique render key

				The current depth is placed at the beginning of the key
				so that it is given priority when sorting transparent obejcts.
			*/
			uint64 m_iRenderKey;

			bool operator<(const sTransparentRenderable &other)const
			{
				return m_iRenderKey < other.m_iRenderKey;
			}

			sTransparentRenderable(mash::MashRenderable *_pRenderable,
				const mash::MashCamera *pCamera):pRenderable(_pRenderable)
			{
				uint32 iViewDistance = (uint32)pCamera->GetDistanceToBox(_pRenderable->GetWorldBoundingBox());
				uint64 iMaterialKey = _pRenderable->GetMaterial()->GetActiveTechnique()->GetRenderKey();
				m_iRenderKey = (iMaterialKey << 32UL)|(iViewDistance);
			}

			sTransparentRenderable():pRenderable(0){}
		};

		struct sShadowData
		{
			eSHADOW_MAP_FORMAT textureFormat;
			uint32 drawDistance;
			f32 bias;
			uint32 textureSize;
		};

		struct sShadowCaster
		{
			MashShadowCaster *caster;
			bool isLoaded;
		};

		enum eREBUILD_SHADER_STATUS
		{
			aREBUILD_DEFERRED_DIR_SHADER = 1,
			aREBUILD_DEFERRED_SPOT_SHADER = 2,
			aREBUILD_DEFERRED_POINT_SHADER = 4,
			aREBUILD_FORWARD_RENDERED_SCENE = 8,
			aREBUILD_AUTO_SHADERS = 16,
			aREBUILD_SCENE_WITH_FOG = 32,
			aREBUILD_DIRECTIONAL_SHADOW_CASTERS = 64,
			aREBUILD_DIRECTIONAL_SPOT_CASTERS = 128,
			aREBUILD_DIRECTIONAL_POINT_CASTERS = 256,

			aREBUILD_ALL = 0xFFFFFFFF
		};
	private:
        
        struct sInstancedSkin
        {
            MashSkin *newSkin;
            MashSkin *oldSkin;
        };
        
        MashArray<sInstancedSkin> m_instancedSkins;
        bool m_createSkinInstances;

		MashRenderSurface *m_pGBufferRT;
		MashRenderSurface *m_pGBufferLightRT;
		MashRenderSurface *m_customViewportRT;

		MashMaterial *m_pGBufferLighting[g_gbufferLightingType];
		MashMaterial *m_pFinalMaterial;
		MashMaterial *m_pGBufferClearMaterial;
		////used for the line renderer
		MashGeometryBatch *m_pPrimitiveBatch;
        
		mash::eRENDER_STAGE m_eRenderPass;

		MashList<sSolidRenderable> m_solidRenderables;
		MashList<sTransparentRenderable> m_transparentRenderables;

		MashList<sSolidRenderable> m_solidDecals;
		MashList<sTransparentRenderable> m_transparentDecals;

		MashList<sSolidRenderable> m_solidParticles;
		MashList<sTransparentRenderable> m_transparentParticles;

		//no transparent pass for deferred renderer
		MashList<sSolidRenderable> m_deferredRenderables;
		MashList<sSolidRenderable> m_deferredDecals;
		MashList<sSolidRenderable> m_deferredParticles;

		MashArray<MashLight*> m_currentRenderSceneLightList;
		MashList<sSolidRenderable> m_shadowRenderables;
		mash::MashAABB m_shadowSceneBounds;

		MashArray<mash::MashSceneNode*> m_lookatTrackers;
        MashArray<MashSceneNode*> m_callbackNodes;

		MashArray<MashCustomRenderPath*> m_batchFlushList;

		bool m_isSceneInitializing;
		MashArray<mash::MashMesh*> m_initialiseMeshLoadData;

		bool m_castTransparentObjectShadows;
		sShadowData m_shadowMapDefaultSettings[aLIGHT_TYPE_COUNT];
		f32 m_fDecalZBias;

		sSceneRenderInfo m_sceneRenderInfo;

		mash::MashVideo *m_pRenderer;
		mash::MashInputManager *m_pInputManager;

		bool m_lastPrimitiveDepthTest;

		/*
			The scene manager doesnt increment the reference counter when adding
			to this list. A node will call the managers _remove method when
			it has been deleted.
			Nodes are only stored for access via scripts, otherwise this list would be removed.
		*/
		MashList<MashSceneNode*> m_nodeList;
		mash::MashCamera *m_pActiveCamera;

		MashArray<MashLight*> m_forwardRenderedLightList;
		sMashLight *m_forwardLightBuffer;
		uint32 m_reservedForwardLightBufferElements;
		eLIGHTING_TYPE m_preferredLightingMode;//TODO : Add access methods

		sShadowCaster m_shadowCasters[aLIGHT_TYPE_COUNT];

		MashCullTechnique *m_activeSceneCullTechnique;
		MashCullTechnique *m_activeShadowCullTechnique;

		/*
			These are used for unique name generation
		*/
		uint32 m_iSceneNodeNameCounter;

		bool m_bIsFogEnabled;
		//flag for regenerating shader code for lighting etc...
		uint32 m_rebuildShaderState;

		bool m_deferredRendererLoadAttempted;
		bool m_deferredRendererValid;

		/*
			These are used to determine when specific shaders
			need to be updated during runtime
		*/
		int32 m_shadowEnabledDirLightCount;
		int32 m_shadowEnabledSpotLightCount;
		int32 m_shadowEnabledPointLightCount;

		MashMeshBuilder *m_pMeshBuilder;

		MashControllerManager *m_pControllerManager;

		/*
			This points to the scene node that is currently
			being updated. This is used in scripts.
		*/
		MashSceneNode *m_pCurrentSceneNode;
        
        uint32 (*m_pRenderKeyHashFunction)(const MashTechniqueInstance *pTechnique);

		eMASH_STATUS RenderShadowMap(mash::MashLight *pLight, MashShadowCaster *pShadowCaster);

		virtual eMASH_STATUS DrawDeferredScene();
		eMASH_STATUS DrawForwardRenderedScene();

		void DefaultSceneCull(MashSceneNode *root);
		void _FlushRenderableBatches();
		eMASH_STATUS CreateGBuffer();
	public:
		CMashSceneManager();
		virtual ~CMashSceneManager();

		eMASH_STATUS _Initialise(mash::MashVideo *pRenderer, mash::MashInputManager *pInputManager, const mash::sMashDeviceSettings &settings);

		void _OnEndUserInitialise();
		void _OnBeginUserInitialise();

        void _AddCallbackNode(mash::MashSceneNode *node);
        void _RemoveCallbackNode(mash::MashSceneNode *node);

		void _AddLookAtTracker(mash::MashSceneNode *node);
		void _RemoveLookAtTracker(mash::MashSceneNode *node);

		void _LateUpdate();

		void GenerateUniqueSceneNodeName(MashStringc &out);
        
        void SetRenderKeyHashFunction(uint32 (*p)(const MashTechniqueInstance *pTechnique));
        uint32 GenerateRenderKeyForTechnique(const MashTechniqueInstance *pTechnique);

	MashDecal* AddDecal(MashSceneNode *parent,
		const MashStringc &sName,
		eDECAL_TYPE decalType,
		int32 decalLimit = 20,
		eLIGHTING_TYPE lightingType = aLIGHT_TYPE_NONE,
		bool createMaterialInstance = true);

	MashDecal* AddDecalCustom(MashSceneNode *parent,
		const MashStringc &sName,
		MashMaterial *pMaterial,
		int32 decalLimit = 20,
		MashSkin *skin = 0);

	MashSkin* CreateSkin();
        
        MashSkin* _CreateSkinInstance(MashSkin *instanceFrom);
        MashSceneNode* AddInstance(MashSceneNode *instanceFrom, MashSceneNode *parent, const MashStringc &instanceName, bool createSkinInstances = true);

	MashBone* AddBone(MashSceneNode *parent,
		const MashStringc &name);

	mash::MashTexture* GetDeferredDiffuseMap()const;
	mash::MashTexture* GetDeferredNormalMap()const;
	mash::MashTexture* GetDeferredSpecularMap()const;
	mash::MashTexture* GetDeferredDepthMap()const;

	mash::MashTexture* GetDeferredLightingMap()const;
	mash::MashTexture* GetDeferredLightingSpecularMap()const;
        
        bool IsDeferredRendererInitialised()const;

		/*
			return MashMesh : This pointer MUST be dropped when you are finished with it.
		*/
		virtual MashStaticMesh* CreateStaticMesh();
		virtual MashDynamicMesh* CreateDynamicMesh();
        
		MashEllipsoidColliderController* CreateEllipsoidColliderController(MashSceneNode *character, 
			MashSceneNode *collisionScene,
			const MashVector3 &radius,
			const MashVector3 &gravity);

		MashCharacterMovementController* CreateCharacterMovementController(uint32 playerId, const MashStringc &inputContext, f32 rotationSpeed = 0.1f, f32 linearSpeed = 10.0f, const MashCharacterMovementController::sInputActionMovement *customActions = 0);
        MashFreeMovementController* CreateFreeMovementController(uint32 playerId, const MashStringc &inputContext, f32 rotationSpeed = 0.1f, f32 linearSpeed = 10.0f, const MashFreeMovementController::sInputActionMovement *customActions = 0);

		/*
			Call MashSceneNode::Remove() to delete a scene node. (unless the user has made
			additional calls to MashSceneNode::Grab()) 
		*/
		virtual MashCamera* AddCamera(MashSceneNode *parent, const MashStringc &sName);
		virtual MashLight* AddLight(MashSceneNode *parent, const MashStringc &sName, mash::eLIGHTTYPE lightType, eLIGHT_RENDERER_TYPE lightRendererType, bool mainLight);
		virtual MashEntity* AddEntity(MashSceneNode *parent, MashModel *pModel, const MashStringc &sName);
		virtual MashEntity* AddEntity(MashSceneNode *parent, const MashStringc &sUserName);
		virtual MashParticleSystem* AddParticleSystem(MashSceneNode *parent, 
			const MashStringc &userName, 
			const sParticleSettings &settings,
			ePARTICLE_TYPE particleType = aPARTICLE_CPU, 
			eLIGHTING_TYPE lightingType = aLIGHT_TYPE_NONE,
			bool createMaterialInstance = true,
			MashModel *model = 0);

		virtual MashParticleSystem* AddParticleSystemCustom(MashSceneNode *parent, 
			const MashStringc &userName, 
			const sParticleSettings &settings,
			MashMaterial *material);

		virtual MashCullTechnique* CreateCullTechnique(eCULL_TECHNIQUE tech);
		virtual void SetCullTechnique(MashCullTechnique *tech);
		virtual void SetCullTechniqueShadow(MashCullTechnique *tech);

		//Helper method
		MashEntity* AddEntity(MashSceneNode *parent, MashArray<MashArray<MashMesh*> > &meshLodList, const MashStringc &sNodeName);
		virtual MashDummy* AddDummy(MashSceneNode *parent, const MashStringc &sName);

		virtual MashModel* CreateModel();
		virtual MashModel* CreateModel(MashArray<MashArray<MashMesh*> > &meshLodList);

		MashTriangleCollider* CreateTriangleCollider(MashTriangleBuffer **buffer, uint32 bufferCount, eTRIANGLE_COLLIDER_TYPE type = aTRIANGLE_COLLIDER_STANDARD, bool waitForDeserialize = false);
		MashTriangleCollider* CreateTriangleCollider(MashModel *model, uint32 lod = 0, eTRIANGLE_COLLIDER_TYPE type = aTRIANGLE_COLLIDER_STANDARD, bool generateTriangleBufferIfNull = true);

		virtual MashTriangleBuffer* CreateTriangleBuffer(MashMesh *pMesh);
		virtual MashTriangleBuffer* CreateTriangleBuffer();

		/*
			Drops all scene nodes.
		*/
		virtual void RemoveAllSceneNodes();
		void RemoveSceneNode(MashSceneNode *pNode);

		eMASH_STATUS LoadSceneFile(const MashArray<MashStringc> &filenames, MashList<mash::MashSceneNode*> &rootNodes, const sLoadSceneSettings &loadSettings);
		eMASH_STATUS LoadSceneFile(const MashStringc &filename, MashList<mash::MashSceneNode*> &rootNodes, const sLoadSceneSettings &loadSettings);
		eMASH_STATUS SaveSceneFile(const MashStringc &filename, const MashList<mash::MashSceneNode*> &rootNodes, const sSaveSceneSettings &saveData);

		eMASH_STATUS SaveShadowCastersToFile(const MashStringc &filename);
		eMASH_STATUS LoadShadowCastersFromFile(const MashStringc &filename, MashLoadCasterFunctor customFunctor);

		virtual void _Update(f32 dt);
		void CompileAllMaterials(uint32 compileFlags = 0);
		void AddRenderableToRenderQueue(mash::MashRenderable *pRenderable, eHLRENDER_PASS pass, eRENDER_STAGE stage);
		eMASH_STATUS CullScene(MashSceneNode *scene);
		eMASH_STATUS DrawScene();
		//TODO : Params for thickness maybe?
		eMASH_STATUS DrawLine(const MashVector3 &start, const MashVector3 &end, const mash::sMashColour &colour, bool depthTest = true);
		eMASH_STATUS DrawLines(const mash::MashVertexColour::sMashVertexColour *pLines, uint32 iVertexCount, bool depthTest = true);	
		eMASH_STATUS DrawAABB(const mash::MashAABB &box, const sMashColour &colour, bool depthTest = true);

		eMASH_STATUS CreateTriangleBufferAdjacencyRenderBuffer(const MashTriangleBuffer *buffer, const MashMatrix4 &worldTransform, MashArray<MashVertexColour::sMashVertexColour> &out);
		eMASH_STATUS CreateTriangleBufferAdjacencyRenderBuffer(const MashModel *model, const MashMatrix4 &worldTransform, MashArray<MashVertexColour::sMashVertexColour> &out);

		//is called at the end of rendering
		void FlushGeometryBuffers();

		void _AddForwardRenderedLight(MashLight *light, bool setAsMain);
		void _RemoveForwardRenderedLight(MashLight *light);

		MashLight* GetFirstForwardRenderedLight()const;
		const MashArray<MashLight*>& GetForwardRenderedLightList()const;

		uint32 GetForwardRenderedLightCount()const;
		bool IsFogEnabled()const;
		void SetFogEnabled(bool bValue);

		void RemoveAllSceneObjects();

		void EnableTransparentObjectShadowCasting(bool value);
		void SetShadowCaster(mash::eLIGHTTYPE type, MashShadowCaster *caster);
		MashDirectionalShadowCascadeCaster* CreateDirectionalCascadeShadowCaster(MashDirectionalShadowCascadeCaster::eCASTER_TYPE casterType);
		MashSpotShadowCaster* CreateSpotShadowCaster(MashSpotShadowCaster::eCASTER_TYPE casterType);
		MashPointShadowCaster* CreatePointShadowCaster(MashPointShadowCaster::eCASTER_TYPE casterType);

		eSHADOW_MAP_FORMAT GetShadowMapTextureFormat(eLIGHTTYPE lightType)const;
		bool IsTransparentObjectShadowCastingEnabled()const;
		void _CreateShadowCaster(mash::eLIGHTTYPE type);
		MashShadowCaster* GetShadowCaster(mash::eLIGHTTYPE type)const;

		
		

		eLIGHTING_TYPE GetPreferredLightingMode()const;
		void SetPreferredLightingMode(eLIGHTING_TYPE type);

		virtual eMASH_STATUS SetActiveCamera(MashCamera *pCamera);
		virtual MashCamera* GetActiveCamera();
		virtual eMASH_STATUS SetActiveCameraByName(const MashStringc &name);

		virtual MashSceneNode* GetSceneNodeByName(const MashStringc &sName)const;
		virtual MashSceneNode* GetSceneNodeByID(uint32 id)const;
        MashSceneNode* GetSceneNodeByUserID(int32 userId)const;
		virtual uint32 GetSceneNodeCount()const;
		const MashList<MashSceneNode*>& GetSceneNodeList()const;

		virtual eMASH_STATUS UpdateScene(f32 dt, MashSceneNode *pScene);
		eRENDER_STAGE GetActivePass()const;

		void SetDecalZBias(f32 fBias = 1.0f);
		f32 GetDecalZBias()const;

		void SetRunTimeMaterialBuilderCallback(eMASH_STATUS (*p)());

		virtual MashMeshBuilder* GetMeshBuilder()const;
		MashControllerManager* GetControllerManager()const;

		bool GetForwardRenderedShadowsEnabled()const;
		bool GetDeferredDirShadowsEnabled()const;
		bool GetDeferredSpotShadowsEnabled()const;
		bool GetDeferredPointShadowsEnabled()const;

		const sSceneRenderInfo* GetCurrentSceneRenderInfo();

		/*
			For script access.
		*/
		MashSceneNode* GetCurrentScriptSceneNode()const;
		virtual void _SetCurrentScriptSceneNode(MashSceneNode *pNode);
        
        void _AddLightToCurrentRenderScene(MashLight *light);
		void _OnLightTypeChange(MashLight *light);

		void _AddCustomRenderPathToFlushList(MashCustomRenderPath *batch);

		void _OnDeferredLightingModeEnabled();
		void _OnShadowsEnabled(MashLight *light);
		void _OnShadowsDisabled(MashLight *light);
		void OnShadowReceiverMaterialRebuildNeeded(MashShadowCaster *caster);
		void OnShadowCasterMaterialRebuildNeeded(MashShadowCaster *caster);

		void _OnViewportChange();
		void _OnPostResolutionChange();
	};
    
    inline bool CMashSceneManager::IsDeferredRendererInitialised()const
    {
        return m_deferredRendererValid;
    }
    
    inline void CMashSceneManager::SetRenderKeyHashFunction(uint32 (*p)(const MashTechniqueInstance *pTechnique))
	{
		m_pRenderKeyHashFunction = p;
	}

	inline const MashSceneManager::sSceneRenderInfo* CMashSceneManager::GetCurrentSceneRenderInfo()
	{
		return &m_sceneRenderInfo;
	}

	inline eLIGHTING_TYPE CMashSceneManager::GetPreferredLightingMode()const
	{
		return m_preferredLightingMode;
	}

	inline const MashArray<MashLight*>& CMashSceneManager::GetForwardRenderedLightList()const
	{
		return m_forwardRenderedLightList;
	}

	inline const MashList<MashSceneNode*>& CMashSceneManager::GetSceneNodeList()const
	{
		return m_nodeList;
	}

	inline void CMashSceneManager::EnableTransparentObjectShadowCasting(bool value)
	{
		m_castTransparentObjectShadows = value;
	}

	inline bool CMashSceneManager::IsTransparentObjectShadowCastingEnabled()const
	{
		return m_castTransparentObjectShadows;
	}

	inline bool CMashSceneManager::IsFogEnabled()const
	{
		return m_bIsFogEnabled;
	}

	inline void CMashSceneManager::SetDecalZBias(f32 fBias)
	{
		m_fDecalZBias = fBias;
	}

	inline f32 CMashSceneManager::GetDecalZBias()const
	{
		return m_fDecalZBias;
	}

	inline MashCamera* CMashSceneManager::GetActiveCamera()
	{
		return m_pActiveCamera;
	}

	inline eRENDER_STAGE CMashSceneManager::GetActivePass()const
	{
		return m_eRenderPass;
	}

	inline MashControllerManager* CMashSceneManager::GetControllerManager()const
	{
		return m_pControllerManager;
	}

	inline MashMeshBuilder* CMashSceneManager::GetMeshBuilder()const
	{
		return m_pMeshBuilder;
	}

	inline MashSceneNode* CMashSceneManager::GetCurrentScriptSceneNode()const
	{
		return m_pCurrentSceneNode;
	}
}

#endif