//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#import "AppDelegate.h"

#ifdef MASH_APPLE

@implementation AppDelegate

- (id)initWithDevice:(mash::CMashMacDevice *)device
{
	self = [super init];
	if (self) 
        m_device = device;
        
    return (self);
}

- (void) applicationDidFinishLaunching:(NSNotification*)notification
{
    m_quit = FALSE;
}
 
- (void) terminate:(id)sender
{
    m_quit = TRUE;
}

- (BOOL) quitApp
{
    return m_quit;
}

- (void)windowWillClose:(id)sender
{
	m_quit = TRUE;
}

- (NSSize)windowWillResize:(NSWindow *)window toSize:(NSSize)newWindowSize
{
	if (m_device->IsResizable())
		return newWindowSize;
	else
		return [window frame].size;
}

- (void)windowDidResize:(NSNotification *)notification
{
    m_device->OnResize();
}

- (void)windowWillMove:(NSNotification *)notification
{
    m_device->OnWindowWillMove();
}

- (void)windowDidMove:(NSNotification *)notification
{
    m_device->OnWindowMoved();
}

- (void)orderFrontStandardAboutPanel:(id)sender
{
	[MASHpp orderFrontStandardAboutPanel:sender];
}

- (void)unhideAllApplications:(id)sender
{
	[MASHpp unhideAllApplications:sender];
}

- (void)hide:(id)sender
{
	[MASHpp hide:sender];
}

- (void)hideOtherApplications:(id)sender
{
	[MASHpp hideOtherApplications:sender];
}

@end

#endif