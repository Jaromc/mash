//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "MashSceneNode.h"
#include "MashSceneManager.h"
#include "MashAnimationBuffer.h"
#include "MashAnimationMixer.h"
#include "MashTriangleCollider.h"
#include "MashHelper.h"
#include "MashMathHelper.h"
#include "MashDevice.h"
#include "MashTimer.h"
#include "MashLog.h"

namespace mash
{
	uint32 MashSceneNode::m_nodeCounter = 0;

	MashSceneNode::MashSceneNode(MashSceneNode *pParent,
			MashSceneManager *pManager,
			const MashStringc &sName,
			const mash::MashVector3 &vPosition,
			const mash::MashQuaternion &qOrientation,
			const mash::MashVector3 &vScale):
				m_relativeTransformState(vPosition, vScale, qOrientation),
				MashReferenceCounter(),
				m_parent(0),
				m_sceneManager(pManager),
				m_userData(0),
				m_nodeName(sName),
				m_userID(0),
				m_internalNodeID(m_nodeCounter++),
				m_animationMixer(0),
                m_isVisible(true),
				m_animationBuffer(0),
				m_lookatNode(0),
				m_inheritTranslationOnly(false),
                m_updateFlags(aUPDATE_FLAG_TRANSFORM),
                m_addedToParentUpdate(false),
                m_interpolationTime(0.0f),
                m_renderTransformUpdateNeeded(true),
                m_renderTransformMatrixUpdateNeeded(true),
                m_lastCullFrame(-1),
				m_lastTransformUpdateRenderFrame(-1),
				m_lastTransformUpdateFrame(-1),
				m_snapToPositionFlags(aNODE_SNAP_ALL)
	{
        m_timer = MashDevice::StaticDevice->GetTimer();

		if (pParent)
		{
			pParent->AddChild(this);
		}

		UpdateAbsoluteTransformation();
		//keep the snap flag enabled for the first frame
		m_snapToPositionFlags = aNODE_SNAP_ALL;
	}

	MashSceneNode::MashSceneNode(const MashSceneNode *pCopy, const int8 *sName):
		MashReferenceCounter(),
		m_internalNodeID(m_nodeCounter++)
	{
		m_nodeName = sName;
	}

	MashSceneNode::~MashSceneNode()
	{
        if (!m_nodeCallbacks.Empty())
        {
            MashArray<sCallback>::Iterator iter = m_nodeCallbacks.Begin();
            MashArray<sCallback>::Iterator end = m_nodeCallbacks.End();
            for(; iter != end; ++iter)
            {
                iter->callback->OnNodeDetach(this);
                iter->callback->Drop();
            }
            
            m_sceneManager->_RemoveCallbackNode(this);
        }

		m_nodeCallbacks.Clear();

		Detach();
		DetachAllChildren();

		SetLookAtNode(false);

		if (m_animationBuffer)
		{
			m_animationBuffer->Drop();
			m_animationBuffer = 0;
		}

		if (m_animationMixer)
		{
			m_animationMixer->Drop();
			m_animationMixer = 0;
		}
	}

	void MashSceneNode::InstanceMembers(MashSceneNode *from)
	{
		m_sceneManager = from->m_sceneManager;

		m_relativeTransformState = from->m_relativeTransformState;
		m_absoluteTransformStartState = from->m_absoluteTransformStartState;
		m_absoluteTransformEndState = from->m_absoluteTransformEndState;
		m_interpolatedTransformState = from->m_interpolatedTransformState;
		m_interpolatedTransformation = from->m_interpolatedTransformation;

		m_totalAABB = from->m_totalAABB;
		m_absoluteBoundingBox = from->m_absoluteBoundingBox;
		m_userID = from->m_userID;
		m_userData = from->m_userData;
		m_isVisible = from->m_isVisible;
		m_animationBuffer = from->m_animationBuffer;

		if (m_animationBuffer)
			m_animationBuffer->Grab();

		//clone all children
		MashList<MashSceneNode*>::Iterator iter = from->m_children.Begin();
		MashList<MashSceneNode*>::Iterator end = from->m_children.End();
		for(; iter != end; ++iter)
			(*iter)->_CreateInstance(this, (*iter)->GetNodeName());

		MashArray<sCallback>::Iterator callbackIter = from->m_nodeCallbacks.Begin();
		MashArray<sCallback>::Iterator callbackIterEnd = from->m_nodeCallbacks.End();
		for(; callbackIter != callbackIterEnd; ++callbackIter)
		{
			m_nodeCallbacks.PushBack(*callbackIter);
			callbackIter->callback->Grab();
		}
	}

