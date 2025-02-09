//
//  main.m
//  Battman
//
//  Created by Torrekie on 2025/1/18.
//

#include <TargetConditionals.h>

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#else
#import <Cocoa/Cocoa.h>
#endif

#include "common.h"
#include "intlextern.h"
#include <libgen.h>
#include <dlfcn.h>
#if __has_include(<mach-o/dyld.h>)
#include <mach-o/dyld.h>
#else
extern int _NSGetExecutablePath(char* buf, uint32_t* bufsize);
#endif

NSString *cond_localize(int localize_id) {
	int preferred_language=1; // current: 0=eng 1=cn
	return nil; // !COND_LOCALIZE_CODE!
	// ^ Do not remove, will be autoprocessed
}

int main(int argc, char * argv[]) {
    // sleep(10);
    /* UIApplicationMain/NSApplicationMain only works when App launched with NSBundle */
    if ([NSBundle mainBundle]) {
#if TARGET_OS_IPHONE
        NSString * appDelegateClassName;
        @autoreleasepool {
            // Setup code that might create autoreleased objects goes here.
            appDelegateClassName = NSStringFromClass([AppDelegate class]);
        }
        /* TODO: add X11 and AppKit support? */
        return UIApplicationMain(argc, argv, nil, @"AppDelegate");
#else
        @autoreleasepool {
        }
        return NSApplicationMain(argc, argv);
#endif
    }

    /* Not running as App, CLI/Daemon code */
    {
        // TODO: cli + x11
        fprintf(stderr, "%s\n", [_("Battman CLI not implemented yet.") UTF8String]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
