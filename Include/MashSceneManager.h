//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_SCENE_MANAGER_H_
#define _MASH_SCENE_MANAGER_H_

#include "MashReferenceCounter.h"
#include "MashCreationParameters.h"
#include "MashEnum.h"
#include "MashList.h"
#include "MashArray.h"
#include "MashFunctor.h"
#include "MashDirectionalShadowCascadeCaster.h"
#include "MashSpotShadowCaster.h"
#include "MashPointShadowCaster.h"
#include "MashFreeMovementController.h"
#include "MashCharacterMovementController.h"

namespace mash
{
	class MashQuaternion;
	class MashVector3;
	class MashControllerManager;
	class MashShadowCaster;
	class MashCullTechnique;
	class MashDirectionalShadowCascadeCaster;
	class MashSpotShadowCaster;
	class MashPointShadowCaster;
	class MashEllipsoidColliderController;

	class MashCamera;
	class MashEntity;
	class MashLight;
	class MashMeshBuilder;
	class MashDummy;
	class MashBone;
	class MashParticleSystem;
	
	class MashStaticMesh;
	class MashDynamicMesh;
	class MashDecal;
	class MashMaterial;

	class MashInputManager;
	class MashCustomRenderPath;
	class MashTriangleBuffer;
	class MashTriangleCollider;
	class MashSkin;

	class MashRenderable;
	class MashVideo;
	class MashTechniqueInstance;

	class MashMesh;
	class MashSceneNode;
	class MashModel;

	struct sMashCasterLoader
	{
		int32 casterType;
		MashShadowCaster *returnVal;
	};
	/*!
		Specialised functor class for shadow caster loading
	*/
	typedef MashFunctor<sMashCasterLoader> MashLoadCasterFunctor;

    /*!
        This manages the creation and updating of all scene objects and functions.
     
        The scene manager should be updated using UpdateScene() from your MashGameLoop::Update().
        In MashGameLoop::Render() you should call CullScene() for a particular scene graph. This
        function will cull the graph using the active cull techniques and add anything that
        passes to an internal list for rendering. DrawScene() can be called to draw that list.
        This sequence can be done multiple times per frame if needed. Scene nodes may also leave
        updating render only data till it passes CullScene().
     
        Different scene culling techniques can be created using CreateCullTechnique() and
        set using SetCullTechnique(). MashCullTechnique Allows you to derive from this class
        to create your own custom culling techniques. Scene culling and shadow culling use
        seperate techniques so they can be optimized for each situation. The shadow technique
        can be set using SetCullTechniqueShadow().
     
        Shadow casters are also set from here using SetShadowCaster(). Each light has its
        own shadow caster, default casters can be created from here but you are also free
        to create your own from MashShadowCaster. 
     
        The scene manager contains CompileAllMaterials() that will batch compile all materials based
        on flags. This function will compile materials faster than compiling many materials individually.
        This is automatically called at the end of MashGameLoop::Initialise() but can be called
        manually if materials are created after that. 
     
        Any scene node creation methods here that start with Addxxx() mean they are added to the
        manager and should not be dropped. Instead call RemoveSceneNode() or 
        RemoveAllSceneNode(). Methods starting with Createxxx() must be dropped
        by the user.
    */
	class MashSceneManager : public MashReferenceCounter
	{
	public:
        
        /*!
            Holds built in culling techniques.
        */
		enum eCULL_TECHNIQUE
		{
            /*!
                Culling technique that uses the camera frustum to determine what is
                and isn't visible.
            */
			aCULL_TECH_CAMERA,
            
            /*!
                Culling technique used in the shadow pass that simply checks to see if an
                object is a shadow caster or not.
            */
			aCULL_TECH_SHADOW
		};
        
        //! Defines the max forward rendered light count.
		enum eFORWARD_RENDERED_VALUES
		{
			aMAX_FORWARD_RENDERED_LIGHT_COUNT = 16
		};

        //! Holds useful information for scene debugging.
		struct sSceneRenderInfo
		{
            //! Shadow caster count.
			uint32 shadowObjectCount;
            
            //! Solid objects rendered in the forward renderer.
			uint32 forwardRenderedSolidObjectCount;
            
            //! Transparent objects rendered in the forward renderer.
			uint32 forwardRenderedTransparentObjectCount;
            
            //! Solid objects rendered in the deferred renderer.
			uint32 deferredObjectSolidCount;
		};

		
	public:
		MashSceneManager(){}
		virtual ~MashSceneManager(){}

        //! Called internally to initialise the scene manager.
        /*!
            \param renderer Video renderer.
            \param inputManager Input manager.
            \param settings Initialise settings.
        */
		virtual eMASH_STATUS _Initialise(MashVideo *renderer, MashInputManager *inputManager, const sMashDeviceSettings &settings) = 0;

        //! Loads a scene file.
        /*!
            Valid file extentions are .nss or .dae.
         
            If this is called after MashGameLoop::Initialise() then you may want to call
            MashSceneManager::CompileAllMaterials(aMATERIAL_COMPILER_NON_COMPILED) to compile any materials
            that may have loaded.
         
            \param filename File to load.
            \param rootNodes Loaded root nodes will be added here.
            \param loadSettings Load settings.
            \return ok on succes, failed otherwise.
        */
		virtual eMASH_STATUS LoadSceneFile(const MashStringc &filename, MashList<MashSceneNode*> &rootNodes, const sLoadSceneSettings &loadSettings) = 0;