	MashAnimationMixer* MashSceneNode::GetAnimationMixer()const
	{
		return m_animationMixer;
	}

	void MashSceneNode::RemoveAnimationMixer()
	{
		if (m_animationMixer)
		{
			m_animationMixer->Drop();
			m_animationMixer = 0;
		}
	}

	void MashSceneNode::RemoveAnimationBuffer()
	{
		if (m_animationBuffer)
		{
			m_animationBuffer->Drop();
			m_animationBuffer = 0;
		}
	}

	uint32 MashSceneNode::GetIndexWithinParent()const
	{
		if (m_parent)
			return m_parent->GetChildIndex(this);

		return 0;
	}

	uint32 MashSceneNode::GetChildIndex(const MashSceneNode *child)const
	{
		MashList<MashSceneNode*>::ConstIterator childLocation = m_children.Begin();
		MashList<MashSceneNode*>::ConstIterator childLocationEnd = m_children.End();
		uint32 count = 0;
		for(; childLocation != childLocationEnd; ++childLocation, ++count)
		{
			if (*childLocation == child)
				return count;
		}

		return count;
	}

	eMASH_STATUS MashSceneNode::AddChildAtLocation(MashSceneNode *child, uint32 location)
	{
		if (!child)
			return aMASH_FAILED;

		eMASH_STATUS status = AddChild(child);

		MashList<MashSceneNode*>::Iterator childLocation = m_children.Begin();
		MashList<MashSceneNode*>::Iterator childLocationEnd = m_children.End();
		for(; childLocation != childLocationEnd; ++childLocation)
		{
			if (*childLocation == child)
			{
				break;
			}
		}

		if (childLocation == m_children.End())
			return status;

		MashList<MashSceneNode*>::Iterator iter = m_children.Begin();
		MashList<MashSceneNode*>::Iterator iterEnd = m_children.End();
		uint32 count = 0;
		for(; iter != iterEnd; ++iter, ++count)
		{
			if (count == location)
			{
				//make sure we are not erasing the same location
				if (childLocation != iter)
				{
					m_children.Erase(childLocation);
					m_children.Insert(iter, child);
				}
				break;
			}
		}

		return status;
	}

	void MashSceneNode::SetAnimationBuffer(MashAnimationBuffer *buffer)
	{
		if (buffer)
			buffer->Grab();

		if (m_animationBuffer)
			m_animationBuffer->Drop();

		m_animationBuffer = buffer;
	}

	void MashSceneNode::SetAnimationMixer(MashAnimationMixer *mixer)
	{			
		if (mixer)
			mixer->Grab();

		if (m_animationMixer)
			m_animationMixer->Grab();

		m_animationMixer = mixer;

		//Snap to the new position then interpolate after
		m_snapToPositionFlags = aNODE_SNAP_ALL;
	}

	void MashSceneNode::AddCallback(MashSceneNodeCallback *pCallback, uint32 order)
	{
		if (!pCallback)
			return;

        if (m_nodeCallbacks.Empty())//add it on the first occasion only
            m_sceneManager->_AddCallbackNode(this);

		m_nodeCallbacks.PushBack(sCallback(pCallback, order));
		m_nodeCallbacks.Sort();
        
        pCallback->Grab();
        
		pCallback->OnNodeAttach(this);
	}

