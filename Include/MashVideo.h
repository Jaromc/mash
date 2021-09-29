//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_RENDERER_H_
#define _MASH_RENDERER_H_

#include "MashCreationParameters.h"
#include "MashReferenceCounter.h"
#include "MashGeometryBatch.h"
#include "MashDirectionalShadowCascadeCaster.h"
#include "MashSpotShadowCaster.h"
#include "MashPointShadowCaster.h"
#include "MashTypes.h"

namespace mash
{
	class MashRenderInfo;
	class MashMaterial;
	class MashMaterialManager;
	class MashTexture;
	class MashVertex;
	class MashVertexBuffer;
	class MashIndexBuffer;
	class MashMeshBuffer;

	class MashVector2;
	class MashRectangle2;

	class MashMesh;
	class MashSceneManager;
		
    class MashDevice;
	class MashRenderSurface;
	class MashMaterialDependentResourceBase;

    /*!
        This is the base class for OpenGL and DirectX renderers.
     
        From here you can create textures, render buffers, and render targets.
     
        Buffers can be manually drawn to the current render target using any of
        the Drawxxx() functions.
    */
	class MashVideo : public MashReferenceCounter
	{
	public:
		MashVideo():MashReferenceCounter(){}
		virtual ~MashVideo(){}

        //! Gets the current renderer type.
		virtual eSHADER_API_TYPE GetCurrentAPI()const = 0;

        //! Saves a screen shot.
        /*!
            \param outputFormat Output file format.
            \param file File and path to save to.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS SaveScreenShotToFile(eSAVE_TEXTURE_FORMAT outputFormat, const MashStringc &file)const = 0;
        
        //! Saves a texture to file.
        /*!
            \param texture Texture to save.
            \param outputFormat Output file format.
            \param file File and path to save to.
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS SaveTextureToFile(const MashTexture *texture, eSAVE_TEXTURE_FORMAT outputFormat, const MashStringc &file)const = 0;

        //! Sets the active rasterizer state.
        /*!
            \param rasterizerIndex Index returned from AddRasteriserState().
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS SetRasteriserState(int32 rasterizerIndex) = 0;
        
        //! Sets the active blend state.
        /*!
            \param blendIndex Index returned from AddBlendState().
            \return Ok on success, failed otherwise.
        */
		virtual eMASH_STATUS SetBlendState(int32 blendIndex) = 0;
        
        //! Creates a rasteriser state.
        /*!
            If a state of the same description already exists then its index will be returned.
         
            Rasterisers states are used when objects are rendered to the screen.
            These are automatically set by a material when they are set for rendering.
         
            \param state Rasteriser state to create.
            \return Rasteriser state handle.
        */
		virtual int32 AddRasteriserState(const sRasteriserStates &state) = 0;
        
        //! Creates a blend state.
        /*!
            If a state of the same description already exists then its index will be returned.
         
            Blend states are used when objects are rendered to the screen.
            These are automatically set by a material when they are set for rendering.
         
            \param state Blend state to create.
            \return Blend state handle.
        */
		virtual int32 AddBlendState(const sBlendStates &state) = 0;
        
        //! Creates a sampler state for texture rendering.
        /*!
            If a state of the same description already exists then its index will be returned.
         
            These can be set to a texture via MashMaterial or MashTechniqueInstance. Or set
            manually in MashEffect.
         
            Sampler states describe how the texture will be read on the GPU.
         
            \param state Texture state to create.
            \return Texture state.
        */
		virtual MashTextureState* AddSamplerState(const sSamplerState &state) = 0;

        //! Gets a blend state description.
        /*!
            \param index Blend state index.
            \return Blend state description.
        */
		virtual const sBlendStates* GetBlendState(int32 index)const = 0;
        
        //! Gets a rasterizer state description.
        /*!
            \param index Rasterizer state index.
            \return Rasterizer state description.
        */
		virtual const sRasteriserStates* GetRasterizerState(int32 iIndex)const = 0;
        
        //! Locks the rasterizer from changing states.
        /*!
            Stops any attempts to change the rasterizer state until it's unlocked.
         
            \param enable Lock state.
        */
        virtual void LockRasterizerState(bool enable) = 0;
        
        //! Locks the blend states from changing.
        /*!
            Stops any attempts to change the blend state until it's unlocked.
         
            \param enable Lock state.
         */
		virtual void LockBlendState(bool enable) = 0;