		//! Loads many scene file.
        /*!
			Use this function to load many scene files instead of loading them one by one to improve
			memory usage and fragmentation.

            Valid file extentions are .nss or .dae.
         
            If this is called after MashGameLoop::Initialise() then you may want to call
            MashSceneManager::CompileAllMaterials(aMATERIAL_COMPILER_NON_COMPILED) to compile any materials
            that may have loaded.
         
            \param filenames Files to load.
            \param rootNodes Loaded root nodes will be added here.
            \param loadSettings Load settings.
            \return ok on succes, failed otherwise.
        */
		virtual eMASH_STATUS LoadSceneFile(const MashArray<MashStringc> &filenames, MashList<MashSceneNode*> &rootNodes, const sLoadSceneSettings &loadSettings) = 0;
        
        //! Saves a scene to a scene file.
        /*!
            \param filename Path and file to save.
            \param rootNodes Root nodes to save.
            \param saveData Save settings.
            \return ok on succes, failed otherwise.
        */
		virtual eMASH_STATUS SaveSceneFile(const MashStringc &filename, const MashList<MashSceneNode*> &rootNodes, const sSaveSceneSettings &saveData) = 0;

        //! Generates a unique node name.
        /*!
            \param out Unique name will be save here.
        */
		virtual void GenerateUniqueSceneNodeName(MashStringc &out) = 0;

        //! Gets the deferred scene diffuse map. 
		virtual MashTexture* GetDeferredDiffuseMap()const = 0;
        
        //! Gets the deferred scene normal map.
		virtual MashTexture* GetDeferredNormalMap()const = 0;
        
        //! Gets the deferred scene specular map.
		virtual MashTexture* GetDeferredSpecularMap()const = 0;
        
        //! Gets the deferred scene depth map.
		virtual MashTexture* GetDeferredDepthMap()const = 0;

        //! Gets the final calculated light scene from the deferred renderer.
		virtual MashTexture* GetDeferredLightingMap()const = 0;
        
        //! Gets the final calculated specular light scene from the deferred renderer.
		virtual MashTexture* GetDeferredLightingSpecularMap()const = 0;

		//! Called to flush any geometry buffers.
        /*!
            This is called internally at the end of MashGameLoop::Render(). You may want to call
            this manually before swaping out render targets.
        */
		virtual void FlushGeometryBuffers() = 0;
        
		//! Creates an empty static mesh.
        /*!
            The returned pointer must be dropped when your done with it.
         
            Static meshes are optimized for geometry being set only once on load
            and not changing from the CPU.
            Most of your scene will probably be made up of static meshes.
         
            \return New static mesh.
        */
		virtual MashStaticMesh* CreateStaticMesh() = 0;
        
        //! Creates an empty dynamic meshes.
        /*!
            The returned pointer must be dropped when your done with it.
         
            Dynamic meshes are designed to be manipulated regularly on the CPU.
            If geometry will only be set once then consider using static meshes
            as they have better rendering performance.

            \return New dynamic mesh.
        */
		virtual MashDynamicMesh* CreateDynamicMesh() = 0;

		//! Creates an ellipsoid collider for scene nodes.
		/*!
			This controller should only be added to one node, after that node has been initialised. 
         
            The collision scene is not grabbed by this controller so care should be taken to
            when dropping nodes.
		*/
		virtual MashEllipsoidColliderController* CreateEllipsoidColliderController(MashSceneNode *character, 
			MashSceneNode *collisionScene,
			const MashVector3 &radius,
			const MashVector3 &gravity) = 0;

		//! Creates a character movement node controller
        /*!
            The returned pointer must be dropped when your done with it.
         
            Character movement controllers can be added to nodes so they move around a scene on the X and Z axis, and
			rotate on the Y axis. The controller can be controlled by keyboard and mouse.

			This type of controller is typically used for FPS or 3rd person characters.
         
			\param playerId Id from InputManager::CreatePlayer().
            \param inputContext The input context the input should use. See MashInputManager for more info.
            \param rotationSpeed Input rotation speed.
            \param linearSpeed Linear movement speed.
			\param customActions Custom key actions. Leave as NULL to use defaults from MashInputManager::CreateDefaultActionMap().
            \return New controller.
        */
		virtual MashCharacterMovementController* CreateCharacterMovementController(uint32 playerId, 
			const MashStringc &inputContext, 
			f32 rotationSpeed = 0.1f, 
			f32 linearSpeed = 10.0f, 
			const MashCharacterMovementController::sInputActionMovement *customActions = 0) = 0;
        
        //! Creates a free movement node controller
        /*!
            The returned pointer must be dropped when your done with it.
         
            Free movement controllers can be added to nodes so they can fly around a scene. The controller can be 
			controlled by keyboard and mouse.
         
			\param playerId Id from InputManager::CreatePlayer().
            \param inputContext The input context the input should use. See MashInputManager for more info.
            \param rotationSpeed Input rotation speed.
            \param linearSpeed Linear movement speed.
			\param customActions Custom key actions. Leave as NULL to use defaults from MashInputManager::CreateDefaultActionMap().
            \return New controller.
        */
        virtual MashFreeMovementController* CreateFreeMovementController(uint32 playerId, 
			const MashStringc &inputContext, 
			f32 rotationSpeed = 0.1f, 
			f32 linearSpeed = 10.0f, 
			const MashFreeMovementController::sInputActionMovement *customActions = 0) = 0;
        
        //! Creates a new instance of a scene graph and adds it to the manager.
        /*!
            The returned pointer should not be dropped.
         
            This method should be used to instance scene nodes rather than using
            MashSceneNode::_CreateInstance().
         
            If creating a new instance of a skinned entity, the root node passed in
            should contain the entity and all affecting bones.
         
            Setting createSkinInstances to true will create a new skin instance for any
            node that has one set. The new skin instance will be filled with the new
            instanced bones. If this value is false then the skin pointer will be shared
            between the original and instanced nodes. 
         
            \param instanceFrom Node graph to create a new instance from.
            \param parent Parent to attach the new instance.
            \param instanceName Instance root node name.
            \param createSkinInstances Creates new skin instances for each node that has a skin.
            \return New instance.
        */
        virtual MashSceneNode* AddInstance(MashSceneNode *instanceFrom, MashSceneNode *parent, const MashStringc &instanceName, bool createSkinInstances = true) = 0;
        