	void MashSceneNode::RemoveCallback(MashSceneNodeCallback *pCallback)
	{
		MashArray<sCallback>::Iterator iter = m_nodeCallbacks.Begin();
		MashArray<sCallback>::Iterator end = m_nodeCallbacks.End();
		for(; iter != end; ++iter)
		{
			if (iter->callback == pCallback)
			{
				pCallback->OnNodeDetach(this);
				m_nodeCallbacks.Erase(iter);
				pCallback->Drop();

				break;
			}
		}
        
        //remove it from the manager when no more callbacks exist
        if (m_nodeCallbacks.Empty())
        {
            m_sceneManager->_RemoveCallbackNode(this);
        }
	}

	void MashSceneNode::RecalculateTotalBoundingBox()
	{
		m_totalAABB.SetLimits(mash::MashVector3(mash::math::MaxFloat(), mash::math::MaxFloat(), mash::math::MaxFloat()),
			mash::MashVector3(mash::math::MinFloat(), mash::math::MinFloat(), mash::math::MinFloat()));

		m_totalAABB.Merge(m_absoluteBoundingBox);

		MashList<MashSceneNode*>::Iterator iter =  m_children.Begin();
		MashList<MashSceneNode*>::Iterator end =  m_children.End();
		for(; iter != end; iter++)
		{
			m_totalAABB.Merge((*iter)->GetTotalBoundingBox());
		}
	}

	MashSceneNode* MashSceneNode::GetChildByID(uint32 iID)
	{
		if (GetNodeID() == iID)
			return this;

		MashList<MashSceneNode*>::Iterator iter =  m_children.Begin();
		MashList<MashSceneNode*>::Iterator end =  m_children.End();
		for(; iter != end; iter++)
		{
			MashSceneNode *pNode = (*iter)->GetChildByID(iID);

			if (pNode)
				return pNode;
		}

		return 0;
	}

	MashSceneNode* MashSceneNode::GetChildByName(const MashStringc &name)
	{
		if (GetNodeName() == name)
			return this;

		MashList<MashSceneNode*>::Iterator iter =  m_children.Begin();
		MashList<MashSceneNode*>::Iterator end =  m_children.End();
		for(; iter != end; iter++)
		{
			MashSceneNode *pNode = (*iter)->GetChildByName(name);

			if (pNode)
				return pNode;
		}

		return 0;
	}

	MashSceneNode* MashSceneNode::GetChildByType(eNODE_TYPE type)
	{
		if (GetNodeType() == type)
			return this;

		MashList<MashSceneNode*>::Iterator iter =  m_children.Begin();
		MashList<MashSceneNode*>::Iterator end =  m_children.End();
		for(; iter != end; iter++)
		{
			MashSceneNode *pNode = (*iter)->GetChildByType(type);

			if (pNode)
				return pNode;
		}

		return 0;
	}

	void MashSceneNode::Detach()
	{
		if (m_parent)
			m_parent->DetachChild(this);
	}

	void MashSceneNode::DetachChild(MashSceneNode *pChild)
	{
		MashList<MashSceneNode*>::Iterator iter =  m_children.Begin();
		MashList<MashSceneNode*>::Iterator end =  m_children.End();
		for(; iter != end; iter++)
		{
			if ((*iter) == pChild)
			{
				pChild->m_parent = 0;
				m_children.Erase(iter);

				//need to update so the totalBB is correct
				pChild->WorldTransformUpdateNeeded();
				WorldTransformUpdateNeeded();

				pChild->Drop();
				break;
			}
		}
	}

	void MashSceneNode::DetachAllChildren()
	{
		MashList<MashSceneNode*>::Iterator iter =  m_children.Begin();
		MashList<MashSceneNode*>::Iterator end =  m_children.End();
		for(; iter != end; iter++)
		{
			(*iter)->m_parent = 0;

			//need to update so the totalBB is correct
			(*iter)->WorldTransformUpdateNeeded();

			(*iter)->Drop();
		}
		
		//need to update so the totalBB is correct
		WorldTransformUpdateNeeded();

		m_children.Clear();
	}

