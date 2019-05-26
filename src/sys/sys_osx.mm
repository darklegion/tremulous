/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2000-2009 Darklegion Development
Copyright (C) 2015-2019 GrangerHub


This file is part of Tremulous.

Tremulous is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 3 of the License,
or (at your option) any later version.

Tremulous is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Tremulous; if not, see <https://www.gnu.org/licenses/>

===========================================================================
*/

#ifndef __APPLE__
#error This file is for Mac OS X only. You probably should not compile it.
#endif

// Please note that this file is just some Mac-specific bits. Most of the
// Mac OS X code is shared with other Unix platforms in sys_unix.c ...

#include "qcommon/q_shared.h"
#include "qcommon/qcommon.h"
#include "dialog.h"
#include "sys_local.h"

//#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <Carbon/Carbon.h>
#import <Cocoa/Cocoa.h>
//
//#import <AppKit/AppKitDefines.h>
//#import <Foundation/NSObject.h>
//#import <Foundation/NSArray.h>
//#import <Foundation/NSDictionary.h>
//#import <AppKit/NSAlert.h>

/*
==============
Sys_Dialog

Display an OS X dialog box
==============
*/
dialogResult_t Sys_Dialog( dialogType_t type, const char *message, const char *title )
{
	dialogResult_t result = DR_OK;
	NSAlert *alert = [NSAlert new];

	[alert setMessageText: [NSString stringWithUTF8String: title]];
	[alert setInformativeText: [NSString stringWithUTF8String: message]];

	if( type == DT_ERROR )
		[alert setAlertStyle: NSCriticalAlertStyle];
	else
		[alert setAlertStyle: NSWarningAlertStyle];

	switch( type )
	{
		default:
			[alert runModal];
			result = DR_OK;
			break;

		case DT_YES_NO:
			[alert addButtonWithTitle: @"Yes"];
			[alert addButtonWithTitle: @"No"];
			switch( [alert runModal] )
			{
				default:
				case NSAlertFirstButtonReturn: result = DR_YES; break;
				case NSAlertSecondButtonReturn: result = DR_NO; break;
			}
			break;

		case DT_OK_CANCEL:
			[alert addButtonWithTitle: @"OK"];
			[alert addButtonWithTitle: @"Cancel"];

			switch( [alert runModal] )
			{
				default:
				case NSAlertFirstButtonReturn: result = DR_OK; break;
				case NSAlertSecondButtonReturn: result = DR_CANCEL; break;
			}
			break;
	}

	[alert release];

	return result;
}
