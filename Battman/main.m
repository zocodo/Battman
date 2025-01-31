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
            /* For some reason, libintl's locale guess was not quite working,
               this is a workaround to force it read correct language */
            char *lang = preferred_language();
            setlocale(LC_ALL, lang);
            free(lang);

            char mainBundle[PATH_MAX];
            uint32_t size = sizeof(mainBundle);
            char binddir[PATH_MAX];
            if (_NSGetExecutablePath(mainBundle, &size) == KERN_SUCCESS) {
                char *bundledir = dirname(mainBundle);
                /* Either /Applications/Battman.app/locales or ./locales */
                sprintf(binddir, "%s/%s", bundledir ? bundledir : ".", "locales");
                char *bindbase = bindtextdomain_ptr(BATTMAN_INTL, binddir);
                if (bindbase) {
                    DBGLOG(@"i18n base dir: %s", bindbase);
                    char *dom = textdomain_ptr(BATTMAN_INTL);
                    DBGLOG(@"textdomain: %s", dom);
                    use_libintl = true;
                } else {
                    show_alert("Error", "Failed to get i18n base", "Cancel");
                }
            } else {
                show_alert("Error", "Unable to get executable path", "Cancel");
            }
        } else {
            show_alert("Warning", "Failed to load Gettext localization, defaulting to English", "OK");
        }
#ifdef _
#undef _
#endif
// Redefine _() for PO template generation
#define _(x) gettext_ptr(x)
        char *locale_name = _("locale_name");
        DBGLOG(@"Locale Name: %s", locale_name);
        if (use_libintl && !strcmp("locale_name", locale_name)) {
            show_alert("Error", "Unable to match existing Gettext localization, defaulting to English", "Cancel");
        }
#undef _
#define _(x) cond_localize(x)
    });

    DBGLOG(@"gettext_ptr(%s) = %s", str, gettext_ptr(str));
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

    /* Not running as App, CLI/Daemon code */
    {
        // TODO: cli + x11
    }
    return EXIT_SUCCESS;
}