    bool MashSceneNode::SetParent(mash::MashSceneNode *pParent)
	{
		//cant parent outselves
		if (pParent == this)
			return false;

		if (m_parent == pParent)
			return false;

		if (m_parent)
		{
			m_parent->DetachChild(this);
			m_parent = 0;
		}

		m_parent = pParent;

        WorldTransformUpdateNeeded();
		return true;
	}

	eMASH_STATUS MashSceneNode::AddChild(MashSceneNode *pChild)
	{
		//make sure we are not adding ourself
		if (this == pChild)
		{
			MASH_WRITE_TO_LOG_EX(MashLog::aERROR_LEVEL_ERROR, 
						"MashSceneNode::AddChild",
						"Nodes cannot parent themselves : %s",
						pChild->GetNodeName().GetCString());

			return aMASH_FAILED;
		}

		if (pChild && pChild->SetParent(this))
		{
			m_children.PushBack(pChild);
			pChild->Grab();
			
			pChild->WorldTransformUpdateNeeded();
			return aMASH_OK;
		}

		return aMASH_FAILED;
	}

	void MashSceneNode::_LookAt()
	{
		if (!m_lookatNode)
			return;

		SetLookAtDirection(((m_lookatNode->GetWorldTransformState().translation + m_lookatOffset) - GetWorldTransformState().translation).Normalize(), false);
        Update();
	}

	void MashSceneNode::SetLookAtNode(bool enable, MashSceneNode *nodeToTrack, const MashVector3 &offset)
	{
		if (enable && nodeToTrack)
		{
			nodeToTrack->Grab();

			if (m_lookatNode)
				m_lookatNode->Drop();

			if (!m_lookatNode)
				m_sceneManager->_AddLookAtTracker(this);

			m_lookatNode = nodeToTrack;
			m_lookatOffset = offset;	
		}
		else if (!enable)
		{
			if (m_lookatNode)
			{
				m_sceneManager->_RemoveLookAtTracker(this);
				m_lookatNode->Drop();
				m_lookatNode = 0;
			}
		}	
	}

	void MashSceneNode::SetLookAtDirection(const mash::MashVector3 &direction, bool snapToPosition)
	{
		mash::MashQuaternion amountToRotate;
		amountToRotate.RotateTo(mash::MashVector3(0.0f, 0.0f, 1.0f), direction);
		SetOrientation(amountToRotate, snapToPosition);
	}

	void MashSceneNode::SetTransformation(const mash::MashMatrix4 &mTransformation, bool snapToPosition)
	{
		MashVector3 scale, position;
		MashQuaternion rotation;
		mTransformation.Decompose(scale, rotation, position);

		SetScale(scale, snapToPosition);
		SetOrientation(rotation, snapToPosition);
		SetPosition(position, snapToPosition);
	}

