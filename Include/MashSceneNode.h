//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_SCENE_NODE_H_
#define _MASH_SCENE_NODE_H_

#include "MashReferenceCounter.h"
#include "MashEnum.h"
#include "MashTypes.h"
#include "MashMatrix4.h"
#include "MashVector3.h"
#include "MashQuaternion.h"
#include "MashString.h"
#include "MashList.h"
#include "MashArray.h"
#include "MashAABB.h"
#include "MashCullTechnique.h"
#include "MashTransformState.h"

namespace mash
{
	class MashSceneManager;
	class MashAnimationBuffer;
	class MashAnimationMixer;
	class MashTriangleCollider;
	class MashSceneNodeCallback;
	class MashTimer;
    
    /*!
        Base class for all scene nodes. Nodes are arranged in a parent child hierarchy.
     
        To create a new instance of previous nodes created use MashSceneManager::AddInstance().
        
        The transforms for nodes consist of local, absolute, and interpolated states. 
        Local contains transforms in its own space (not influenced by the parent).
        Absolute is the local position with the parents absolute transform added.
        Interpolated is the transform used for rendering and is ONLY valid when it has passed culling.
        This is calculated in OnCullPass(). Nodes maybe updated many times per frame, or skipped entirely and only
        rendered on some frames depending on the frame rate. The interpolated transform starts from the last 
        updated absolute position and interpolates to the position it's expected to be at by the next update 
        (the absolute position). This removes jumpy animation as frame rates vary.
     
        Many local position methods have SnapToPosition parameters that stop render interpolation for that frame
        and snaps it to the transform set. This is good for initialisation only.
     
        SetInheritTranslationOnly() can be handy for displaying icons or names above nodes without having them rotate
        with its parent. Children will then only inherit translation, not rotation or scale.
     
        Scene nodes and their transforms are updated when needed from the Update() method. This is automatically 
        called by the scene manager so it shouldn't need to be called directly. Some nodes will leave updating 
        render only data till they pass culling to save calculating unneeded data. The culling technique should 
        call OnCullPass() to make sure the node is fully updated when drawn.
    */
	class MashSceneNode : public MashReferenceCounter
	{
	public:
		enum eSCENE_UPDATE_FLAGS
		{
            aUPDATE_FLAG_TRANSFORM = 1,
            aUPDATE_FLAG_CHILD_TRANSFORM = 2,
			aUPDATE_ALL = 0xFFFFFFFF
		};

		enum eNODE_SNAP_FLAGS
		{
			aNODE_SNAP_TRANSLATION = 1,
			aNODE_SNAP_ROTATION = 2,
			aNODE_SNAP_SCALE = 4,
			aNODE_SNAP_ALL = 0xFFFFFFFF
		};
    protected:
        // Implimented by derived classes and called when the world transform changes.
        virtual void OnNodeTransformChange(){}
        // Implimented by derived classes and called when the node passes culling.
        virtual void OnPassCullImpl(f32 interpolateAmount){}
		void InstanceMembers(MashSceneNode *from);        
        uint32& GetUpdateFlags();
        // returns true if any flags are set.
        bool IsUpdateNeeded()const;
        void WorldTransformUpdateNeeded();

        MashSceneManager *m_sceneManager;
    private:

		struct sCallback
		{
			MashSceneNodeCallback *callback;
			uint32 order;

			sCallback(){}
			sCallback(MashSceneNodeCallback *_callback, uint32 _order):callback(_callback), order(_order){}

			bool operator<(const sCallback &rhs)const
			{
				//reverse operation
				return order > rhs.order;
			}
		};

		MashArray<sCallback> m_nodeCallbacks;
        MashArray<MashSceneNode*> m_childrenToUpdate;

		MashAABB m_absoluteBoundingBox;
        MashAABB m_totalAABB;
		MashTransformState m_relativeTransformState;

		MashTransformState m_absoluteTransformStartState;
		MashTransformState m_absoluteTransformEndState;

		/*
			Interp transformations are only calculated after a cull pass.
		*/
		mutable MashTransformState m_interpolatedTransformState;
		mutable MashMatrix4 m_interpolatedTransformation;

		MashSceneNode *m_parent;
		MashList<MashSceneNode*> m_children;

		bool m_inheritTranslationOnly;
		MashSceneNode *m_lookatNode;
		MashVector3 m_lookatOffset;

		MashAnimationMixer *m_animationMixer;
		MashAnimationBuffer *m_animationBuffer;
		int32 m_userID;
		void* m_userData;
		MashStringc m_nodeName;