        //! Sets the current viewport for rendering.
        /*!
            This can be called before a Drawxxx() to change the render area from the whole
            screen, to a smaller portion of the screen, and back again. This can be used
            for features such as minimaps or simply rendering a texture to a particular
            portion of the screen.
         
            \param viewport Viewport to set.
            \return Ok on success, failed if any errors occured.
        */
        virtual eMASH_STATUS SetViewport(const sMashViewPort &viewport) = 0;
        
        //! Gets the active viewport.
		virtual const sMashViewPort& GetViewport()const = 0;
        
        //! Called internally to set the active vertex declaration for rendering.
        /*!
            This is commonly called from mesh buffers before being rendered.
            Users do not need to call this.
         
            \param vertex Vertex type to set for rendering.
            \return Ok on success, failed if any errors occured.
         */
		virtual eMASH_STATUS SetVertexFormat(MashVertex *vertex) = 0;
        
        //! Gets the current backbuffer size.
		/*!
            This is not the window size.
         
			Returns the default back buffer size. If returnActiveRenderSurfaceSize is true
			then the active render surface (if set) is returned.
         
            \param returnActiveRenderSurfaceSize Return default or active render target size.
            \return Backbuffer size.
		*/
		virtual const mash::MashVector2 GetBackBufferSize(bool returnActiveRenderSurfaceSize = false)const = 0;
		
        //! Called internally before MashGameLoop::Render() to begin rendering.
		virtual eMASH_STATUS BeginRender() = 0;
        
        //! Called internally after MashGameLoop::Render() to end rendering.
		virtual eMASH_STATUS EndRender() = 0;
        
        //! Sets the default clear buffer colour.
        /*!
            \param fillColour Default clear colour.
        */
        virtual void SetFillColour(const sMashColour4 &fillColour) = 0;

		//! Gets the default clear buffer colour.
		/*!
			\return Fill colour.
		*/
		virtual const sMashColour4& GetFillColour()const = 0;

        //! Called to clear the current target.
        /*!
            This is automatically called once at the start of rendering.
            This can be called many times during MashGameLoop::Render() as needed.
         
            \param clearFlags Bitwise eCLEAR_FLAG.
            \param colour Clear colour.
            \param ZDepth Clear depth buffer.
            \return Ok on success, failed if any errors occured.
        */
		virtual eMASH_STATUS ClearTarget(uint32 clearFlags, const sMashColour4 &colour, f32 ZDepth = 1.0f) = 0;

        //! Resizes the backbuffer size.
        /*!
            If running in windowed mode then width and height will be equal to both the
            window size and backbuffer size.
            If running in fullscreen mode the width and height will be equal to the
            backbuffer size only.
         
            \param fullscreen Run in fullscreen or windowed mode.
            \param width Backbuffer width.
            \param height Backbuffer height.
            \return Ok on success, failed if any errors occured.
        */
		virtual eMASH_STATUS SetScreenResolution(bool fullscreen, uint32 width, uint32 height) = 0;

        //! Creates a geoemtry batch for efficent rendering.
        /*!
            The returned batch can be used to render a large amount of primitives efficently.
            It must be dropped when you are done with it.
            
            \param material Material the batch will use.
            \param type Primitive type of the batch.
            \param batchType Batch type.
            \return New geometry batch.
        */
		virtual MashGeometryBatch* CreateGeometryBatch(MashMaterial *material, ePRIMITIVE_TYPE type, MashGeometryBatch::eBATCH_TYPE batchType) = 0;

        //! Creates a mesh buffer for rendering.
		/*!
            The new buffer will need to be dropped when you are done with it.
         
            Mesh buffers are compiled API versions of MashMesh.
			Multiple streams are only needed in special situations, such as instancing.
         
            This will create new vertex and index buffers as needed.
         
            \param initVertexStreamData Streams data for each stream in the vertex declaration. This can be less
                than the number of streams in the vertex declaration. See sVertexStreamInit for more info.
            \param initVertexStreamCount Number of elements in the array.
            \param vertexDecl Vertex declaration.
            \param indexData Index array data.
            \param indexCount Number of elements in the index array. NOT the size in Bytes.
            \param indexFormat Format of the index array.
            \param indexUsage Index buffer usage.
            \return Created mesh buffer.
		*/
		virtual MashMeshBuffer* CreateMeshBuffer(const sVertexStreamInit *initVertexStreamData,
			uint32 initVertexStreamCount,
            const MashVertex *vertexDecl, 
			const void *indexData = 0, 
			uint32 indexCount = 0, 
			eFORMAT indexFormat = aFORMAT_R16_UINT,
			eUSAGE indexUsage = aUSAGE_STATIC) = 0;