        //! Adds a camera scene node to the manager.
        /*!            
            A camera can be set as the active camera by using SetActiveCamera(). That camera is
            then responsible for rendering the scene. That camera must still be attached to a scene graph
            for updating, it is not done by the manager.
         
            \param parent Parent scene node. May be NULL.
            \param name Name for the node.
            \return New camera node.
        */
		virtual MashCamera* AddCamera(MashSceneNode *parent, const MashStringc &name) = 0;
        
        //! Adds a light to the scene manager.
        /*!
            The returned pointer should not be dropped.
         
            In deferred rendered scenes, all lights attached to a scene graph will be used
            to light a scene.
            MashLight::SetForwardRenderedLight() can be used to set a light to light objects
            using forward rendering. Note these lights will still light objects using deferred
            rendering.
         
            Shadows can be enabled or disabled using MashLight::SetShadowsEnabled(). In a forward
            rendered scene only the main light can create shadows.
         
            \param parent Parent scene node. May be NULL.
            \param name Name for the node.
            \param lightType Light type. This may be changed later.
            \param lightRendererType Is deferred or forward render type.
            \param mainLight The main light in a forward rendered scene is the only light that can emit shadows. 
            \return New light node.
        */
		virtual MashLight* AddLight(MashSceneNode *parent, const MashStringc &name, eLIGHTTYPE lightType, eLIGHT_RENDERER_TYPE lightRendererType, bool mainLight) = 0;
        
        //! Adds an entity to the scene manager.
        /*!
            The returned pointer should not be dropped.
         
            Entities hold the scene geoemtry. This maybe buildings, trees, characters, etc...
            Entities maybe animated using skinning via bones.
            
            They automatically handle mesh lodding based on options set by the user in MashEntity.
            
            \param parent Parent scene node. May be NULL.
            \param model Model this entity will be created with.
            \param name Name for the node.
            \return New entity node.
        */
		virtual MashEntity* AddEntity(MashSceneNode *parent, MashModel *model, const MashStringc &name) = 0;
        
        //! Adds an entity to the scene manager.
        /*!
            The returned pointer should not be dropped.
         
            Entities hold the scene geoemtry. This maybe buildings, trees, characters, etc...
            Entities maybe animated using skinning via bones.
         
            They automatically handle mesh lodding based on options set by the user in MashEntity.
         
            The model will need to be set for the returned pointer.
         
            \param parent Parent scene node. May be NULL.
            \param name Name for the node.
            \return New entity node.
        */
		virtual MashEntity* AddEntity(MashSceneNode *parent, const MashStringc &name) = 0; 
		
        //! Adds a particle system to the manager.
		/*!
            The returned pointer should not be dropped.
         
            Particle systems use built in materials for ease of use and can be created to
            favor CPU or GPU computation depending on the performance of your scene.
         
            Soft particles can be created that fade out particle edges when intersecting
            scene geoemtry. This results in nicer looking particles. These utilise data
            from the deferred renderer therefore they must use forward rendering and the
            scene geometry that affects it must use deferred rendering.
         
			If all particle systems in a scene will use the same textures then set createMaterialInstance
			to false. Those particle systems will then share the same material pointer. This will reduce
            memory consuption and may increase performance.
         
            Particle systems with custom materials can be created using AddParticleSystemCustom().

			For textures on Mesh particles:
				- rgb = diffuse
				- a = specular
			The alpha value of the whole mesh can be controlled via the particle settings.
         
            \param parent Parent scene node. May be NULL.
            \param name Name for the node.
			\param settings Particle settings.
            \param particleType Particle type.
            \param lightingType Lighting type. Vertex or no lighting can improve application performance.
            \param createMaterialInstance The material set will be instanced from a base material. This will allow
                you to set different textures to different systems. Set to false if all systems are using the same textures.
            \param model Model used for mesh particles. Leave as NULL if not using mesh particles.
            \return New particle system node.
		*/
		virtual MashParticleSystem* AddParticleSystem(MashSceneNode *parent, 
			const MashStringc &name, 
			const sParticleSettings &settings,
			ePARTICLE_TYPE particleType = aPARTICLE_CPU, 
			eLIGHTING_TYPE lightingType = aLIGHT_TYPE_NONE,
			bool createMaterialInstance = true,
			MashModel *model = 0) = 0;
			

        //! Adds a particle system to the manager.
        /*!
            The returned pointer should not be dropped.
         
            Consider using AddParticleSystem() for built implimentations.
         
            This allows you to creat particle systems that utilise custom materials.
            The vertex usages that are supported are :
                aDECLUSAGE_POSITION
                aDECLUSAGE_NORMAL
                aDECLUSAGE_TEXCOORD
         
            \param parent Parent scene node. May be NULL.
            \param name Name for the node.
			\param settings Particle settings.
            \param material Custom material.
            \return New particle node.
        */
		virtual MashParticleSystem* AddParticleSystemCustom(MashSceneNode *parent, 
			const MashStringc &name, 
			const sParticleSettings &settings,
			MashMaterial *material) = 0;

        //! Adds a new bone node to the manager.
        /*!
            The returned pointer should not be dropped.
         
            These are used for skinned animation of entities and adding to MashSkin objects.
            
            \param parent Parent scene node. May be NULL.
            \param name Name for the node.
            \return New particle node.
        */
		virtual MashBone* AddBone(MashSceneNode *parent, const MashStringc &name) = 0;

