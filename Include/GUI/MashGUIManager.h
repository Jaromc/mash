//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------

#ifndef _MASH_GUI_MANAGER_H_
#define _MASH_GUI_MANAGER_H_

#include "MashEnum.h"
#include "MashReferenceCounter.h"
#include "MashGUICustomRender.h"
#include "MashGUIEventDispatch.h"
#include "MashGUIComponent.h"
#include "MashGUIButton.h"
#include "MashGUISprite.h"
#include "MashGUIWindow.h"
#include "MashGUIScrollBar.h"
#include "MashGUIScrollbarView.h"
#include "MashGUICheckBox.h"
#include "MashGUICore.h"
#include "MashGUITabControl.h"
#include "MashGUIFont.h"
#include "MashGUIStaticText.h"
#include "MashGUISkin.h"
#include "MashGUIStyle.h"
#include "MashGUIListBox.h"
#include "MashGUIPopupMenu.h"
#include "MashGUIMenuBar.h"
#include "MashGUITextBox.h"
#include "MashGUIOpenFileDialog.h"
#include "MashGUIView.h"
#include "MashGUITree.h"
#include "MashGUIViewport.h"
#include "MashGUILoadCallback.h"
#include "MashInputManager.h"
#include "MashCreationParameters.h"
#include "MashLog.h"

namespace mash
{
	class MashGUIFactory;
	class MashVideo;

	class MashGUIManager : public MashReferenceCounter, public MashGUIEventDispatch
	{
	public:
		MashGUIManager():MashReferenceCounter(){}

		virtual ~MashGUIManager(){}
	
		//! For internal use only.
		/*!
			Initialises the gui manager.
			\param settings Load settings.
			\param renderer Renderer.
			\param inputManager inputManager.
			\return Ok if everything was fine, failed otherwise.
		*/
		virtual eMASH_STATUS _Initialise(const mash::sMashDeviceSettings &settings, MashVideo *renderer, MashInputManager *inputManager) = 0;

		//! Gets the colour debug items are drawn.
		/*!
			\return Debug colour.
		*/
		virtual const mash::sMashColour& GetDebugDrawColour()const = 0;

		//! Sets the debug draw colour.
		/*!
			\param colour Debug colour.
		*/
		virtual void SetDebugDrawColour(const mash::sMashColour &colour) = 0;

		//! Loads a layout file.
		/*!
			\param layoutFileName Layout file name.
			\param root The root node to attach the loaded object to. Can be NULL.
			\param callback This will be called when new items are created.
			\return Ok on success, failed otherwise.
		*/
		virtual eMASH_STATUS LoadGUILayout(const int8 *layoutFileName, MashGUIView *root = 0, MashGUILoadCallback *callback = 0) = 0; 

		//! Saves a gui scene to file.
		/*!
			\param layoutFileName Layout file name to save to.
			\param root The scene to save,
			\param saveRoot true to save the root, false to just save from the children.
			\return Ok on success, failed otherwise.
		*/
		virtual eMASH_STATUS SaveGUILayout(const int8 *layoutFileName, MashGUIComponent *root, bool saveRoot) = 0;

		//! Loads a style from file.
		/*!
			A style can be loaded with custom elements. Otherwise default values will be used.
			The custom element ids will be equal to their location within the array.

			\param styleFileName Style file name.
			\param customElements Custom element string array. Must be NULL terminated. 
			\return Ok on success, failed otherwise.
		*/
		virtual eMASH_STATUS LoadGUIStyle(const int8 *styleFileName, const int8 **customElements = 0) = 0;

		//! This must be called before drawing any gui items.
		/*!
			Once all gui items have been rendered call EndDraw() to end the draw.
			This combination maybe done multiple times per frame if needed.

			\return Ok on success, failed otherwise.
		*/
		virtual eMASH_STATUS BeginDraw() = 0;

		//! Call this after all gui rendering.
		/*!
			BeginDraw() must have been called previously.
		*/
		virtual void EndDraw() = 0;

		//! Draws and updates all gui items if needed.
		/*!
			Make sure BeginDraw() is called before calling this.
		*/
		virtual void DrawAll() = 0;

		//! Draws an item.
		/*!
			Make sure BeginDraw() is called before calling this.

			\param component Item to draw.
		*/
		virtual void DrawComponent(MashGUIComponent *component) = 0;