        //! Creates a new vertex buffer.
        /*!
            The new buffer will need to be dropped when you are done with it.
         
            This can be used as a generic buffer for sending data to the GPU.
         
            If this buffer will one be set with data once then create the buffer as static. Static
            buffer performance is better than dynamic.
            If the buffer will be updated at runtime then create this buffer as dynamic.
            
            \param data Buffer data. May be NULL to create an empty buffer. The usage must be dynamic in that case.
            \param sizeInBytes Data size in bytes. If the data array is NULL then this is the buffer size in bytes.
            \param usage Buffer usage.
            \return New buffer.
        */
		virtual MashVertexBuffer* CreateVertexBuffer(const void *data, uint32 sizeInBytes, eUSAGE usage) = 0;

        //! Creates a new index buffer.
        /*!
            The new buffer will need to be dropped when you are done with it.
         
            Index buffers are used to reduce a vertex buffers size.
            
            \param data Buffer data. May be NULL to create an empty buffer. The usage must be dynamic in that case.
            \param indexCount Number of indices in the array. NOT the size in bytes. Look at CreateIndexBufferBySize()
                to create a buffer by size.
            \param usage Buffer usage.
            \param format The format of the elements in the data array.
            \return New buffer.
        */
		virtual MashIndexBuffer* CreateIndexBuffer(const void *data, uint32 indexCount, eUSAGE usage, eFORMAT format) = 0; 
		
        //! Creates a render surface using a cube texture.
        /*!
            The new render surface will need to be dropped when you are done with it.
            
            Render surfaces can be set using SetRenderTarget().
         
            \param size Texture pixel size.
            \param useMipmaps Set to true to use mipmapping, false to use only 1 mip level.
            \param textureFormat Texture format.
            \param useDepth Set to true for this render surface to have its own depth buffer. False for no depth.
            \param depthFormat If useDepth is true then this is the depth buffer format.
            \return New render surface.
        */
		virtual MashRenderSurface* CreateCubicRenderSurface(uint32 size, bool useMipmaps,
			eFORMAT textureFormat, bool useDepth, eFORMAT depthFormat = aFORMAT_DEPTH32_FLOAT) = 0;

        //! Creates a render surface.
		/*!
			The new render surface will need to be dropped when you are done with it.

			Passing -1 for width and height will make the surface fit to the default backbuffer, and will resize
            automatically if the backbuffer resizes.

            Render surfaces can have mutliple targets that can be written to from the pixel shader.
         
            Note, render targets will automatically call SetViewport() to set a viewport that is equal to
            their size when SetRenderTarget() is called.
         
            \param width Backbuffer width.
            \param height Backbuffer height.
            \param formats The format for each surface.
            \param targetCount Number of elements in the formats array.
            \param useMipmaps Set to true to use mipmapping, false to use only 1 mip level.
            \param depthOption Option using eDEPTH_BUFFER_OPTIONS.
                depth buffer.
            \param depthFormat If useDepth is true then this is the depth buffers format.
            \return New render surface.
		*/
		virtual MashRenderSurface* CreateRenderSurface(int32 iWidth, int32 iHeight, const eFORMAT *formats,
            uint32 targetCount, bool useMipmaps, eDEPTH_BUFFER_OPTIONS depthOption, eFORMAT depthFormat = aFORMAT_DEPTH32_FLOAT) = 0;