		bool m_isVisible;

		uint32 m_snapToPositionFlags;//flags are eNODE_SNAP_FLAGS
		bool m_addedToParentUpdate;
        mutable bool m_renderTransformUpdateNeeded;
        mutable bool m_renderTransformMatrixUpdateNeeded;
        mutable uint32 m_lastCullFrame;
		uint32 m_lastTransformUpdateRenderFrame;
		uint32 m_lastTransformUpdateFrame;
        mutable f32 m_interpolationTime;
        MashTimer *m_timer;
		/*
			This must be called after a call the Update() as
			it relies on an updated world bounds
		*/
		void RecalculateTotalBoundingBox();
		bool SetParent(MashSceneNode *pParent);
        void ChildUpdateNeeded(MashSceneNode *child);
        void _UpdateFromParent();
        void _Update(bool parentHasChanged);
        void PrepareForRenderTransformUpdate()const;
        const mash::MashTransformState& _GetRenderTransformState()const;
	private:

		static uint32 m_nodeCounter;
		uint32 m_internalNodeID;
        uint32 m_updateFlags;
	public:
		MashSceneNode(MashSceneNode *parent,
			MashSceneManager *manager,
			const MashStringc &name,
			const MashVector3 &position = MashVector3(0.f,0.f,0.f),
			const MashQuaternion &orientation = MashQuaternion(),
			const MashVector3 &scale = MashVector3(1.f,1.f,1.f));

		MashSceneNode(const MashSceneNode *pCopy, const int8 *sName);

		virtual ~MashSceneNode();
        
        //! Creates an instance of this node.
        /*!
            The returned pointer should not be dropped.
         
            Use MashSceneManager::AddInstance() instead of calling this directly.
         
            The instance will share some data from the base node including models and
            animation buffers. If this node has any children then they will also be
            instanced.
         
            See derived classed for more info.
         
            \param parent New parent for the instance.
            \param name New name for the instance.
        */
		virtual MashSceneNode* _CreateInstance(MashSceneNode *parent, const MashStringc &name) = 0;

        //! Adds a callback to this node.
        /*!
            Your copy of the callback can be dropped after adding it.

			Order can be important as some callbacks should be performed before others.
			For example, collision handling should be performed after movement controllers.
         
            \param callback Callback to add.
			\param order Highest numbers will be performed first.
        */
		void AddCallback(MashSceneNodeCallback *callback, uint32 order = 0);
		
		//! Finds the callback in this nodes list and drops its copy if found.
        /*!
            \param callback Callback to drop.
        */
        void RemoveCallback(MashSceneNodeCallback *callback);
		
		//! Gets the bounding box for this node in local space.
        /*!
            This does not contain child nodes.
        */
		virtual const MashAABB& GetLocalBoundingBox()const = 0;
        
        //! Gets the bounding box for this node in absolute space.
        /*!
            This does not contain child nodes.
         */
        const MashAABB& GetWorldBoundingBox()const;

		//! Gets the bounding box for this node AND its children in absolute space.
		const MashAABB& GetTotalBoundingBox()const;

        //! Gets the triangle collider for this node.
        /*!
            Only objects with meshes support this method, eg MashEntity.
         
            Triangle colliders can be used for ray picking, collision detection, or
            creating collision objects in the physics library.
         
            Triangle colliders are contained within MashModel.
         
            \return Triangle collider.
        */
		virtual const MashTriangleCollider* GetTriangleCollider()const;

		//! Adds a child to this node.
        /*!
            The child will inherit all transformations from its new parent. This
            behaviour can be modified using SetInheritTranslationOnly(). This can be
            handy for displaying icons or names above nodes without having them rotate
            with its parent.
         
            \param child Child node to add to this node.
            \return Failed if this node is parenting itself or is already a child.
        */
		eMASH_STATUS AddChild(MashSceneNode *child);
	
        //! Sets the local position.
        /*!
            \param position Local position.
            \param snapToPosition Stops interpolation for this frame and snaps the node to its new location.
        */
		void SetPosition(const MashVector3 &position, bool snapToPosition = false);
        
        //! Sets the local scale.
        /*!
            \param position Local scale.
            \param snapToPosition Stops interpolation for this frame and snaps the node to its new scale.
        */
		void SetScale(const MashVector3 &scale, bool snapToPosition = false);
        