        //! Adds a dummy node to the scene.
        /*!
            The returned pointer should not be dropped.
         
            These are just empty nodes that can be used for any purpose.
         
            \param parent Parent scene node. May be NULL.
            \param name Name for the node.
            \return New particle node.
        */
		virtual MashDummy* AddDummy(MashSceneNode *parent, const MashStringc &name) = 0;
        
        //! Adds a decal node to the scene manager.
        /*!
            The returned pointer should not be dropped.
         
            Decal nodes are objects that batch render many decals. They can be attached to
            other scene nodes so the decals move with a node.
         
            Set decal limit to < 0 for static buffers. Vertices can only be appended
            before the first render. This is perfect for decals on static landscape
            such as posters or graffiti on buildings. Static buffers will have better
            performance than dynamic. Setting a decal limit > 0 means decals can be added
            at runtime for thing like bullet holes or tyre skid marks.
         
            If all decal nodes in a scene will use the same textures then set createMaterialInstance
            to false. This will reduce memory consuption and may increase performance.
         
            \param parent Parent scene node. May be NULL.
            \param name Name for the node.
            \param decalType Decal type.
            \param decalLimit See descript for details.
            \param lightingType Lighting type. Use vertex or none for better performance.
            \param createMaterialInstance The material set will be instanced from a base material. This will allow
                you to set different textures to different systems. Set to false if all systems are using the same textures.
            \return New decal node.
         */
		virtual MashDecal* AddDecal(MashSceneNode *parent,
                                   const MashStringc &name,
                                   eDECAL_TYPE decalType = aDECAL_STANDARD,
                                   int32 decalLimit = 20,
                                   eLIGHTING_TYPE lightingType = aLIGHT_TYPE_NONE,
                                   bool createMaterialInstance = true) = 0;
        
        //! Adds a custom decal node to the scene manager.
        /*!
            The returned pointer should not be dropped.
         
            See AddDecal() for more details.
         
            This allows you to create decals that use custom materials. The vertex usages that are supported
            are :
                aDECLUSAGE_POSITION
                aDECLUSAGE_NORMAL
                aDECLUSAGE_TEXCOORD
                aDECLUSAGE_BLENDWEIGHT
                aDECLUSAGE_BLENDINDICES
         
            Custom decals can be used to create materials for skinned decals. These might be used for decals
            that deform as the entity they are applied to deform/animate using skinning. Handy for showing
            bullet holes on skinned entities. In this case you would want to attach this node to the entity
            so it inherits all transforms.
            
            \param parent Parent scene node. May be NULL.
            \param name Name for the node.
            \param material Custom decal material.
            \param decalLimit See descript for details.
            \param skin Skin that will be used for animated decals. Set to NULL if not being used.
            \return New decal node.
        */
		virtual MashDecal* AddDecalCustom(MashSceneNode *parent,
                                         const MashStringc &name,
                                         MashMaterial *material,
                                         int32 decalLimit = 20,
                                         MashSkin *skin = 0) = 0;
        
        //! Creates an empty model.
        /*!
            The returned pointer must be dropped by the user when done.
         
            Models are containers for a single mesh or a group of meshes. 
            Entities can use these to determine what mesh to use at particular lod levels.
         
            Multiple meshes may be added to a lod level to allow different materials
            to be applied to them.
         
            \return New model.
        */
		virtual MashModel* CreateModel() = 0;
        
        //! Creates a model from a list of meshes.
        /*!
            The returned pointer must be dropped by the user when done.
         
            Models are containers for a single mesh or a group of meshes. 
            Entities can use these to determine what mesh to use at particular lod levels.
         
            \param meshLodList Meshes to add to the new model. One element per lod. meshLodList[lod][mesh].
            \return New model.
        */
		virtual MashModel* CreateModel(MashArray<MashArray<MashMesh*> > &meshLodList) = 0;
        
        //! Creates a new skin.
        /*!
            The returned pointer must be dropped by the user when done.
            
            Skins are a collection of MashBone and are used for skinned animation of entities. Once filled
            they should be set to the entity they affect.
            
            \return An empty skin.
        */
        virtual MashSkin* CreateSkin() = 0;

        //! Creates a new triangle from a mesh.
		/*!
            The returned pointer must be dropped by the user when done.
         
            Triangle buffers are a runtime accessible collection of triangles. These can be used
            to create triangle colliders or used to create physics colliders. Once created this
            should be set to the mesh that will use it.

			This function does not set it to the mesh automatically because you
			may want to create a collision mesh for a high poly mesh from a low poly version.
         
            \param mesh Mesh to create the buffer from.
            \return New triangle buffer.
		*/
		virtual MashTriangleBuffer* CreateTriangleBuffer(MashMesh *mesh) = 0;
        
        //! Creates a new empty triangle buffer.
        /*!
            The returned pointer must be dropped by the user when done.
         
            Triangle buffers are a runtime accessible collection of triangles. These can be used
            to create triangle colliders or used to create physics colliders. Once created this
            should be set to the mesh that will use it.
         
            \return New triangle buffer.
        */
		virtual MashTriangleBuffer* CreateTriangleBuffer() = 0;

        //! Create a triangle collider.
		/*!
            The returned pointer must be dropped by the user when done.
         
            Triangle colliders and be used for ray picking, applying decals and collision detection.
            Colliders can be shared by one or many models for collision detection.
			Once created this should be set to the model(s) that will use it. Entities will then
            access a models collider during collision detection.

			The param waitForDeserialize is used for scene loading only and should be set to 'false' in all other cases.
			Setting waitForDeserialize to true allows you to call MashTriangleCollider::Deserialize() with data previously saved
			using MashTriangleCollider::Serialize().
         
            \param buffer An array of buffers that will make up this collider.
            \param bufferCount Number of buffers in the array.
            \param type Collider type to create.
			\param waitForDeserialize Thie should almost always be 'false'. Only set this to true if you will be calling MashTriangleCollider::Deserialize() with serialized data.
            \return New triangle collider.
		*/
		virtual MashTriangleCollider* CreateTriangleCollider(MashTriangleBuffer **buffer, uint32 bufferCount, eTRIANGLE_COLLIDER_TYPE type = aTRIANGLE_COLLIDER_STANDARD, bool waitForDeserialize = false) = 0;
		