        //! Sets the active render target for writing.
		/*!
			The paramter surface specifies the surface to set as the target. For instance, you may wish
			to have a particular surface of an MRT set, or a particular face from a cube map. A value of
            -1 specifies all surfaces to be set from the MRT. Use eCUBEMAP_FACE to set a face for a cube target.
            The targets set can then be written to from the pixel shader.
         
            You probably want to call MashSceneManager::FlushGeometryBuffers() to make sure everything has
            been flushed to the target before swapping targets.
         
            Note, render targets will automatically call SetViewport() to set a viewport that is equal to
            their size.
         
            Setting the render target to NULL will not set the default target and the behaviour will
            be undefined. It will only unmount the current target. Call SetRenderTargetDefault() to 
            set the default target. SetRenderTargetDefault() must be called before leaving MashGameLoop::Render()
            if you have changed render targets.
         
            \param renderTarget Target to set.
            \param surface Surface from the render target to set. Leave as -1 to set all surfaces.
            \return Ok on success, failed if any errors occured.
		*/
		virtual eMASH_STATUS SetRenderTarget(MashRenderSurface *renderTarget, int32 surface = -1) = 0;
        
        //! Sets the default backbuffer as the active target.
        /*!
            This will need to be called before leaving MashGameLoop::Render() if you have
            set a custom render target using SetRenderTarget(). If this is not done then the final
            backbuffer image will be undefined.
         
            \return Ok on success, failed if any errors occured.
        */
		virtual eMASH_STATUS SetRenderTargetDefault() = 0;

        //! Gets the active render surface.
		/*!
			Returns the last target set by SetRenderTarget(). Note, the engine may set
			render targets during rendering so the last target the user set is not
			necessarily the target that will be returned.
         
            \return Last render target set. NULL if the default target is set.
		*/
		virtual MashRenderSurface* GetRenderSurface()const = 0;

        //! Draws an indexed list to the current render surface.
        /*!
            \param buffer Mesh buffer to render.
            \param vertexCount Number of vertices in vertex stream 0.
            \param indexCount Number of indies in the index list.
            \param primitiveCount Number of primitive in the index list.
            \param primType Primitive type.
            \return Ok on success, failed if any errors occured.
        */
		virtual eMASH_STATUS DrawIndexedList(const MashMeshBuffer *buffer, uint32 vertexCount, uint32 indexCount,
				uint32 primitiveCount, ePRIMITIVE_TYPE primType) = 0;

        //! Draws a vertex list to the current render surface.
        /*!
            \param buffer Mesh buffer to render.
            \param vertexCount Number of vertices in vertex stream 0.
            \param primitiveCount Number of primitive in the index list.
            \param primType Primitive type.
            \return Ok on success, failed if any errors occured.
         */
		virtual eMASH_STATUS DrawVertexList(const MashMeshBuffer *buffer, uint32 vertexCount,
			uint32 primitiveCount, ePRIMITIVE_TYPE primType) = 0;

        //! Draws a mesh buffer that contains instance data to the current render surface.
        /*!
            This renders a vertex list only, not index data.
         
            You would use this if your mesh buffer and vertex contains more than 1 vertex streams.
            More than 1 stream would be used if you are using HW instancing. Stream 0 would contain
            geometry data while streams > 0 contain data for each instance. The instance data maybe
            positional, colour, or any other information needed for each instance.
         
            \param buffer Mesh buffer to render.
            \param vertexCount Number of vertices in vertex stream 0.
            \param primitiveCount Number of primitive in the index list.
            \param primType Primitive type.
            \param instanceCount Number of instances to render.
            \return Ok on success, failed if any errors occured.
         */
		virtual eMASH_STATUS DrawVertexInstancedList(const MashMeshBuffer *buffer, uint32 vertexCount,
			uint32 primitiveCount, ePRIMITIVE_TYPE primType, uint32 instanceCount) = 0;

        //! Draws an indexed mesh buffer that contains instance data to the current render surface.
        /*!         
            You would use this if your mesh buffer and vertex contains more than 1 vertex streams.
            More than 1 stream would be used if you are using HW instancing. Stream 0 would contain
            geometry data while streams > 0 contain data for each instance. The instance data maybe
            positional, colour, or any other information needed for each instance.
         
            \param buffer Mesh buffer to render.
            \param vertexCount Number of vertices in vertex stream 0.
            \param indexCount Number of indies in the index list.
            \param primitiveCount Number of primitive in the index list.
            \param primType Primitive type.
            \param instanceCount Number of instances to render.
            \return Ok on success, failed if any errors occured.
         */
		virtual eMASH_STATUS DrawIndexedInstancedList(const MashMeshBuffer *buffer, uint32 vertexCount, uint32 indexCount,
				uint32 primitiveCount, ePRIMITIVE_TYPE primType, uint32 instanceCount) = 0;