        //! Sets the local rotation.
        /*!
            \param position Local orientation.
            \param snapToPosition Stops interpolation for this frame and snaps the node to its new orientation.
         */
		void SetOrientation(const MashQuaternion &orientation, bool snapToPosition = false);
        
        //! Adds onto the current local orientation.
        /*!
            \param position Local orientation.
            \param snapToPosition Stops interpolation for this frame and snaps the node to its new orientation.
        */
		void AddOrientation(const MashQuaternion &orientation, bool snapToPosition = false);
        
        //! Adds onto the current local position.
        /*!
            \param position Local position.
            \param snapToPosition Stops interpolation for this frame and snaps the node to its new position.
         */
		void AddPosition(const MashVector3 &position, bool snapToPosition = false);
        
        //! Adds onto the current local scale.
        /*!
            \param position Local scale.
            \param snapToPosition Stops interpolation for this frame and snaps the node to its new scale.
         */
		void AddScale(const MashVector3 &scale, bool snapToPosition = false);
        
        //! Sets the orientation so that this node faces the given direction.
        /*!
            \param direction Direction to face.
            \param snapToPosition Stops interpolation for this frame and snaps the node to its new scale.
        */
        void SetLookAtDirection(const MashVector3 &direction, bool snapToPosition = false);
        
        //! Sets the local transformation.
		/*!
            This will break the matrix down into scale, rotation, and translation. Therefore
            it's not the fastest method for setting the relative transformation.
         
            \param transformation New local transformation.
            \param snapToPosition Stops interpolation for this frame and snaps the node to its new transformation.
         */
		virtual void SetTransformation(const MashMatrix4 &transformation, bool snapToPosition = false);
        
        //! Gets the local transform state.
		const MashTransformState& GetLocalTransformState()const;
        
        //! Gets the absoulte transform state.
        /*!
            This is the destination of the interpolated (render) state.
            This is only valid after an update.
         
            \return World transform.
        */
		const MashTransformState& GetWorldTransformState()const;
        
        //! Gets the interpolated transform state.
        /*!
            The render transform interpolates from the last updates world transform 
            towards GetWorldTransformState() depending on the current frame rate.
            This will be the render position for the current frame.
         
            The render transform state helps to smooth out movement as frame rates change.
            This call will interpolate the render transform if needed.
         
            \return Render transform.
        */
		const MashTransformState& GetRenderTransformState()const;
        
        //! Gets the interpolated transform state as a matrix.
        /*!
            See GetRenderTransformState() for more info.
            This call will interpolate the render transform if needed.
         
            \return Interpolated transform state as a matrix.
        */
		const MashMatrix4& GetRenderTransformation()const;
		
        //! Gets the current snap to position flag.
        /*!
            This may change each update. Flags are of type eNODE_SNAP_FLAGS.
         
            \return Number other than 0 if the node is snapping, not interpolating, to its position this frame.
        */
		uint32 GetSnapToPositionFlags()const;

        //! The orientation of this node will automatically update to follow another nodes position.
        /*!
            The nodes orientation will be updated after MashGameLoop::Update(). It needs to be left till
            then so that the node its tracking is in its final position when tracking is calculated.
         
            \param enable Enable or disable node tracking.
            \param nodeToTrack Node to track. Can be NULL if disabling.
            \param offset An offset from the node to track. This will become the lookat location.
        */
		void SetLookAtNode(bool enable, MashSceneNode *nodeToTrack = 0, const MashVector3 &offset = MashVector3(0.0f, 0.0f, 0.0f));

        //! Gets the unique runtime generated node id. 
		uint32 GetNodeID()const;

        //! Gets the number of children stored at this node. (Not including its childrens children).
		uint32 GetChildCount()const;

        //! Gets the children stored at this node. (Not including its childrens children). This list should not be modified.
		const MashList<MashSceneNode*>& GetChildren()const;

        //! Returns a child by unique node id.
		/*!
			Recursivly searches from this node and down into its children in
			search of a node with the given ID.
         
            \param uniqueId Node id to search for.
            \return Node with the given id. NULL if not found.
		*/
		MashSceneNode* GetChildByID(uint32 uniqueId);

        //! Returns a child by name.
		/*!
			Recursivly searches from this node and down into its children in
			search of a node with a given name.
         
            \param name Name to search for.
            \return The first node found with the given name. NULL if not found.
		*/
		virtual MashSceneNode* GetChildByName(const MashStringc &name);