        //! Creates a triangle collider.
        /*!
            The returned pointer must be dropped by the user when done.
         
            Creates a triangle collider from each mesh at a particular lod that contains a triangle buffer.
            The triangle buffer is then added to the model where it may be removed or changed if the user wishes too.
         
            Triangle colliders and be used for ray picking, applying decals and collision detection.
            Colliders can be shared by one or many models for collision detection.
            Once created this should be set to the model(s) that will use it. Entities will then
            access a models collider during collision detection.
         
            \param model Model to search for triangle buffers.
            \param lod Lod to search for triangle buffers. 
            \param type Collider type to create.
            \param generateTriangleBufferIfNull Creates a triangle buffer for each mesh at the given lod if one has
                not already been created.
            \return New triangle collider.
		*/
		virtual MashTriangleCollider* CreateTriangleCollider(MashModel *model, uint32 lod = 0, eTRIANGLE_COLLIDER_TYPE type = aTRIANGLE_COLLIDER_STANDARD, bool generateTriangleBufferIfNull = true) = 0;

        //! Creates a new culling technique.
        /*!
            The returned pointer must be dropped by the user when done.
         
            Culling techniques are used to cull a scene for rendering.
         
            This creates built in culling techniques. You can create custom techniques by deriving
            from MashCullTechnique.
         
            Techniques must be set using SetCullTechnique() or SetCullTechniqueShadow().
         
            \param tech Technique to create.
        */
		virtual MashCullTechnique* CreateCullTechnique(eCULL_TECHNIQUE tech) = 0;
        
        //! Sets a culling techniques
        /*!
            Culling techniques are used to cull a scene for rendering. Nodes that pass this technique
            will go on to be rendered.
         
            Some nodes leave some updating till they pass this technique. This is to save on operations
            that are only needed if the node is to be rendered.
         
            This will not cull a scene during the shadow pass, use SetCullTechniqueShadow() instead.
            These operations are split up so they can be optimized for a particular situations. Also custom
            shadow casters may need to use specialzed culling techniques.
         
            \param tech Technique to set. Any previous techniques will be dropped.
        */
		virtual void SetCullTechnique(MashCullTechnique *tech) = 0;
        
        //! Sets a culling techniqued used for the shadow pass.
        /*!
            This technique will only be used to cull during the shadow pass. 
            See SetCullTechnique() for more info.
         
            \param tech Technique to set. Any previous techniques will be dropped.
        */
		virtual void SetCullTechniqueShadow(MashCullTechnique *tech) = 0;

        //! Enable for disable transparent objects from casting shadows.
        /*!
            Stops objects with transparent materials from casting shadows.
         
            \param value Enable or disable transparent shadow casting.
        */
		virtual void EnableTransparentObjectShadowCasting(bool value) = 0;

        //! Sets the shadow caster for a particular light.
		/*!
			Replaces and sets a lights shadow caster with the one provided.
         
            MashShadowCaster can be of built in type or custom made.
         
            \param type Light type to assign this caster to.
            \param caster New shadow caster. Any previous caster set will be dropped.
		*/
		virtual void SetShadowCaster(eLIGHTTYPE type, MashShadowCaster *caster) = 0;

        //! Creates a built in shadow caster.
        /*!
            The returned pointer must be dropped by the user when done.
         
            Custom casters can be created by deriving from MashShadowCaster.
         
            \param casterType Caster to create.
            \param parameters Any parameters that may need to be set.
            \return New shadow caster.
        */
        
        //! Creates a shadow caster for directional lights.
        /*!
            The returned caster should be set using SetShadowCaster().
            See MashDirectionalShadowCascadeCaster for more informtion.
         
            \param casterType Sets the algorithm type.
            \return Shadow caster.
        */
		virtual MashDirectionalShadowCascadeCaster* CreateDirectionalCascadeShadowCaster(MashDirectionalShadowCascadeCaster::eCASTER_TYPE casterType = MashDirectionalShadowCascadeCaster::aCASTER_TYPE_STANDARD) = 0;
        
        //! Creates a shadow caster for spot lights.
        /*!
            The returned caster should be set using SetShadowCaster().
            See MashSpotShadowCaster for more informtion.

            \param casterType Sets the algorithm type.
            \return Shadow caster.
         */
		virtual MashSpotShadowCaster* CreateSpotShadowCaster(MashSpotShadowCaster::eCASTER_TYPE casterType = MashSpotShadowCaster::aCASTER_TYPE_STANDARD) = 0;
        
        //! Creates a shadow caster for point lights.
        /*!
            The returned caster should be set using SetShadowCaster().
            See MashPointShadowCaster for more informtion.

            \param casterType Sets the algorithm type.
            \return Shadow caster.
         */
		virtual MashPointShadowCaster* CreatePointShadowCaster(MashPointShadowCaster::eCASTER_TYPE casterType = MashPointShadowCaster::aCASTER_TYPE_STANDARD) = 0;

        //! Saves current shadow casters data to xml file.
        /*!
            This can be used for editors to save out shadow casters that have 
            been set using SetShadowCaster().
            The saved file can be loaded using LoadShadowCastersFromFile().
         
            \param filename eg, shadowdata.xml
            \return ok on success, failed otherwise.
        */
		virtual eMASH_STATUS SaveShadowCastersToFile(const MashStringc &filename) = 0;
        