        //! Draw mesh helper method.
        /*!
            This method selects which DrawVertexxxx or DrawIndexedxxx method to use to render
            the current mesh.
         
            \param mesh Mesh to render.
            \param instanceCount Instances to render. Set to 0 if instancing is not used in this mesh.
            \return Ok on success, failed if any errors occured.
        */
		virtual eMASH_STATUS DrawMesh(const MashMesh *mesh, uint32 instanceCount) = 0;
        
        //! Draws a fullscreen quad that can be used for post processing effects.
        /*!
            This uses the vertex declaration MashVertexPosTex. Your material will need to use
            this vertex type.
            Quads points range from : Top left (1, -1) Bottom right (-1, 1) and Depth is set to 1.
         
            Example usages:
         
            \code
            if (myMaterial->OnSet())
                renderer->DrawFullScreenQuad();
            \endcode
         
            \return Ok on success, failed if any errors occured.
        */
        virtual eMASH_STATUS DrawFullScreenQuad() = 0;

        //! Draws a full screen quad with custom texture coordinates.
        /*!
            This uses the vertex declaration MashVertexPosTex. Your material will need to use
            this vertex type.
            Quads points range from : Top left (1, -1) Bottom right (-1, 1) and Depth is set to 1.
         
            This function allows you to adjust the texture coordinates of the fullscreen quad. This
            allows you to zoom into a texture or clip it.
            SetViewport() could be called before hand to draw textures to different locations around
            the current render target. Also see DrawTextureClip() or DrawTexture().
         
            Example usages:
         
            \code
            if (myMaterial->OnSet())
                renderer->DrawFullScreenQuadTexCoords();
            \endcode
            
            \param texCoords Custom coordinates.
            \return Ok on success, failed if any errors occured.
        */
		virtual eMASH_STATUS DrawFullScreenQuadTexCoords(const mash::MashRectangle2 &texCoords) = 0;

        //! Draws a clipped texture to a particluar location on the current target.
        /*!
            This is a helper method for drawing a texture quickly. A rendering material will
            automatically be set to render the texture.
         
            \param texture Texture to render.
            \param screenPos Screen position to render to.
            \param clippingArea Area of the texture to render in pixels.
            \param isTransparent Enables alpha blending.
            \return Ok on success, failed if any errors occured.
        */
		virtual eMASH_STATUS DrawTextureClip(mash::MashTexture *texture,
			const mash::MashRectangle2 &screenPos,
			const mash::MashRectangle2 &clippingArea,
            bool isTransparent = false) = 0;
        
        //! Draws a texture to a screen position.
        /*!
            This is a helper method for drawing a texture quickly. A rendering material will
            automatically be set to render the texture.
         
            \param texture Texture to render.
            \param screenPos Screen area to render to.
            \param isTransparent Enables alpha blending.
            \return Ok on success, failed if any errors occured.
         */
		virtual eMASH_STATUS DrawTexture(mash::MashTexture *texture, const mash::MashRectangle2 &screenPos, 
                                        bool isTransparent = false) = 0;

        //! Gets the number of calls to Drawxxx this frame.
		/*!
            Debug method. This is reset before entering MashGameLoop::Render().
         
            \return Draw calls for the current frame.
		*/
		virtual uint32 GetCurrentFrameDrawCount()const = 0;
        
        //! Gets the number technique changes this frame.
        /*!
            Debug method. This is reset before entering MashGameLoop::Render().
         
            \return Technique changes for the current frame.
        */
		virtual uint32 GetCurrentFrameTechniqueChangeCount()const = 0;

        //! Gets the material manager.
		virtual mash::MashMaterialManager* GetMaterialManager()const = 0;

        //! Gets the render blackboard.
        /*!
            This is render info that is used for sending data to the GPU from
            auto parameters.
         
            \return Render info.
        */
		virtual MashRenderInfo* GetRenderInfo()const = 0;

		//! Creates a new texture.
        /*! 
            The returned pointer must not be dropped. To remove use RemoveTextureFromCache().
         
            Use GetTexture() if you want to load a texture from file.
         
            \param name Must be unique. Leave as "" to automatically generate a name.
            \param width Texture width.
            \param height Texture height.
            \param useMipmaps Enables mipmapping.
            \param usage Texture usage. Set to dynamic if you will be updating the texture frequently. Use static 
                if it will on be set once.
            \param format Texture format.
            \return New texture. NULL if any errors or the name was not unqiue.
        */
		virtual MashTexture* AddTexture(const MashStringc &name,
										uint32 width, 
										uint32 height, 
										bool useMipmaps, 
										eUSAGE usage, 
										eFORMAT format) = 0;