	void MashSceneNode::UpdateAbsoluteTransformation()
	{
		const uint32 currentFrameCount = m_timer->GetFrameCount();

		/*
			If a render frame has passed then we set this nodes starting position
			to last frames end position. The transform may be updated more than
			once per frame in the case of lookat nodes whos final position will need
			to be updated after the scene is updated.
		*/
		if (m_lastTransformUpdateRenderFrame != currentFrameCount)
		{
			m_absoluteTransformStartState = m_absoluteTransformEndState;
			m_lastTransformUpdateRenderFrame = currentFrameCount;
		}

		//TODO : Only do this when the orientation has changed
		 m_relativeTransformState.orientation.Normalize();

		if (m_parent)
		{
			const MashTransformState *parentState = &m_parent->GetWorldTransformState();

			if (m_inheritTranslationOnly)
			{
				m_absoluteTransformEndState.orientation = m_relativeTransformState.orientation;
				m_absoluteTransformEndState.scale = m_relativeTransformState.scale;
				m_absoluteTransformEndState.translation = parentState->translation + m_relativeTransformState.translation;
			}
			else
			{
				m_absoluteTransformEndState.orientation = parentState->orientation * m_relativeTransformState.orientation;
				m_absoluteTransformEndState.scale = parentState->scale * m_relativeTransformState.scale;
				m_absoluteTransformEndState.translation = parentState->orientation.TransformVector(parentState->scale * m_relativeTransformState.translation);
				m_absoluteTransformEndState.translation += parentState->translation;
			}			
		}
		else
		{
			m_absoluteTransformEndState = m_relativeTransformState;
		}

        //Child needs to snap if the parent is snaping
		if (m_parent)//inherit any parent updates
			m_snapToPositionFlags |= m_parent->GetSnapToPositionFlags();
		
		/*
			snap if this has not been rendered yet. We check for a render because the node
			should be in its starting position (and initialized) at this point.
		*/
		if (m_lastCullFrame == -1)
			m_snapToPositionFlags |= aNODE_SNAP_ALL;

        if (m_snapToPositionFlags)
        {
			if (m_snapToPositionFlags & aNODE_SNAP_TRANSLATION)
			{
				m_absoluteTransformStartState.translation = m_absoluteTransformEndState.translation;
			}

			if (m_snapToPositionFlags & aNODE_SNAP_ROTATION)
			{
				m_absoluteTransformStartState.orientation = m_absoluteTransformEndState.orientation;
			}

			if (m_snapToPositionFlags & aNODE_SNAP_SCALE)
			{
				m_absoluteTransformStartState.scale = m_absoluteTransformEndState.scale;
			}

			/*
				Note, the snap flag is reset in the update so that children know the state
				of the parent
			*/
        }
        
        // Starts interpolating the position forward
        m_interpolationTime = 0.0f;
        m_renderTransformUpdateNeeded = true;
        m_renderTransformMatrixUpdateNeeded = true;
		m_lastTransformUpdateFrame = m_timer->GetUpdateCount();
	}

	void MashSceneNode::SetVisible(bool bIsVisible)
	{
		m_isVisible = bIsVisible;
	}

	void MashSceneNode::AddOrientation(const mash::MashQuaternion &orientation, bool snapToPosition)
	{
		if (orientation != MashQuaternion(1.0f, 0.0f, 0.0f, 0.0f))
		{
			//m_qRelativeOrientation *= orientation;
			m_relativeTransformState.orientation *= orientation;
			//m_needsUpdate = true;
			WorldTransformUpdateNeeded();

			if (snapToPosition)
				m_snapToPositionFlags |= aNODE_SNAP_ROTATION;
		}
	}
    
    void MashSceneNode::_UpdateFromParent()
    {
        if (m_parent)
            m_parent->_UpdateFromParent();
        else
        {
            if (IsUpdateNeeded())
                _Update(false);
        }
    }
    
    void MashSceneNode::ChildUpdateNeeded(MashSceneNode *child)
    {
        GetUpdateFlags() |= aUPDATE_FLAG_CHILD_TRANSFORM;
        m_childrenToUpdate.PushBack(child);
        
        if (m_parent && !m_addedToParentUpdate)
        {
            m_parent->ChildUpdateNeeded(this);
            m_addedToParentUpdate = true;
        }
    }
    
    void MashSceneNode::WorldTransformUpdateNeeded()
    {        
        if (m_parent && !m_addedToParentUpdate)
        {
            m_parent->ChildUpdateNeeded(this);
            m_addedToParentUpdate = true;
        }
        
        GetUpdateFlags() |= aUPDATE_FLAG_TRANSFORM;
    }
    
    const mash::MashTransformState& MashSceneNode::GetUpdatedWorldTransformState()
    {
        _UpdateFromParent();
        
        return GetWorldTransformState();
    }
    
    void MashSceneNode::Update()
	{
        _UpdateFromParent();
    }
    