        //! Returns a child by type.
        /*!
            Recursivly searches from this node and down into its children in
            search of a node with a given type.
         
            \param type Type to search for.
            \return The first node found with the given type. NULL if not found.
        */
		MashSceneNode* GetChildByType(eNODE_TYPE type);

        //! Gets the visibility state of this node.
        /*!
            This is accessed during culling to determine if this node should be rendered.
            Only this node is affected, not its children.
         
            \return Visibility state.
        */
		bool IsVisible()const;
        
        //! Sets the visibility state.
        /*!
            This is accessed during culling to determine if this node should be rendered.
            Only this node is affected, not its children.
         
            \param isVsible Visibility state.
        */
		void SetVisible(bool isVsible);

        //! Detaches this node from its parent. The parent will drop its reference.
        /*!
            To remove a node from the Scene manager use MashSceneManager::RemoveSceneNode().
        */
		void Detach();
        
        //! Detaches a child from this node. The childs reference will be dropped.
        /*!
            \param child Child to remove.
        */
		void DetachChild(MashSceneNode *child);
        
        //! Detaches all children from this node.
		void DetachAllChildren();

        //! Sets user data.
        /*!
            This data can be used for any purpose.
         
            \param data User data.
        */
		void SetUserData(void *data);
        
        //! Gets the user data.
        /*!
            \return User data.
        */
		void* GetUserData()const;

        //! Sets the user id.
        /*!
            This id can be used for any purpose.
         
            \param userId User generated id. 
        */
		void SetUserID(int32 userId);
        
        //! Gets the user id.
		int32 GetUserID()const;
        
        //! Sets the node node.
        /*!
            This name does not need to be unique.
         
            \param str Node name.
        */
		void SetNodeName(const MashStringc &str);
        
        //! Gets the node name.
        const MashStringc& GetNodeName()const;

        //! Updates this node and its children only if needed.
		/*!
            This is called by the scene manager when updating so the user shouldn't need to
            call this directly.
         
            Updates this node and its childrens absolute transforms and bounds only if needed.
		*/
		void Update();
        
        //! Updates the absolute transforms of this node only.
        /*!
            Called during an update when needed so it shouldn't need to be called directly.
        */
        void UpdateAbsoluteTransformation();

		//! Gets the parent node. NULL if no parent is set.
		MashSceneNode* GetParent()const;

		//! Called when this node passes culling.
		/*!
            This is normally called from a culling technique.
         
			Some scene nodes leave render only data to be updated when the node has passed culling.
            So it is important this is called when culling is passed. The interpolated position is
            also calculated here using interpolatedTime.
         
            \param interpolatedTime Interpolated frame time. This originates from MashGameLoop::Render().
            \param frameCount Current frame count from MashTimer::GetFrameCount().
		*/
		void OnCullPass();
        
        //! Adds any renderables contained in this node to the render queue.
        /*!
            This is normally called from a culling technique.
         
            This calls on MashSceneManager::AddRenderableToRenderQueue() to add renderables
            for scene rendering.
         
            \param state States whether this object should be considered as a shadow caster or simply
                a scene object.
            \param functPtr Performs final culling on this node. See MashCullTechnique for more info.
            \return True if a renderable was added to the render queue. False if nothing was added.
        */
		virtual bool AddRenderablesToRenderQueue(eRENDER_STAGE stage, MashCullTechnique::CullRenderableFunctPtr functPtr){return false;}

        //! Gets the scene node type.
		virtual uint32 GetNodeType()const = 0;

		//! Returns true if this node contains renderables.
		virtual bool ContainsRenderables()const{return false;}
        
        //! Sets the animation buffer.
        /*!
            This method grabs a copy of the buffer and drops any previously set.
         
            Animation buffers hold animation keys for this node. They can be added
            to animation mixers to animate one or more nodes.
         
            Buffers can be created from the controller manager.
         
         \param buffer Animation buffer to set.
         */
		void SetAnimationBuffer(MashAnimationBuffer *buffer);
        
        //! Gets the animation buffer.
        /*!
            \param Animation buffer. NULL if nothing is set.
         */
		MashAnimationBuffer* GetAnimationBuffer()const;

        //! Gets this nodes animation mixer.
		virtual MashAnimationMixer* GetAnimationMixer()const;
        
        //! Sets this nodes animation mixer.
        /*!
            Animation mixers control the animation for one or many nodes.
            You would normally add all nodes to a mixer that are related. For example with skinned 
            animations, normally the mixer would contain all the bones that affect an entity. The node
            that would own the mixer maybe the entity, or the parent of both the entity and bones.
         
            \param mixer Mixer to set to this node.
        */
		virtual void SetAnimationMixer(MashAnimationMixer *mixer);

