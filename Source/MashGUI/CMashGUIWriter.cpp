//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#include "CMashGUIWriter.h"
#include "MashDevice.h"
#include "MashFileManager.h"
#include "MashGUIManager.h"
#include "MashGUIButton.h"
#include "MashGUICheckBox.h"
#include "MashGUIStaticText.h"
#include "MashGUITextBox.h"
#include "MashGUIMenuBar.h"
#include "MashGUISprite.h"
#include "MashGUIView.h"
#include "MashGUIWindow.h"
#include "MashGUIPopupMenu.h"
namespace mash
{
	void CMashGUIWriter::SaveTreeItems(MashXMLWriter *xmlWriter, const MashArray<MashGUITree::sItemData> &items)
	{
		for(uint32 i = 0; i < items.Size(); ++i)
		{
			xmlWriter->WriteChild("item");
			xmlWriter->WriteAttributeString("text", items[i].text.GetCString());
			xmlWriter->WriteAttributeInt("value", items[i].id);

			if (!items[i].children.Empty())
				SaveTreeItems(xmlWriter, items[i].children);

			xmlWriter->PopChild();
		}
	}

	void CMashGUIWriter::SavePopupMenu(MashXMLWriter *xmlWriter, MashGUIPopupMenu *popup)
	{
		MashArray<MashGUIPopupMenu::sPopupItem> items;
		popup->GetItems(items);
		for(uint32 i = 0; i < items.Size(); ++i)
		{
			xmlWriter->WriteChild("item");
			xmlWriter->WriteAttributeString("text", items[i].text.GetCString());
			xmlWriter->WriteAttributeInt("value", items[i].id);

			if (items[i].hasSubMenu)
				SavePopupMenu(xmlWriter, popup->GetItemSubMenu(items[i].id));

			xmlWriter->PopChild();
		}
	}