        //! Creates a new cube texture.
        /*! 
            The returned pointer must not be dropped. To remove use RemoveTextureFromCache().
         
            Use GetTexture() if you want to load a texture from file.
         
            \param name Must be unique. Leave as "" to automatically generate a name.
            \param size Texture pixel size.
            \param useMipmaps Enables mipmapping.
            \param usage Texture usage. Set to dynamic if you will be updating the texture frequently. Use static 
                if it will on be set once.
            \param format Texture format.
            \return New texture. NULL if any errors or the name was not unqiue.
         */
		virtual MashTexture* AddCubeTexture(const MashStringc &name,
											uint32 size,
											bool useMipmaps,
											eUSAGE usage, 
											eFORMAT format) = 0;

        //! Loads a texture from file.
		/*!
			If a texture with the same file name has been previously loaded then a pointer
			to that instance will be returned.

			\param fileName Location of the texture.
			\return pointer to the texture. NULL if load failed.
		*/
		virtual MashTexture* GetTexture(const MashStringc &fileName) = 0;

        //! Removes a texture from the internal list.
		/*!
			This removes the texture from the internal texture list and drops
            its reference. If nothing else references the texture then
            it will be deleted from memory.

			\return True if the texture was deleted from memory.
		*/
		virtual bool RemoveTextureFromCache(MashTexture *texture) = 0;
		
        //! Removes all texture from the internal list and drops their reference.
        /*!
            If nothing else references the textures then they will be deleted from memory.
		*/
		virtual void RemoveAllTexturesFromCache() = 0;
        
        //! Internal method for creating a new vertex declartion.
        /*!
            This method will first search to find any previous vertex declaration created
            with the same elements. If one is found then it will be returned rather than
            creating another.
         
            \param material Material that uses this vertex declaration.
            \param vertexDecl Vertex element array.
            \param declCount Number of vertex elements.
            \return Vertex declaration.
        */
		virtual MashVertex* _CreateVertexType(MashMaterial *material,
                                             const sMashVertexElement *vertexDecl,
                                             uint32 declCount) = 0;
        
        //! Called to initialise the renderer.
        /*!
            \param device Device.
            \param creationParameters Creation parameters.
            \param extraData This is dependent on the API.
            \return Ok on success, failed if any errors occured.
        */
        virtual eMASH_STATUS _Initialise(MashDevice *device, const sMashDeviceSettings &creationParameters, void *extraData) = 0;
        
        //! Called internally on resolution change.
        /*!
            \param width New backbuffer width.
            \param height New backbuffer height.
            \return Ok on success, failed if any errors occured.
        */
		virtual eMASH_STATUS OnResolutionChange(uint32 width, uint32 height) = 0;
        
        //! Creates a new empty mesh buffer. The returned pointer must be dropped when you are done with it.
        virtual MashMeshBuffer* _CreateMeshBuffer() = 0;
        
		//! Called by any class considered to be a dependency when it has been compiled. This then notifies anthing waiting on a compile.
        /*!
            It then notifies anything depending on it that it has been compiled.
         
            \param dependency Resource that was just compiled.
        */
		virtual void _OnDependencyCompiled(MashMaterialDependentResourceBase *dependency) = 0;
        
        //! Called when a material dependent resource is created.
        /*!
            This allows compiling of API resources to be delayed until everything has been loaded.
            This reduces resources needing recompiling as settings change.
         
            \param dependency Object the new resource is dependent on.
            \param resource New resource that was created.
        */
		virtual void _AddCompileDependency(MashMaterialDependentResourceBase *dependency, MashMaterialDependentResourceBase *resource) = 0;

		//! Sets the scene manager.
		virtual void _SetSceneManager(MashSceneManager *sceneManager) = 0;
        
        //! Used internally by techniques when technique swaps happen.
        /*!
            Debug method for counting technique changes. Increments the counter by 1.
        */
		virtual void _IncrementCurrentFrameTechniqueChanges() = 0;
	};
}

#endif