		//! Draws a sprite to the screen.
		/*!
			Make sure BeginDraw() is called before calling this.

			\param vertices Vertices to draw.
			\param vertexCount Vertex count.
			\param skin Skin to render with.
			\param transparencyOverride Overrides the transparency value within the skin.
		*/
		virtual void DrawSprite(const mash::MashVertexGUI::sMashVertexGUI *vertices,
			uint32 vertexCount,
			const MashGUISkin *skin,
			const sGUIOverrideTransparency &transparencyOverride) = 0;

		//! Draws a sprite to the screen.
		/*!
			Make sure BeginDraw() is called before calling this.

			\param rect Region to draw to.
			\param clippingRect The region will be clipped against this.
			\param skin Skin to render with.
			\param transparencyOverride Overrides the transparency value within the skin.
		*/
		virtual void DrawSprite(const mash::MashRectangle2 &rect,
			const mash::MashRectangle2 &clippingRect,
			const MashGUISkin *skin,
			const sGUIOverrideTransparency &transparencyOverride) = 0;

		//! Draws text to the screen.
		/*!
			Make sure BeginDraw() is called before calling this.

			\param vertices Vertices to draw.
			\param vertexCount Vertex count.
			\param texture Font texture.
			\param fontColour Font colour.
		*/
		virtual void DrawText(const mash::MashVertexPosTex::sMashVertexPosTex *vertices,
			uint32 vertexCount,
			mash::MashTexture *texture,
			const mash::sMashColour &fontColour) = 0;

		//! Draws a boarder.
		/*!
			Make sure BeginDraw() is called before calling this.

			\param rect Boarder to draw.
			\param colour Boarder colour.
		*/
		virtual void DrawBorder(const mash::MashRectangle2 &rect,
			const mash::sMashColour &colour) = 0;

		//! Draws a solid rect.
		/*!
			Make sure BeginDraw() is called before calling this.

			\param rect Rect to draw.
			\param colour Rect colour.
		*/
		virtual void DrawSolidShape(const mash::MashRectangle2 &rect,
			const mash::sMashColour &colour) = 0;

		//! Draws a vertex triangle list.
		/*!
			Make sure BeginDraw() is called before calling this.

			\param vertices Vertices to render.
			\param vertexCount Vertex count.
		*/
		virtual void DrawSolidTriangles(const mash::MashVertexColour::sMashVertexColour *vertices, uint32 vertexCount) = 0;

		//! Draws a line.
		/*!
			Make sure BeginDraw() is called before calling this.

			\param start Line start.
			\param end Line end.
			\param colour Line colour.
		*/
		virtual void DrawLine(const mash::MashVector2 &start, const mash::MashVector2 &end,
			const mash::sMashColour &colour) = 0;