	void CMashGUIWriter::SaveComponent(MashXMLWriter *xmlWriter, const MashGUIComponent *component)
	{
		if (component->GetGUIType() != aGUI_SCROLL_BAR_VIEW)//dont save scrollbarviews. These are created by view elements.
		{
			xmlWriter->WriteChild("guielement");
			xmlWriter->WriteAttributeString("type", GetGUITypeAsString(component->GetGUIType()));
			xmlWriter->WriteAttributeString("name", component->GetName().GetCString());

			MashGUIRect destinationRegion;
			component->GetResetDestinationRegion(destinationRegion);

			xmlWriter->WriteChild("destinationregion");
			xmlWriter->WriteChild("left");
			xmlWriter->WriteAttributeDouble("scale", destinationRegion.left.scale);
			xmlWriter->WriteAttributeDouble("offset", destinationRegion.left.offset);
			xmlWriter->PopChild();
			xmlWriter->WriteChild("right");
			xmlWriter->WriteAttributeDouble("scale", destinationRegion.right.scale);
			xmlWriter->WriteAttributeDouble("offset", destinationRegion.right.offset);
			xmlWriter->PopChild();
			xmlWriter->WriteChild("top");
			xmlWriter->WriteAttributeDouble("scale", destinationRegion.top.scale);
			xmlWriter->WriteAttributeDouble("offset", destinationRegion.top.offset);
			xmlWriter->PopChild();
			xmlWriter->WriteChild("bottom");
			xmlWriter->WriteAttributeDouble("scale", destinationRegion.bottom.scale);
			xmlWriter->WriteAttributeDouble("offset", destinationRegion.bottom.offset);
			xmlWriter->PopChild();
			xmlWriter->PopChild();

			xmlWriter->WriteAttributeInt("clippingenabled", component->GetClippingEnabled());
			xmlWriter->WriteAttributeInt("canhavefocus", component->GetCanHaveFocus());
			xmlWriter->WriteAttributeInt("renderenabled", component->GetRenderEnabled());
			xmlWriter->WriteAttributeInt("focuslock", component->LockFocusWhenActivated());


			//TODO : The gui editor turns things off. They need to be re-enabled before saving.
			//xmlWriter->WriteAttributeInt("eventsenabled", component->GetEventsEnabled());

			switch(component->GetGUIType())
			{
			case aGUI_BUTTON:
				{
					MashGUIButton *button = (MashGUIButton*)component;
					xmlWriter->WriteAttributeString("text", button->GetText().GetCString());
					xmlWriter->WriteAttributeInt("isswitch", button->IsSwitch());
					break;
				}
			case aGUI_CHECK_BOX:
				{
					MashGUICheckBox *checkBox = (MashGUICheckBox*)component;
					xmlWriter->WriteAttributeInt("checked", checkBox->IsChecked());
					break;
				}
			case aGUI_STATIC_TEXT:
				{
					MashGUIStaticText *staticText = (MashGUIStaticText*)component;
					xmlWriter->WriteAttributeString("text", staticText->GetText().GetCString());
					xmlWriter->WriteAttributeInt("wordwrap", (bool)staticText->GetWordWrap());
					break;
				}
			case aGUI_TEXT_BOX:
				{
					MashGUITextBox *textBox = (MashGUITextBox*)component;
					xmlWriter->WriteAttributeString("text", textBox->GetText().GetCString());
					xmlWriter->WriteAttributeString("textformat", GetGUITextFormatAsString(textBox->GetTextFormat()));
					xmlWriter->WriteAttributeInt("precision", (int32)textBox->GetFloatPrecision());
					xmlWriter->WriteAttributeInt("usenumberbuttons", (bool)textBox->GetNumberButtonState());
					//xmlWriter->WriteAttributeInt("wordwrap", (bool)textBox->GetWordWrap());
					xmlWriter->WriteAttributeInt("minnumber", textBox->GetMinNumber());
					xmlWriter->WriteAttributeInt("maxnumber", textBox->GetMaxNumber());
					break;
				}
			case aGUI_TREE:
				{
					MashGUITree *tree = (MashGUITree*)component;
					MashArray<MashGUITree::sItemData> treeItems;
					tree->GetItems(treeItems);
					SaveTreeItems(xmlWriter, treeItems);
					break;
				}
			case aGUI_VIEW:
				{
					MashGUIView *view = (MashGUIView*)component;

					xmlWriter->WriteAttributeInt("hscrollenabled", view->IsHorizontalScrollEnabled());
					xmlWriter->WriteAttributeInt("vscrollenabled", view->IsVerticalScrollEnabled());
					xmlWriter->WriteAttributeInt("renderbackground", view->GetRenderbackgroundState());

					MashList<MashGUIComponent*>::ConstIterator iter = view->GetChildren().Begin();
					MashList<MashGUIComponent*>::ConstIterator end = view->GetChildren().End();
					for(; iter != end; ++iter)
					{
						SaveComponent(xmlWriter, *iter);
					}
					break;
				}
			case aGUI_WINDOW:
				{
					MashGUIWindow *window = (MashGUIWindow*)component;
					xmlWriter->WriteAttributeString("text", window->GetTitleBarText().GetCString());
					xmlWriter->WriteAttributeInt("closebuttonenabled", window->GetCloseButtonEnabled());
					xmlWriter->WriteAttributeInt("minimizebuttonenabled", window->GetMinimizeButtonEnabled());
					xmlWriter->WriteAttributeInt("mousedragenabled", window->GetMouseDragEnabled());
					xmlWriter->WriteAttributeInt("hscrollenabled", window->GetView()->IsHorizontalScrollEnabled());
					xmlWriter->WriteAttributeInt("vscrollenabled", window->GetView()->IsVerticalScrollEnabled());
					xmlWriter->WriteAttributeInt("renderbackground", window->GetView()->GetRenderbackgroundState());
					xmlWriter->WriteAttributeInt("alwaysontop", window->GetAlwaysOnTop());

					MashList<MashGUIComponent*>::ConstIterator iter = window->GetView()->GetChildren().Begin();
					MashList<MashGUIComponent*>::ConstIterator end = window->GetView()->GetChildren().End();
					for(; iter != end; ++iter)
					{
						/*
							dont save out a windows view. This is created automatically by a window on creation.
						*/
						if (*iter != window->GetView())
							SaveComponent(xmlWriter, *iter);
					}
					break;
				}
			case aGUI_SPRITE:
				{
					MashGUISprite *sprite = (MashGUISprite*)component;
					if (sprite->GetSkin()->GetTexture())
					{
						xmlWriter->WriteAttributeString("texture", sprite->GetSkin()->GetTexture()->GetName().GetCString());
						xmlWriter->WriteAttributeInt("texturesourceleft", (int32)sprite->GetSkin()->baseSource.left);
						xmlWriter->WriteAttributeInt("texturesourceright", (int32)sprite->GetSkin()->baseSource.right);
						xmlWriter->WriteAttributeInt("texturesourcetop", (int32)sprite->GetSkin()->baseSource.top);
						xmlWriter->WriteAttributeInt("texturesourcebottom", (int32)sprite->GetSkin()->baseSource.bottom);
					}

					break;
				}
			case aGUI_MENUBAR:
				{
					MashArray<MashGUIMenuBar::sMenuBarItem> menuItems;
					MashGUIMenuBar *menuBar = (MashGUIMenuBar*)component;
					menuBar->GetItems(menuItems);
					for(uint32 i = 0; i < menuItems.Size(); ++i)
					{
						xmlWriter->WriteChild("menuitem");
						xmlWriter->WriteAttributeString("text", menuItems[i].text.GetCString());
						xmlWriter->WriteAttributeInt("value", menuItems[i].userValue);

						SavePopupMenu(xmlWriter, menuBar->GetItemSubMenu(menuItems[i].id));

						xmlWriter->PopChild();
					}
					break;
				}
			case aGUI_LISTBOX:
				{
					MashGUIListBox *listBox = (MashGUIListBox*)component;

					xmlWriter->WriteAttributeInt("itemheight", listBox->GetItemHeight());
					xmlWriter->WriteAttributeInt("iconsenabled", listBox->GetIconsEnabled());

					xmlWriter->WriteChild("itemiconregion");
					xmlWriter->WriteChild("left");
					xmlWriter->WriteAttributeDouble("scale", listBox->GetItemIconRegion().left.scale);
					xmlWriter->WriteAttributeDouble("offset", listBox->GetItemIconRegion().left.offset);
					xmlWriter->PopChild();
					xmlWriter->WriteChild("right");
					xmlWriter->WriteAttributeDouble("scale", listBox->GetItemIconRegion().right.scale);
					xmlWriter->WriteAttributeDouble("offset", listBox->GetItemIconRegion().right.offset);
					xmlWriter->PopChild();
					xmlWriter->WriteChild("top");
					xmlWriter->WriteAttributeDouble("scale", listBox->GetItemIconRegion().top.scale);
					xmlWriter->WriteAttributeDouble("offset", listBox->GetItemIconRegion().top.offset);
					xmlWriter->PopChild();
					xmlWriter->WriteChild("bottom");
					xmlWriter->WriteAttributeDouble("scale", listBox->GetItemIconRegion().bottom.scale);
					xmlWriter->WriteAttributeDouble("offset", listBox->GetItemIconRegion().bottom.offset);
					xmlWriter->PopChild();
					xmlWriter->PopChild();

					xmlWriter->WriteChild("itemtextregion");
					xmlWriter->WriteChild("left");
					xmlWriter->WriteAttributeDouble("scale", listBox->GetItemTextRegion().left.scale);
					xmlWriter->WriteAttributeDouble("offset", listBox->GetItemTextRegion().left.offset);
					xmlWriter->PopChild();
					xmlWriter->WriteChild("right");
					xmlWriter->WriteAttributeDouble("scale", listBox->GetItemTextRegion().right.scale);
					xmlWriter->WriteAttributeDouble("offset", listBox->GetItemTextRegion().right.offset);
					xmlWriter->PopChild();
					xmlWriter->WriteChild("top");
					xmlWriter->WriteAttributeDouble("scale", listBox->GetItemTextRegion().top.scale);
					xmlWriter->WriteAttributeDouble("offset", listBox->GetItemTextRegion().top.offset);
					xmlWriter->PopChild();
					xmlWriter->WriteChild("bottom");
					xmlWriter->WriteAttributeDouble("scale", listBox->GetItemTextRegion().bottom.scale);
					xmlWriter->WriteAttributeDouble("offset", listBox->GetItemTextRegion().bottom.offset);
					xmlWriter->PopChild();
					xmlWriter->PopChild();

					for(uint32 i = 0; i < listBox->GetItemCount(); ++i)
					{
						xmlWriter->WriteChild("item");
						xmlWriter->WriteAttributeString("text", listBox->GetItemText(i).GetCString());
						xmlWriter->WriteAttributeInt("value", listBox->GetItemUserValue(i));

						mash::MashTexture *icon = listBox->GetItemIcon(i);
						if (icon)
						{
							mash::MashRectangle2 sourceRegion = listBox->GetItemIconSourceRegion(i);

							xmlWriter->WriteAttributeString("iconlocation", icon->GetName().GetCString());
							xmlWriter->WriteAttributeInt("iconsourceleft", (int32)sourceRegion.left);
							xmlWriter->WriteAttributeInt("iconsourceright", (int32)sourceRegion.right);
							xmlWriter->WriteAttributeInt("iconsourcetop", (int32)sourceRegion.top);
							xmlWriter->WriteAttributeInt("iconsourcebottom", (int32)sourceRegion.bottom);
						}

						xmlWriter->PopChild();
					}
					break;
				}
			};
			
			xmlWriter->PopChild();
		}
	}

	eMASH_STATUS CMashGUIWriter::Save(MashDevice *device, const MashStringc &filename, MashGUIComponent *root, bool saveRoot)
	{
		MashXMLWriter *xmlWriter = device->GetFileManager()->CreateXMLWriter(filename.GetCString(), "root");

		if (saveRoot)
		{
			SaveComponent(xmlWriter, root);
		}
		else
		{
			MashGUIView *tempRootNode = 0;
			if (root == 0)
				tempRootNode = device->GetGUIManager()->GetRootWindow();
			else if (root->GetGUIType() == aGUI_WINDOW)
				tempRootNode = ((MashGUIWindow*)root)->GetView();
			else if (root->GetGUIType() == aGUI_VIEW)
				tempRootNode = (MashGUIView*)root;

			if (tempRootNode)
			{
				MashList<MashGUIComponent*>::ConstIterator iter = tempRootNode->GetChildren().Begin();
				MashList<MashGUIComponent*>::ConstIterator end = tempRootNode->GetChildren().End();
				for(; iter != end; ++iter)
				{
					SaveComponent(xmlWriter, *iter);
				}
			}
		}

		xmlWriter->SaveAndDestroy();

		return aMASH_OK;
	}
}