        //! Drops the animation mixer from this node. Doesn't delete it.
		virtual void RemoveAnimationMixer();

		//! Drops the animation buffer from this node. Doesn't delete it.
		virtual void RemoveAnimationBuffer();

        //! Set this node to only inherit translation from its parent. Not rotation or scale.
        /*!
            This is handy for displaying icons or names above nodes without having them rotate
            with its parent.
         
            \param state Inherit translation only state.
        */
		void SetInheritTranslationOnly(bool state);
        
        //! Gets the inherite translation only state.
		bool GetInheritTranslationOnly()const;

		/*!
			Specialty functions only. Used in cases when a node needs to be inserted
			into a specific, valid, location within the parents child list. If the location
			isn't valid then the child will simply be added to the end of the list
		*/
		uint32 GetIndexWithinParent()const;
		uint32 GetChildIndex(const MashSceneNode *child)const;
        
        //! Adds a child to particular location within the child list.
        /*!
            Specialty functions only. Consider using SetParent instead.
            
            Used in cases when a node needs to be inserted into a specific, valid, location within 
            the parents child list. If the location isn't valid then the child will simply be added 
            to the end of the list.
         
            \param child Child to add.
            \param location Location within the child list.
            \return Failed if the child is adding itself or is already added.
         */
		eMASH_STATUS AddChildAtLocation(MashSceneNode *child, uint32 location);
        
        //! Updates the world transform if neeeded and returns it.
		/*!
			GetWorldTransformState() returns the state directly, updated or not.

			This function is a bit slower because it recurses to the top of the
			graph then updates its node if needed.
		*/
        const MashTransformState& GetUpdatedWorldTransformState();
        
        //! Updates any callback each frame. Called before MashGameLoop::Update().
        void _UpdateCallbacks(f32 dt);
        
        //! Updates this node to lookat its target.
        /*!
            Called internally after the scene has updated.
        */
        void _LookAt();
	};
    
    inline bool MashSceneNode::IsUpdateNeeded()const
    {
        return (m_updateFlags != 0);
    }
    
    inline uint32& MashSceneNode::GetUpdateFlags()
    {
        return m_updateFlags;
    }

	inline uint32 MashSceneNode::GetSnapToPositionFlags()const
	{
		return m_snapToPositionFlags;
	}

	inline void MashSceneNode::SetInheritTranslationOnly(bool state)
	{
		m_inheritTranslationOnly = state;
	}

	inline bool MashSceneNode::GetInheritTranslationOnly()const
	{
		return m_inheritTranslationOnly;
	}

	inline const MashTransformState& MashSceneNode::GetLocalTransformState()const
	{
		return m_relativeTransformState;
	}

	inline const MashTransformState& MashSceneNode::GetWorldTransformState()const
	{
		return m_absoluteTransformEndState;
	}

	inline MashAnimationBuffer* MashSceneNode::GetAnimationBuffer()const
	{
		return m_animationBuffer;
	}

	inline const MashAABB& MashSceneNode::GetWorldBoundingBox()const
	{
		return m_absoluteBoundingBox;
	}

	inline const MashList<MashSceneNode*>& MashSceneNode::GetChildren()const
	{
		return m_children;
	}

	inline MashSceneNode* MashSceneNode::GetParent()const
	{
		return m_parent;
	}

	inline const MashTriangleCollider* MashSceneNode::GetTriangleCollider()const
	{
		return 0;
	}

	inline const MashAABB& MashSceneNode::GetTotalBoundingBox()const
	{
		return m_totalAABB;
	}

	inline bool MashSceneNode::IsVisible()const
	{
		return m_isVisible;
	}

	inline uint32 MashSceneNode::GetChildCount()const
	{
		return m_children.Size();
	}

	inline uint32 MashSceneNode::GetNodeID()const
	{
		return m_internalNodeID;
	}

	inline void MashSceneNode::SetUserID(int32 id)
	{
		m_userID = id;
	}

	inline int32 MashSceneNode::GetUserID()const
	{
		return m_userID;
	}

	inline const MashStringc& MashSceneNode::GetNodeName()const
	{
		return m_nodeName;
	}

	inline void MashSceneNode::SetUserData(void *data)
	{
		m_userData = data;
	}

	inline void* MashSceneNode::GetUserData()const
	{
		return m_userData;
	}
}

#endif