		//! Adds a viewport.
		/*!
			\param destRegion Destination region.
			\param parent parent to attach the new component to. If null then it will be attached to the root node.
			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUIViewport* AddViewport(const MashGUIRect &destRegion, MashGUIView *parent = 0, int32 styleElement = -1) = 0;

		//! Adds a sprite.
		/*!
			\param destRegion Destination region.
			\param parent parent to attach the new component to. If null then it will be attached to the root node.
			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUISprite* AddSprite(const MashGUIRect &destRegion, MashGUIView *parent = 0, int32 styleElement = -1) = 0;

		//! Adds a button.
		/*!
			\param destRegion Destination region.
			\param parent parent to attach the new component to. If null then it will be attached to the root node.
			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUIButton* AddButton(const MashGUIRect &destRegion, MashGUIView *parent = 0, int32 styleElement = -1) = 0;

		//! Adds a window.
		/*!
			\param destRegion Destination region.
			\param parent parent to attach the new component to. If null then it will be attached to the root node.
			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUIWindow* AddWindow(const MashGUIRect &destRegion, MashGUIView *parent = 0, int32 styleElement = -1) = 0;

		//! Adds a check box.
		/*!
			\param destRegion Destination region.
			\param parent parent to attach the new component to. If null then it will be attached to the root node.
			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUICheckBox* AddCheckBox(const MashGUIRect &destRegion, MashGUIView *parent = 0, int32 styleElement = -1) = 0;

		//! Adds a scrollbar.
		/*!
			\param destRegion Destination region.
			\param isVertical True if it's a vertical scrollbar. False if it's a horizontal scrollbar.
			\param incrementAmount 
			\param parent parent to attach the new component to. If null then it will be attached to the root node.
			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUIScrollBar* AddScrollBar(const MashGUIRect &destRegion, bool isVertical, f32 incrementAmount = 0.1f, MashGUIView *parent = 0, int32 styleElement = -1) = 0;

		//! Adds a tab control.
		/*!
			\param destRegion Destination region.
			\param parent parent to attach the new component to. If null then it will be attached to the root node.
			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUITabControl* AddTabControl(const MashGUIRect &destRegion, MashGUIView *parent, int32 styleElement = -1) = 0;

		//! Adds static text.
		/*!
			\param destRegion Destination region.
			\param parent parent to attach the new component to. If null then it will be attached to the root node.
			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUIStaticText* AddStaticText(const MashGUIRect &destRegion, MashGUIView *parent, int32 styleElement = -1) = 0;

		//! Adds a tab control.
		/*!
			\param destRegion Destination region.
			\param parent parent to attach the new component to. If null then it will be attached to the root node.
			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUIListBox* AddListBox(const MashGUIRect &destRegion, MashGUIView *pParent, int32 styleElement = -1) = 0;

		//! Creates a popup.
		/*!
			The returned pointer must be dropped by the owner.

			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUIPopupMenu* CreatePopupMenu(int32 styleElement = -1) = 0;
		
		//! Adds a menubar.
		/*!
			\param parent parent to attach the new component to. If null then it will be attached to the root node.
			\param left Left destination region.
			\param right Right destination region.
			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUIMenuBar* AddMenuBar(MashGUIView *parent, const MashGUIUnit &left = MashGUIUnit(0.0f, 0.0f), const MashGUIUnit &right = MashGUIUnit(1.0f, 0.0f), int32 styleElement = -1) = 0;

		//! Adds a textbox.
		/*!
			\param destRegion Destination region.
			\param parent parent to attach the new component to. If null then it will be attached to the root node.
			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUITextBox* AddTextBox(const MashGUIRect &destRegion, MashGUIView *parent, int32 styleElement = -1) = 0;

		//! Adds a tree.
		/*!
			\param destRegion Destination region.
			\param parent parent to attach the new component to. If null then it will be attached to the root node.
			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUITree* AddTree(const MashGUIRect &destRegion, MashGUIView *parent, int32 styleElement = -1) = 0;

		//! Adds a view.
		/*!
			\param destRegion Destination region.
			\param parent parent to attach the new component to. If null then it will be attached to the root node.
			\param styleElement The style element to use for this item. Use -1 to use default values.
			\return New component.
		*/
		virtual MashGUIView* AddView(const MashGUIRect &destRegion, MashGUIView *parent, int32 styleElement = -1) = 0;

		//! Creates a file dialog.
		/*!
			Can be used for generic file selection.
			Most application only need to create one of these if needed.
			The returned pointer must be dropped by the owner.

			\param destRegion Destination region.
			\return New component.
		*/
		virtual MashGUIOpenFileDialog* CreateOpenFileDialog(const MashGUIRect &destRegion) = 0;

		//! Adds a debug window.
		/*!
			Error logging can be displayed in this window.

			\param destRegion Destination region.
			\param logFlags Errors to log in this window.
			\param maxMessageCount maximum messages to be rendered to this window. Old messages will be removed when this limit is reached.
			\return New component.
		*/
		virtual MashGUIWindow* AddDebugLogWindow(const MashGUIRect &destRegion, uint32 logFlags = MashLog::aERROR_LEVEL_ALL, uint32 maxMessageCount = 10) = 0;

		//! Gets a font.
		/*!
			If the font is not found then it will be loaded.
			\param texture font texture.
			\param dataFileName Font file name.
			\return Font.
		*/
		virtual MashGUIFont* GetFont(mash::MashTexture *texture, const int8 *dataFileName) = 0;
		
		//! Gets the root window.
		/*!
			\return Root view.
		*/
		virtual MashGUIView* GetRootWindow()const = 0;

		//! Called when an object is made visible.
		/*!
			\param element Element.
		*/
		virtual void OnShowElement(MashGUIComponent *element) = 0;

		//! Called when an object is hidden.
		/*!
			\param element Element.
		*/
		virtual void OnHideElement(MashGUIComponent *element) = 0;

		//! Called when the screen is resized.
		/*!
			This will inturn call the root window and pass on the message.
		*/
		virtual void OnResize() = 0;

		//! Called when an element is destroyed.
		/*!
			\param component Component.
		*/
		virtual void OnDestroyElement(MashGUIComponent *component) = 0;