        //! Loads shadow caster data from file.
        /*!
            Any previous casters set using SetShadowCaster() will be dropped.
         
            customFunctor can be used to load custom casters (casters other than eSHADOW_CASTER_TYPE) 
            that have been saved. A struct will be passed to the function that contains the loaded 
            caster type and a pointer to return a new'd caster object.
         
            \param filename eg, shadowdata.xml
            \param customFunctor Custom caster loader.
            \return ok on success, failed otherwise.
        */
		virtual eMASH_STATUS LoadShadowCastersFromFile(const MashStringc &filename, MashLoadCasterFunctor customFunctor = MashLoadCasterFunctor()) = 0;

        //! Debug triangle buffer renderer.
		/*!
            A mesh that can be rendered efficiently will be returned.
         
			Triangles with closed edges will be rendered in green. Open edges are red.
            \param buffer Buffer to render.
            \param worldTransform Triangles will be transformed by this matrix.
            \param out Generated vertex data for rendering.
            \return ok on success, failed otherwise.
		*/
		virtual eMASH_STATUS CreateTriangleBufferAdjacencyRenderBuffer(const MashTriangleBuffer *buffer, const MashMatrix4 &worldTransform, MashArray<MashVertexColour::sMashVertexColour> &out) = 0;
        
        //! Debug triangle buffer renderer.
		/*!
            A mesh that can be rendered efficiently will be returned.

            Triangles with closed edges will be rendered in green. Open edges are red.
         
            \param model Triangle buffers will be collected from this model.
            \param worldTransform Triangles will be transformed by this matrix.
            \param out Generated vertex data for rendering.
            \return ok on success, failed otherwise.
         */
		virtual eMASH_STATUS CreateTriangleBufferAdjacencyRenderBuffer(const MashModel *model, const MashMatrix4 &worldTransform, MashArray<MashVertexColour::sMashVertexColour> &out) = 0;
		
        //! Gets the shadow caster assigned to a light type.
        /*!
            \param type Light type to return.
            \return Shadow caster. NULL if nothing set.
        */
        virtual MashShadowCaster* GetShadowCaster(eLIGHTTYPE type)const = 0;
        
        //! True if transparent shadow casting is enabled.
        /*!
            \return True if transparent shadow casting is enabled. False otherwise.
        */
		virtual bool IsTransparentObjectShadowCastingEnabled()const = 0;

        //! Gets the preferred lighting mode.
        /*!
            This is the lighting mode used when materials are set to auto.
            
            \return Preferred lighting mode.
        */
		virtual eLIGHTING_TYPE GetPreferredLightingMode()const = 0;
		
        //! Sets the preferred lighting mode.
        /*!
            This is the lighting mode used when materials are set to auto.
			This will force a rebuild on all the shaders.
         
            \param type Lighting type used for any materials using auto lighting.
		*/
		virtual void SetPreferredLightingMode(eLIGHTING_TYPE type) = 0;

        //! Sets the active camera.
        /*!
            This will be the camera responsible for rendering the scene. This camera must still be attached to a 
            scene graph for updating, it is not done by the manager.
         
            \param camera New active camera. Any previous camera set will be dropped.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS SetActiveCamera(MashCamera *camera) = 0;
        
        //! Sets the active camera by name.
        /*!
            See SetActiveCamera() for more info.
         
            \param name Name of the camera to set.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS SetActiveCameraByName(const MashStringc &name) = 0;
        
        //! Gets the active scene camera.
        /*!
            This will be the camera responsible for rendering the scene.
         
            \return Active camera.
        */
		virtual MashCamera* GetActiveCamera() = 0;

        //! Gets a scene node from the manager by name.
        /*!
            \param name Node to search for.
            \return The first node found with the given name. NULL if nothing was found.
        */
		virtual MashSceneNode* GetSceneNodeByName(const MashStringc &name)const = 0;
        
        //! Gets a scene node from the manager by node id.
        /*!
            \param nodeId Node id to search for.
            \return Found node, else null.
        */
		virtual MashSceneNode* GetSceneNodeByID(uint32 nodeId)const = 0;
        
        //! Gets a scene node from the manager by user id.
        /*!
         \param userId Node id to search for.
         \return The first node found with the given user id.
         */
		virtual MashSceneNode* GetSceneNodeByUserID(int32 userId)const = 0;
        
        //! Gets the number of scene nodes within the manager.
        /*!
            \return Number of scene nodes within the manager.
        */
		virtual uint32 GetSceneNodeCount()const = 0;

        //! Gets the internal list of scene nodes.
        /*!
            \return Scene node list. This list should not be modified.
        */
		virtual const MashList<MashSceneNode*>& GetSceneNodeList()const = 0;

        //! Removes all scene nodes from the manager.
		/*!
			Drops all scene nodes from the internal list. If the nodes have been grabbed elsewhere
            they may still be active after calling this.
		*/
		virtual void RemoveAllSceneNodes() = 0;
		
        //! Removes a scene node from the manager.
        /*
			Drops a node from the internal list and detaches it from its parent. This should result in it being deleted
            if the node has not been grabbed elsewhere.
         
            \param node Node to remove.
		*/
		virtual void RemoveSceneNode(MashSceneNode *node) = 0;

        //! Batch compile method.
        /*!
            Batch compiles all materials. This function will compile materials faster than compiling many materials 
            individually. This is automatically called at the end of MashGameLoop::Initialise() but can be called
            manually if materials are created after that.
         
            \param compileFlags Bitwise eMATERIAL_COMPILER_FLAGS.
        */
        virtual void CompileAllMaterials(uint32 compileFlags = 0) = 0;
        
