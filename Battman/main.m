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

/* Use gettext i18n for App & CLI consistency */
/* While running as CLI, NSBundle is unset,
   which means we cannot use Localizables.strings
   and NSLocalizedString() at such scene. */
NSString *cond_localize(char *str) {
    static dispatch_once_t onceToken;
    static bool use_libintl = false;

    dispatch_once(&onceToken, ^{
        if (libintl_available()) {
            char mainBundle[PATH_MAX];
            uint32_t size = sizeof(mainBundle);
            char binddir[PATH_MAX];
            if (_NSGetExecutablePath(mainBundle, &size)) {
                char *bundledir = dirname(mainBundle);
                /* Either /Applications/Battman.app/locales or ./locales */
                sprintf(binddir, "%s/%s", bundledir ? bundledir : ".", "locales");
                char *bindbase = bindtextdomain_ptr(BATTMAN_INTL, binddir);
                if (bindbase) {
                    NSLog(@"i18n base dir: %s", bindbase);
                    textdomain(BATTMAN_INTL);
                    use_libintl = true;
                } else {
                    show_alert("Error", "Failed to get i18n base", "Cancel");
                }
            }
        } else {
            show_alert("Warning", "Failed to load Gettext localization, defaulting to English", "OK");
        }
    });

    return [NSString stringWithCString:(use_libintl ? gettext_ptr(str) : str) encoding:NSUTF8StringEncoding];
}

int main(int argc, char * argv[]) {
    /* UIApplicationMain/NSApplicationMain only works when App launched with NSBundle */
    if ([NSBundle mainBundle]) {
#if TARGET_OS_IPHONE
        NSString * appDelegateClassName;
        @autoreleasepool {
            // Setup code that might create autoreleased objects goes here.
            appDelegateClassName = NSStringFromClass([AppDelegate class]);
        }
        /* TODO: add X11 and AppKit support? */
        return UIApplicationMain(argc, argv, nil, appDelegateClassName);
#else
        @autoreleasepool {
        }
        return NSApplicationMain(argc, argv);
#endif
    }

    return EXIT_SUCCESS;
}