		//! Sets an item to have focus.
		/*!
			\param hasFocus Component to set focused.
		*/
		virtual void SetFocusedElement(MashGUIComponent *hasFocus) = 0;

		//! Gets the focused component.
		/*!
			\return Focused element. NULL if no element was focused.
		*/
		virtual MashGUIComponent* GetFocusedElement() = 0;
		
		//! Sets the active style.
		/*!
			\param name Style to set as active.
			\return Ok on success, failed otherwise.
		*/
		virtual eMASH_STATUS SetGUIStyle(const MashStringc &name) = 0;

		//! Gets a style.
		/*!
			\param name Style name.
			\return Style. NULL if the style doesn't exist.
		*/
		virtual MashGUIStyle* GetGUIStyle(const MashStringc &name) = 0;

		//! Gets the active gui style.
		/*!
			\return Active gui style.
		*/
		virtual MashGUIStyle* GetActiveGUIStyle() = 0;

		//! Flushes render buffers.
		virtual void FlushBuffers() = 0;

		//! Gets the gui factory.
		/*!
			This should be accessed only when creating composite objects such as
			the file dialog. Otherwise all creation should be done with the 
			gui manager.

			\return Gui factory.
		*/
		virtual MashGUIFactory* _GetGUIFactory()const = 0;		

		//! Called when an element is destroyed.
		/*!
			\param element Element to destroy.
		*/
		virtual void _DestroyElement(MashGUIComponent *element) = 0;
	};

	/*!
		Objects creates from here are the responsibility of the parent, not the gui manager.
	*/
	class MashGUIFactory : public MashReferenceCounter
	{
	public:
		MashGUIFactory():MashReferenceCounter(){}
		virtual ~MashGUIFactory(){}

		virtual MashGUIViewport* CreateViewport(const MashGUIRect &destRegion, MashGUIComponent *pParent = 0, int32 styleElement = -1) = 0;
		virtual MashGUISprite* CreateSprite(const MashGUIRect &destRegion, MashGUIComponent *pParent = 0, int32 styleElement = -1) = 0;
		virtual MashGUIButton* CreateButton(const MashGUIRect &destRegion, MashGUIComponent *pParent = 0, int32 styleElement = -1) = 0;
		virtual MashGUIWindow* CreateWindowView(const MashGUIRect &destRegion, MashGUIComponent *pParent = 0, int32 styleElement = -1) = 0;
		virtual MashGUICheckBox* CreateCheckBox(const MashGUIRect &destRegion, MashGUIComponent *pParent = 0, int32 styleElement = -1) = 0;
		virtual MashGUIScrollBar* CreateScrollBar(const MashGUIRect &destRegion, bool isVertical, f32 incrementAmount = 0.1f, MashGUIView *pParent = 0, int32 styleElement = -1) = 0;
		virtual MashGUITabControl* CreateTabControl(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement = -1) = 0;
		virtual MashGUIStaticText* CreateStaticText(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement = -1) = 0;
		virtual MashGUIListBox* CreateListBox(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement = -1) = 0;
		virtual MashGUITree* CreateTree(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement = -1) = 0;
		virtual MashGUIPopupMenu* CreatePopupMenu(int32 styleElement = -1) = 0;
		virtual MashGUIMenuBar* CreateMenuBar(MashGUIComponent *pParent, const MashGUIUnit &left = MashGUIUnit(0.0f, 0.0f), const MashGUIUnit &right = MashGUIUnit(1.0f, 0.0f), int32 styleElement = -1) = 0;
		virtual MashGUITextBox* CreateTextBox(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement = -1) = 0;
		virtual MashGUIView* CreateView(const MashGUIRect &destRegion, MashGUIComponent *pParent, int32 styleElement = -1) = 0;
		virtual MashGUIOpenFileDialog* CreateOpenFileDialog(const MashGUIRect &destRegion) = 0;
		virtual MashGUIStaticText* CreateDebugLog(const MashGUIRect &destRegion, MashGUIComponent *pParent, uint32 logFlags = MashLog::aERROR_LEVEL_ALL, uint32 maxMessageCount = 10) = 0;
		//for internal use only
		virtual MashGUIScrollbarView* CreateScrollBarView(bool isVertical, MashGUIComponent *pParent = 0, int32 styleElement = -1) = 0;
	};

	MashGUIManager* CreateMashGUI();
}

#endif
