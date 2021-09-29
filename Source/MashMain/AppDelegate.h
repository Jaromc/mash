//-------------------------------------------------------------------------
// This file is part of Mash 3D Engine
// Copyright (c) 2012-2016 Alegra Software
// For license and distribution see Mash.h
//-------------------------------------------------------------------------
#ifndef MacOSX_AppDelegate_h
#define MacOSX_AppDelegate_h

#include "MashDataTypes.h"
#ifdef MASH_APPLE

#import <Cocoa/Cocoa.h>
#import "CMashMacDevice.h"

@interface AppDelegate : NSObject
{
    BOOL m_quit;
    mash::CMashMacDevice *m_device;
}

- (id)initWithDevice:(mash::CMashMacDevice *)device;
- (BOOL) quitApp;

@end

#endif

#endif
