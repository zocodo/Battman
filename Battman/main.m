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
NSString *cond_localize(unsigned long long localize_id) {
	if (localize_id > 10000)
		return [NSString stringWithUTF8String:(const char *)localize_id];
	int preferred_language = 0; // current: 0=eng 1=cn
	// ^^ TODO: Make it dynamically modifyable
	// Also TODO: detect locale
	return (__bridge NSString *)localization_arr[LOCALIZATION_COUNT * preferred_language + localize_id - 1];
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
            //setenv("LANG", lang, 1);

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
                    char *enc = bind_textdomain_codeset_ptr(BATTMAN_TEXTDOMAIN, "UTF-8");
                    DBGLOG(@"codeset: %s", enc);
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
            DBGALT(lang, locale_name, "OK");
            free(lang);
        } else {
            show_alert("Warning", "Failed to load Gettext, defaulting to English", "OK");
        }
    });

    return [NSString stringWithCString:(use_libintl ? gettext_ptr(str) : str) encoding:NSUTF8StringEncoding];
}
#endif

#ifdef DEBUG
NSMutableAttributedString *redirectedOutput;
#endif

int main(int argc, char * argv[]) {
#if DEBUG
    char *tty = ttyname(0);
    if (tty) {
        show_alert("Current TTY", tty, "OK");
    } else {
        redirectedOutput = [[NSMutableAttributedString alloc] init];
        // Create a pipe for redirecting output
        static int pipe_fd[2];
        pipe(pipe_fd);

        // Save the original stdout and stderr file descriptors
        int __unused original_stdout = dup(STDOUT_FILENO);
        int __unused original_stderr = dup(STDERR_FILENO);

        // Redirect stdout and stderr to the pipe
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO);

        // Create a new dispatch queue to read from the pipe
        dispatch_queue_t queue = dispatch_queue_create("outputRedirectQueue", NULL);
        dispatch_async(queue, ^{
            char buffer[1024];
            ssize_t bytesRead;

            while ((bytesRead = read(pipe_fd[0], buffer, sizeof(buffer))) > 0) {
                // Append output to NSMutableAttributedString
                NSString *output = [[NSString alloc] initWithBytes:buffer length:bytesRead encoding:NSUTF8StringEncoding];
                NSAttributedString *attrString = [[NSAttributedString alloc] initWithString:output];
                [redirectedOutput appendAttributedString:attrString];
            }
        });

        // Close the write end of the pipe
        close(pipe_fd[1]);
    }
#endif
    // sleep(10);
    /* UIApplicationMain/NSApplicationMain only works when App launched with NSBundle */
    /* FIXME: NSBundle still exists if with Info.plist, we need better detection */
    if ([NSBundle mainBundle] && getenv("XPC_SERVICE_NAME")) {
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