    void MashSceneNode::_UpdateCallbacks(f32 dt)
    {
        if (!m_nodeCallbacks.Empty())
        {
            //callbacks may animate this node in some way.
            //These are called first as they might affect the update status of the node.
            MashArray<sCallback>::Iterator iter = m_nodeCallbacks.Begin();
            MashArray<sCallback>::Iterator end = m_nodeCallbacks.End();
            for(; iter != end; ++iter)
            {
                iter->callback->OnNodeUpdate(this, dt);
            }
        }
    }
    
    void MashSceneNode::_Update(bool bParentHasChanged)
	{
        bool needsTransformUpdate = (GetUpdateFlags() & aUPDATE_FLAG_TRANSFORM) || bParentHasChanged/* || m_targetNode*/;
        
		if (needsTransformUpdate)
		{
			UpdateAbsoluteTransformation();
            
			//update world bounds
			m_absoluteBoundingBox = GetLocalBoundingBox();
			m_absoluteBoundingBox.Transform(m_absoluteTransformEndState);
            
            OnNodeTransformChange();
            
            //update all children
            MashList<MashSceneNode*>::Iterator children = m_children.Begin();
            MashList<MashSceneNode*>::Iterator childrenEnd = m_children.End();
            for(; children != childrenEnd; children++)
            {
                (*children)->_Update(true);
            }
		}
        else
        {
            MashArray<MashSceneNode*>::Iterator childIter = m_childrenToUpdate.Begin();
            MashArray<MashSceneNode*>::Iterator childIterEnd = m_childrenToUpdate.End();
            for(; childIter != childIterEnd; ++childIter)
            {
                (*childIter)->_Update(false);
            }
        }

		//reset the snap flag here so that children know the parent state
		m_snapToPositionFlags = 0;
        
        //No positional changes should happen after this point!
        m_addedToParentUpdate = false;
        
        m_childrenToUpdate.Clear();
        
		/*
         This needs to be done after all the children have updated
         so we get the correct bounding box
         */
		if (needsTransformUpdate || (GetUpdateFlags() & aUPDATE_FLAG_CHILD_TRANSFORM))
			RecalculateTotalBoundingBox();
        
        m_updateFlags = 0;
    }
    
    void MashSceneNode::SetPosition(const mash::MashVector3 &vPosition, bool snapToPosition)
	{
		if (vPosition != m_relativeTransformState.translation)
		{
			m_relativeTransformState.translation = vPosition;
			WorldTransformUpdateNeeded();
	        
			if (snapToPosition)
				m_snapToPositionFlags |= aNODE_SNAP_TRANSLATION;
		}
	}
    
	void MashSceneNode::SetOrientation(const mash::MashQuaternion &qOrientation, bool snapToPosition)
	{
		if (qOrientation != m_relativeTransformState.orientation)
		{
			m_relativeTransformState.orientation = qOrientation;
			WorldTransformUpdateNeeded();
	        
			if (snapToPosition)
				m_snapToPositionFlags |= aNODE_SNAP_ROTATION;
		}
	}
    
	void MashSceneNode::SetScale(const mash::MashVector3 &vScale, bool snapToPosition)
	{
		if (vScale != m_relativeTransformState.scale)
		{
			m_relativeTransformState.scale = vScale;
			WorldTransformUpdateNeeded();
	        
			if (snapToPosition)
				m_snapToPositionFlags |= aNODE_SNAP_SCALE;
		}
	}
    
	void MashSceneNode::AddPosition(const mash::MashVector3 &vPosition, bool snapToPosition)
	{
		if (vPosition != MashVector3(0.0f, 0.0f, 0.0f))
		{
			m_relativeTransformState.translation += vPosition;
			WorldTransformUpdateNeeded();
	        
			if (snapToPosition)
				m_snapToPositionFlags |= aNODE_SNAP_TRANSLATION;
		}
	}
    
