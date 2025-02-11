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

CFStringRef localization_arr[]={
	// !LOCALIZATION_ARR_CODE!
	/* ^ DO NOT REMOVE, will be autoprocessed */
};

#ifndef LOCALIZATION_COUNT
#define LOCALIZATION_COUNT 1
#endif

#ifndef USE_GETTEXT
NSString *cond_localize(int localize_id) {
	int preferred_language=0; // current: 0=eng 1=cn
	// ^^ TODO: Make it dynamically modifyable
	// Also TODO: detect locale
	return (__bridge NSString *)localization_arr[LOCALIZATION_COUNT*preferred_language+localize_id-1];
}
#else
/* Use gettext i18n for App & CLI consistency */
/* While running as CLI, NSBundle is unset,
   which means we cannot use Localizables.strings
   and NSLocalizedString() at such scene. */
/* TODO: try implement void *cond_localize(void *strOrCFSTR)? */
NSString *cond_localize(const char *str) {
    static dispatch_once_t onceToken;
    static bool use_libintl = false;

    dispatch_once(&onceToken, ^{
        if (libintl_available()) {
#ifdef DEBUG
            assert(bindtextdomain_ptr != NULL);
            assert(textdomain_ptr != NULL);
            assert(gettext_ptr != NULL);
#endif
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
                char *bindbase = bindtextdomain_ptr(BATTMAN_TEXTDOMAIN, binddir);
                if (bindbase) {
                    DBGLOG(@"i18n base dir: %s", bindbase);
                    char *dom = textdomain_ptr(BATTMAN_TEXTDOMAIN);
                    DBGLOG(@"textdomain: %s", dom);
                    use_libintl = true;
                } else {
                    show_alert("Error", "Failed to get i18n base", "Cancel");
                }
            } else {
                show_alert("Error", "Unable to get executable path", "Cancel");
            }
#ifdef _
#undef _
#endif
// Redefine _() for PO template generation
#define _(x) gettext_ptr(x)
            /* locale_name should not be "locale_name" if target language has been translated */
            char *locale_name = _("locale_name");
            DBGLOG(@"Locale Name: %s", locale_name);
            if (use_libintl && !strcmp("locale_name", locale_name)) {
                show_alert("Error", "Unable to match existing Gettext localization, defaulting to English", "Cancel");
            }
#undef _
#define _(x) cond_localize(x)
            DBGLOG(@"gettext_ptr(%s) = %s", str, gettext_ptr(str));
        } else {
            show_alert("Warning", "Failed to load Gettext, defaulting to English", "OK");
        }
    });

    return [NSString stringWithCString:(use_libintl ? gettext_ptr(str) : str) encoding:NSUTF8StringEncoding];
}
#endif

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