        //! Scene manager update.
		/*!
			Called by the user from MashGameLoop::Update() to update a scene graph.
            This can be called multiple times by the user if a nodes transform is updated 
            after calling this.
         
            This will call MashSceneNode::Update() for the root node.
         
            \param dt Time passed since last update.
            \param scene Scene graph to update.
            \return Ok on success, failed otherwise.
		*/
		virtual eMASH_STATUS UpdateScene(f32 dt, MashSceneNode *scene) = 0;
        
        //! Culls a scene and sets it up for rendering.
        /*!
            This is called by the user from MashGameLoop::Render() before calling DrawScene().
         
            The culling techniques used can be set by SetCullTechniques().
         
            Scene nodes that pass a culling technique are aded to internal render buckets that
            will be flushed when calling DrawScene().
         
            Some scene nodes leave updating render specific data till it passes culling to improve
            performace.
         
            This can be called as many times as needed before calling DrawScene().
         
            \param scene Scene graph to cull.
            \return Ok on success, failed otherwise.
        */
        virtual eMASH_STATUS CullScene(MashSceneNode *scene) = 0;
        
        
        //! Adds a renderable object for scene rendering.
        /*!
            Objects that pass culling call this to add it for rendering. This may also be called
            to manually add a custom renderable for rendering. Custom objects derived from MashRenderable
            can be implimented by the user.
         
            A renderable can be added to all stages if needed. Just call this function multiple times for
            that object.
         
            \param renderable Renderable object to add for scene rendering.
            \param pass Determines what order this object will be rendered in. Most objects use aHLPASS_SCENE.
            \param state Determines how it will be used. As a shadow caster or scene object. 
        */
        virtual void AddRenderableToRenderQueue(MashRenderable *renderable, eHLRENDER_PASS pass, eRENDER_STAGE stage) = 0;
        
        //! Draws a culled scene.
        /*!
            This is called by the user from MashGameLoop::Render() after calling CullScene(). 
            This will draw any objects that  passed CullScene() or where manually add using 
            AddRenderableToRenderQueue().
         
            CullScene() maybe called many times before calling this to fill the render buckets.
            DrawScene() can be called as many time as needed after a CullScene().
         
            After calling this all render buckets are flushed.
         
            \return Ok on success, failed otherwise.
        */
        virtual eMASH_STATUS DrawScene() = 0;
        
        //! Draws 3D lines in the scene using the active camera.
        /*!
            This uses a line list. So 2 vertices are needed per line.
         
            \param lines List of vertices.
            \param vertexCount Number of vertices in the list.
			\param depthTest Set to true to enable depth testing. False to overwrite any previous pixel value.
            \return Ok on success, failed otherwise. 
         */
		virtual eMASH_STATUS DrawLines(const MashVertexColour::sMashVertexColour *lines, uint32 vertexCount, bool depthTest = true) = 0;

		//! Draws a 3D line in the scene using the active camera.
        /*!         
            \param start Line start.
            \param end Line end.
			\param colour Line colour.
			\param depthTest Set to true to enable depth testing. False to overwrite any previous pixel value.
            \return Ok on success, failed otherwise. 
         */
		virtual eMASH_STATUS DrawLine(const MashVector3 &start, const MashVector3 &end, const sMashColour &colour, bool depthTest = true) = 0;
		
        //! Draws a wireframe 3D AABB in the scene using the active camera.
        /*!
            \param box AABB to draw.
            \param colour Colour to render the box.
			\param depthTest Set to true to enable depth testing. False to overwrite any previous pixel value.
            \return Ok on success, failed otherwise. 
        */
        virtual eMASH_STATUS DrawAABB(const MashAABB &box, const sMashColour &colour, bool depthTest = true) = 0;

        //! Gets the first light in the forward rendered light list.
        /*!
            This is the main light that can render shadows in forward rendered scenes.
         
            \return First forward rendered light. NULL if no light is set.
        */
		virtual MashLight* GetFirstForwardRenderedLight()const = 0;
        
        //! Gets the forward rendering light list.
        /*!
            \return Forward rendering light list. This list should not be modified.
        */
		virtual const MashArray<MashLight*>& GetForwardRenderedLightList()const = 0;
        
        //! Gets the active pass. Used during scene rendering.
		virtual eRENDER_STAGE GetActivePass()const = 0;
        
        //! Gets the mesh builder.
        virtual MashMeshBuilder* GetMeshBuilder()const = 0;
        
        //! Gets the controller manager.
		virtual MashControllerManager* GetControllerManager()const = 0;
        
        //! Gets the number of forwrd rendered lights.
        virtual uint32 GetForwardRenderedLightCount()const = 0;
        
        //! Returns true if shadows are enabled on the main forward rendered light.
		virtual bool GetForwardRenderedShadowsEnabled()const = 0;
        
        //! Returns true if any directional lights have shadows enabled.
		virtual bool GetDeferredDirShadowsEnabled()const = 0;
        
        //! Returns true if any spot lights have shadows enabled.
		virtual bool GetDeferredSpotShadowsEnabled()const = 0;
        
        //! Returns true if any point lights have shadows enabled.
		virtual bool GetDeferredPointShadowsEnabled()const = 0;
        
        //! Returns true if the deferred renderer has been successfully created.
        /*!
            If this returns true then the deferred render targets and materials
            have been created.
        */
        virtual bool IsDeferredRendererInitialised()const = 0;
        