	void MashSceneNode::AddScale(const mash::MashVector3 &vScale, bool snapToPosition)
	{
		if (vScale != MashVector3(0.0f, 0.0f, 0.0f))
		{
			m_relativeTransformState.scale += vScale;
			WorldTransformUpdateNeeded();
	        
			if (snapToPosition)
				m_snapToPositionFlags |= aNODE_SNAP_SCALE;
		}
	}

	void MashSceneNode::SetNodeName(const MashStringc &str)
	{
        m_nodeName = str;
	}
    
    const mash::MashTransformState& MashSceneNode::_GetRenderTransformState()const
	{
        if (m_renderTransformUpdateNeeded)
        {            
            if (m_interpolatedTransformState.translation != m_absoluteTransformEndState.translation)
                m_interpolatedTransformState.translation = m_absoluteTransformStartState.translation.Lerp(m_absoluteTransformEndState.translation, m_interpolationTime);
            
            if (m_interpolatedTransformState.scale != m_absoluteTransformEndState.scale)
                m_interpolatedTransformState.scale = m_absoluteTransformStartState.scale.Lerp(m_absoluteTransformEndState.scale, m_interpolationTime);
            
            if (m_interpolatedTransformState.orientation != m_absoluteTransformEndState.orientation)
                m_interpolatedTransformState.orientation.Slerp(m_absoluteTransformStartState.orientation, m_absoluteTransformEndState.orientation, m_interpolationTime);
            
            m_renderTransformUpdateNeeded = false;
        }
        
		return m_interpolatedTransformState;
	}
    
    const mash::MashTransformState& MashSceneNode::GetRenderTransformState()const
	{
        PrepareForRenderTransformUpdate();
        return _GetRenderTransformState();
	}
    
    const mash::MashMatrix4& MashSceneNode::GetRenderTransformation()const
	{   
        PrepareForRenderTransformUpdate();
        
        if (m_renderTransformMatrixUpdateNeeded)
        {
           m_interpolatedTransformation = _GetRenderTransformState().ToMatrix();
           m_renderTransformMatrixUpdateNeeded = false;
        }
        
        return m_interpolatedTransformation;
	}
    
    void MashSceneNode::PrepareForRenderTransformUpdate()const
    {
       // {
            // Only enter if the frame has changed or we havent yet passed culling.
            /*
             Note a node may be culled twice, once for the scene and once for shadows.
             If a node passes any one of those culling techniques then it needs
             to interpolate its position.
             */
            f32 lastInterpTime = m_interpolationTime;
            
            /*
             The interpolation time will interpolate from 0.0 - 1.0 and rendering
             may happen many times before the next update. 
             
             This will stop the interpolation time from cycling from 0.0 - 1.0 and back
             again after each update which causes a jitter. This makes sure interpolation
             only moves forward towards 1 and stays there.
             
             Note, we add plus 1 because the update counter will increment by the
             time the following render is reached. So the matching render frame
             for a transform update that just occured is (m_lastTransformUpdateFrame + 1).
             */
			if ((m_lastTransformUpdateFrame + 1) == m_timer->GetUpdateCount())
				m_interpolationTime = m_timer->GetFrameInterpolatorTime();
			else
				m_interpolationTime = 1.0f;
            
            /*
             If the interpolation time has changed then we need to update the
             interpolation transforms. Note when transforms are updated then
             these bools are also set to true.
             */
            if (!math::FloatEqualTo(lastInterpTime, m_interpolationTime, 0.00001f))
			{
				m_renderTransformUpdateNeeded = true;
				m_renderTransformMatrixUpdateNeeded = true;
			}
        //}
    }

	void MashSceneNode::OnCullPass()
	{
        uint32 frameCount = m_timer->GetFrameCount();
        
        if (m_lastCullFrame != frameCount)
        {
            /*
                Note this is done when the transform is fetched, so we dont
                need to do it here as long as the transform is ALWAYS accessed
                via the GET function.
            */
			OnPassCullImpl(m_timer->GetFrameInterpolatorTime());
            m_lastCullFrame = frameCount;
        }
	}
}