        //! Sets the hash function used to sort scene nodes before rendering.
        /*!
            This is used to order similar techniques together when rendering scene objects
            to reduce state changes. Grouping similar techniques together may improve
            performance when rendering.
         
            This function creates a 32bit hash based on factors that may include rasterizer
            state, blend state, technique id, and texture at indexs 0, 1, 2, ....
         
            Note that technique instances will only update their keys the first time they are drawn,
            then each time textures change. So if anything is changed in MashTechnique after
            the first draw call then the key wont reflect that change. The key will only
            update after the first draw call if textures change in MashTechniqueInstace.
         
            \param p Function pointer to determine techniqu hash. Pass NULL to set to the default hash function.
        */
        virtual void SetRenderKeyHashFunction(uint32 (*p)(const MashTechniqueInstance *technique)) = 0;
        
        //! Generates a render key for a technique.
        /*!
            The key returned is generated from a hashing function set with SetRenderKeyHashFunction().
            This is called automatically by techniques when needed.
         
            \param technique Technique to hash.
            \return Hashed render key.
        */
        virtual uint32 GenerateRenderKeyForTechnique(const MashTechniqueInstance *technique) = 0;

		//! Gets debug render info.
		virtual const MashSceneManager::sSceneRenderInfo* GetCurrentSceneRenderInfo() = 0;

		//! Gets the current updating scene node.
        /*!
            This is access by the script manager.
         
            \return Current updating scene node.
        */
		virtual MashSceneNode* GetCurrentScriptSceneNode()const = 0;
        
        //! Creates a new skin during instancing.
        /*!
            The returned pointer must be dropped when finished.
         
            Used during instancing to create a skin.
         
            This will either return the original pointer or an new empty skin
            based on the arguments used in MashSceneManager::AddInstance().
         
            \param instanceFrom Skin to instance from.
            \return Skin pointer.
        */
        virtual MashSkin* _CreateSkinInstance(MashSkin *instanceFrom) = 0;
		
        //! Sets the current updating scene node.
        /*!
            Called by scene nodes to notify the manager they are currently updating.
            This data is then accessed by the script manager.
         
            \param node Active node.
        */
		virtual void _SetCurrentScriptSceneNode(MashSceneNode *node) = 0;
        
        //! Adds a light to the forward renderer.
        /*!
            Internal use only.
            This will update runtime shaders if needed.
         
            \param light Light to set.
            \param setAsMain Sets this light as the main light that can cast shadows.
        */
        virtual void _AddForwardRenderedLight(MashLight *light, bool setAsMain) = 0;
        
        //! Removes a light from the forward rendered light list.
        /*!
            Internal use only.
            This will update runtime shaders if needed.
         
            \param light Light to remove.
        */
		virtual void _RemoveForwardRenderedLight(MashLight *light) = 0;
        
        //! Called by lights whehn their light type changes.
        /*!
            This will udpating runtime shaders if needed.
         
            \param light Light that changed.
        */
		virtual void _OnLightTypeChange(MashLight *light) = 0;
        
        //! Adds a node that has callbacks.
        /*!
            The callbacks will be updated before MashGameLoop::Update() each frame.
         
            \param node Node to add.
        */
        virtual void _AddCallbackNode(MashSceneNode *node) = 0;
        
        //! Removes a node that had callbacks.
        /*!
            \param node Node to remove.
        */
        virtual void _RemoveCallbackNode(MashSceneNode *node) = 0;
        
        //! Adds nodes that automatically look at other nodes as they move.
        /*!
            This nodes rotation will be updated after MashGameLoop::Update().
            
            \param node Node to automatically update.
        */
        virtual void _AddLookAtTracker(MashSceneNode *node) = 0;
        
        //! Removes a node from the tracker list.
		virtual void _RemoveLookAtTracker(MashSceneNode *node) = 0;
        
        //! Called internally before MashGameLoop::Initilize().
        /*!
            Sets the scene manager up for user initialization.
        */
		virtual void _OnBeginUserInitialise() = 0;
        
        //! Called internally after MashGameLoop::Initilize().
        /*!
            This will compile any non compiled materials, remove any loading data,
            and set up the scene as needed.
        */
		virtual void _OnEndUserInitialise() = 0;

		//!  Updates the scene manager.
        /*!
            \param dt Time passed since last frame.
        */
		virtual void _Update(f32 dt) = 0;
        
        //! Updates anything that is left till the user is done updating.
        /*!
            This includes updating lookat trackers.
        */
        virtual void _LateUpdate() = 0;

        //! Called after a resolution change.
		virtual void _OnPostResolutionChange() = 0;
        
        //! Called on a viewport change.
		virtual void _OnViewportChange() = 0;
        
        //! Adds a custom render path to the internal flush list.
		virtual void _AddCustomRenderPathToFlushList(MashCustomRenderPath *batch) = 0;

        //! Called on material light type changes.
		virtual void _OnDeferredLightingModeEnabled() = 0;
        
        //! Called when a light enables shadows.
		virtual void _OnShadowsEnabled(MashLight *light) = 0;
        
        //! Called when a light disables shadows.
		virtual void _OnShadowsDisabled(MashLight *light) = 0;

		//! Called by a shadow caster when the receiver material needs to be updated.
		/*!
			A shadow casters paramters may change and this allows a caster to
			force the scene manager to update all scene materials that may
			use shadows.

			\param caster Caster that has been updated.
		*/
		virtual void OnShadowReceiverMaterialRebuildNeeded(MashShadowCaster *caster) = 0;

		//! Called by a shadow caster when the caster material needs to be updated.
		/*!
			A shadow casters paramters may change and this allows a caster to
			force the scene manager to update all scene materials that may
			use shadows.

			\param caster Caster that has been updated.
		*/
		virtual void OnShadowCasterMaterialRebuildNeeded(MashShadowCaster *caster) = 0;
        
        //! Adds a light to the current render frame.
        virtual void _AddLightToCurrentRenderScene(MashLight *light) = 0;
	};
}

#endif