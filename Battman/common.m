#include <TargetConditionals.h>
#include <Foundation/Foundation.h>

#if TARGET_OS_IPHONE
#include <UIKit/UIKit.h>
extern UIWindow *gWindow;
#elif TARGET_OS_OSX
#include <Cocoa/Cocoa.h>
#endif

#include <regex.h>
#include <sys/sysctl.h>

#include "common.h"
#include "intlextern.h"
#include "gtkextern.h"
#include "battery_utils/battery_info.h"

/* Consider make this a standalone header */
#define SYM_EXIST(...) check_ptr(__VA_ARGS__)

#define PTR_TYPE_NAME(x, y) x ## _ptr = y; DBGLOG(@"%s_ptr (%p) = %s (%p)", #x, x, #y, y);
#define PTR_TYPE(x) PTR_TYPE_NAME(x, x)
#define PTR_TYPE_NAME_DLSYM(handle, y, x) ((y ## _ptr = (typeof(x)*)dlsym(handle, #x)) != NULL)
#define PTR_TYPE_DLSYM(handle, x) PTR_TYPE_NAME_DLSYM(handle, x, x)

typeof(gettext) *gettext_ptr;
typeof(textdomain) *textdomain_ptr;
typeof(bindtextdomain) *bindtextdomain_ptr;
typeof(bind_textdomain_codeset) *bind_textdomain_codeset_ptr;

typeof(gtk_dialog_get_type) *gtk_dialog_get_type_ptr;
typeof(gtk_message_dialog_new) *gtk_message_dialog_new_ptr;
typeof(gtk_dialog_add_button) *gtk_dialog_add_button_ptr;
typeof(gtk_dialog_run) *gtk_dialog_run_ptr;
typeof(gtk_widget_destroy) *gtk_widget_destroy_ptr;

static char *get_CFLocale()
{
    CFArrayRef list = CFLocaleCopyPreferredLanguages();
    
    if (list == NULL || CFArrayGetCount(list) == 0)
        return NULL;

    char *lang = (char *)malloc(256 * sizeof(char));

    if (!CFStringGetCString(CFArrayGetValueAtIndex(list, 0), lang, 256, kCFStringEncodingUTF8)) {
        CFRelease(list);
        return NULL;
    }

    CFRelease(list);
    return lang;
}

// Caller free!!!
char *preferred_language (void)
{
    /* Convert new-style locale names with language tags (ISO 639 and ISO 15924)
       to Unix (ISO 639 and ISO 3166) names.  */
    typedef struct { const char langtag[10+1]; const char unixy[12+1]; }
            langtag_entry;
    static const langtag_entry langtag_table[] = {
      /* Mac OS X has "az-Arab", "az-Cyrl", "az-Latn".
         The default script for az on Unix is Latin.  */
      { "az-Latn", "az" },
      /* Mac OS X has "bs-Cyrl", "bs-Latn".
         The default script for bs on Unix is Latin.  */
      { "bs-Latn", "bs" },
      /* Mac OS X has "ga-dots".  Does not yet exist on Unix.  */
      { "ga-dots", "ga" },
      /* Mac OS X has "kk-Cyrl".
         The default script for kk on Unix is Cyrillic.  */
      { "kk-Cyrl", "kk" },
      /* Mac OS X has "mn-Cyrl", "mn-Mong".
         The default script for mn on Unix is Cyrillic.  */
      { "mn-Cyrl", "mn" },
      /* Mac OS X has "ms-Arab", "ms-Latn".
         The default script for ms on Unix is Latin.  */
      { "ms-Latn", "ms" },
      /* Mac OS X has "pa-Arab", "pa-Guru".
         Country codes are used to distinguish these on Unix.  */
      { "pa-Arab", "pa_PK" },
      { "pa-Guru", "pa_IN" },
      /* Mac OS X has "shi-Latn", "shi-Tfng".  Does not yet exist on Unix.  */
      /* Mac OS X has "sr-Cyrl", "sr-Latn".
         The default script for sr on Unix is Cyrillic.  */
      { "sr-Cyrl", "sr" },
      /* Mac OS X has "tg-Cyrl".
         The default script for tg on Unix is Cyrillic.  */
      { "tg-Cyrl", "tg" },
      /* Mac OS X has "tk-Cyrl".
         The default script for tk on Unix is Cyrillic.  */
      { "tk-Cyrl", "tk" },
      /* Mac OS X has "tt-Cyrl".
         The default script for tt on Unix is Cyrillic.  */
      { "tt-Cyrl", "tt" },
      /* Mac OS X has "uz-Arab", "uz-Cyrl", "uz-Latn".
         The default script for uz on Unix is Latin.  */
      { "uz-Latn", "uz" },
      /* Mac OS X has "vai-Latn", "vai-Vaii".  Does not yet exist on Unix.  */
      /* Mac OS X has "yue-Hans", "yue-Hant".
         The default script for yue on Unix is Simplified Han.  */
      { "yue-Hans", "yue" },
      /* Mac OS X has "zh-Hans", "zh-Hant".
         Country codes are used to distinguish these on Unix.  */
      { "zh-Hans", "zh_CN" },

      { "zh-Hant", "zh_TW" },
      { "zh-Hant-HK", "zh_HK" },
    };
    /* Convert script names (ISO 15924) to Unix conventions.
       See https://www.unicode.org/iso15924/iso15924-codes.html  */
    typedef struct { const char script[4+1]; const char unixy[9+1]; }
            script_entry;
    static const script_entry script_table[] = {
      { "Arab", "arabic" },
      { "Cyrl", "cyrillic" },
      { "Latn", "latin" },
      { "Mong", "mongolian" }
    };
    char *name = get_CFLocale();
    /* Step 2: Convert using langtag_table and script_table.  */
    if ((strlen (name) == 7 || strlen (name) == 10) && name[2] == '-')
      {
        unsigned int i1, i2;
        i1 = 0;
        i2 = sizeof (langtag_table) / sizeof (langtag_entry);
        while (i2 - i1 > 1)
          {
            /* At this point we know that if name occurs in langtag_table,
               its index must be >= i1 and < i2.  */
            unsigned int i = (i1 + i2) >> 1;
            const langtag_entry *p = &langtag_table[i];
            if (strcmp (name, p->langtag) < 0)
              i2 = i;
            else
              i1 = i;
          }
        if (strncmp (name, langtag_table[i1].langtag, strlen(langtag_table[i1].langtag)) == 0)
          {
            strcpy (name, langtag_table[i1].unixy);
            return name;
          }

        i1 = 0;
        i2 = sizeof (script_table) / sizeof (script_entry);
        while (i2 - i1 > 1)
          {
            /* At this point we know that if (name + 3) occurs in script_table,
               its index must be >= i1 and < i2.  */
            unsigned int i = (i1 + i2) >> 1;
            const script_entry *p = &script_table[i];
            if (strcmp (name + 3, p->script) < 0)
              i2 = i;
            else
              i1 = i;
          }
        if (strcmp (name + 3, script_table[i1].script) == 0)
          {
            name[2] = '@';
            strcpy (name + 3, script_table[i1].unixy);
            return name;
          }
      }

    /* Step 3: Convert new-style dash to Unix underscore. */
    {
      char *p;
      for (p = name; *p != '\0'; p++)
        if (*p == '-')
          *p = '_';
    }
    return name;
}


/* https://developer.apple.com/library/archive/documentation/MacOSX/Conceptual/BPFrameworks/Concepts/WeakLinking.html
    Note: When checking for the existence of a symbol, you must explicitly compare it to NULL or nil in your code. You cannot use the negation operator ( ! ) to negate the address of the symbol.
*/
bool check_ptr(void* ptr1, ...)
{
    va_list args;
    va_start(args, ptr1);

    // Check the first pointer
    if (ptr1 == NULL) {
        return false;  // Return false if the first pointer is NULL
    }

    // Check subsequent pointers
    void* ptr;
    while ((ptr = va_arg(args, void*)) != NULL) {
        if (ptr == NULL) {
            va_end(args);
            return false;  // Return false if any pointer is NULL
        }
    }

    va_end(args);
    return true;  // Return true if all pointers are non-NULL
}



/* Conditional libintl */
bool libintl_available(void)
{
    static bool avail = false;
    static void *libintl_handle = NULL;

    if (avail) return avail;
    
    if (PTR_TYPE_DLSYM(NULL, gettext) &&
    	PTR_TYPE_DLSYM(NULL, bindtextdomain) &&
    	PTR_TYPE_DLSYM(NULL, textdomain) &&
        PTR_TYPE_DLSYM(NULL, bind_textdomain_codeset)) {
    	avail=true;
        DBGLOG(@"Avail as direct: %p %p %p %p", gettext_ptr, bindtextdomain_ptr, textdomain_ptr, bind_textdomain_codeset_ptr);
    } else if (PTR_TYPE_NAME_DLSYM(NULL, gettext, libintl_gettext) &&
               PTR_TYPE_NAME_DLSYM(NULL, bindtextdomain, libintl_bindtextdomain) &&
               PTR_TYPE_NAME_DLSYM(NULL, textdomain, libintl_textdomain) &&
               PTR_TYPE_NAME_DLSYM(NULL, bind_textdomain_codeset, libintl_bind_textdomain_codeset)) {
    	avail=true;
        DBGLOG(@"Avail as direct (libintl_*): %p %p %p %p", gettext_ptr, bindtextdomain_ptr, textdomain_ptr, bind_textdomain_codeset_ptr);
    }

    /*if (SYM_EXIST(gettext, bindtextdomain, textdomain)) {
        DBGLOG(@"Avail as direct: %p %p %p", gettext, bindtextdomain, textdomain);
        avail = true;
        PTR_TYPE(gettext);
        PTR_TYPE(bindtextdomain);
        PTR_TYPE(textdomain);
    } else if (SYM_EXIST(libintl_gettext, libintl_bindtextdomain, libintl_textdomain)) {
        DBGLOG(@"Avail as direct (libintl_*): %p %p %p", libintl_gettext, libintl_bindtextdomain, libintl_textdomain);
        avail = true;
        PTR_TYPE_NAME(gettext, libintl_gettext);
        PTR_TYPE_NAME(bindtextdomain, libintl_bindtextdomain);
        PTR_TYPE_NAME(textdomain, libintl_textdomain);
    }*/

    if (!avail) {
        if (!libintl_handle) {
            int i;
            for (i = 0; libintl_paths[i] != NULL; i++) {
                libintl_handle = dlopen(libintl_paths[i], RTLD_LAZY);
                if (libintl_handle) break;
            }
            //DBGALT("Using libintl: %s", libintl_paths[i], "OK");
        }

        if (libintl_handle) {
            if (PTR_TYPE_DLSYM(libintl_handle, gettext) &&
                PTR_TYPE_DLSYM(libintl_handle, bindtextdomain) &&
                PTR_TYPE_DLSYM(libintl_handle, textdomain) &&
                PTR_TYPE_DLSYM(libintl_handle, bind_textdomain_codeset)) {
                avail = true;
                DBGLOG(@"Avail as dlsym: %p %p %p %p", gettext_ptr, bindtextdomain_ptr, textdomain_ptr, bind_textdomain_codeset_ptr);
            } else if (PTR_TYPE_NAME_DLSYM(libintl_handle, gettext, libintl_gettext) &&
                       PTR_TYPE_NAME_DLSYM(libintl_handle, bindtextdomain, libintl_bindtextdomain) &&
                       PTR_TYPE_NAME_DLSYM(libintl_handle, textdomain, libintl_textdomain) &&
                       PTR_TYPE_NAME_DLSYM(libintl_handle, bind_textdomain_codeset, libintl_bind_textdomain_codeset)) {
                DBGLOG(@"Avail as dlsym (libintl_*): %p %p %p %p", gettext_ptr, bindtextdomain_ptr, textdomain_ptr, bind_textdomain_codeset_ptr);
                avail = true;
            }
        }
    }
    return avail;
}

/* Conditional libgtk */
bool gtk_available(void)
{
    static bool avail = false;
    static void *libgtk3_handle = NULL;

    if (avail) return avail;

    /*if (SYM_EXIST(gtk_dialog_get_type, gtk_message_dialog_new, gtk_dialog_add_button, gtk_dialog_run, gtk_widget_destroy)) {
        avail = true;
        PTR_TYPE(gtk_dialog_get_type);
        PTR_TYPE(gtk_message_dialog_new);
        PTR_TYPE(gtk_dialog_add_button);
        PTR_TYPE(gtk_dialog_run);
        PTR_TYPE(gtk_widget_destroy);
    }*/
    if (PTR_TYPE_DLSYM(NULL, gtk_dialog_get_type) &&
        PTR_TYPE_DLSYM(NULL, gtk_message_dialog_new) &&
        PTR_TYPE_DLSYM(NULL, gtk_dialog_add_button) &&
        PTR_TYPE_DLSYM(NULL, gtk_dialog_run) &&
        PTR_TYPE_DLSYM(NULL, gtk_widget_destroy)) avail = true;

    if (!avail) {
        if (!libgtk3_handle) {
            for (int i = 0; libgtk3_paths[i] != NULL; i++) {
                libgtk3_handle = dlopen(libgtk3_paths[i], RTLD_LAZY);
                if (libgtk3_handle) break;
            }
        }

        if (libgtk3_handle) {
            if (PTR_TYPE_DLSYM(libgtk3_handle, gtk_dialog_get_type) &&
                PTR_TYPE_DLSYM(libgtk3_handle, gtk_message_dialog_new) &&
                PTR_TYPE_DLSYM(libgtk3_handle, gtk_dialog_add_button) &&
                PTR_TYPE_DLSYM(libgtk3_handle, gtk_dialog_run) &&
                PTR_TYPE_DLSYM(libgtk3_handle, gtk_widget_destroy)) avail = true;
        }
    }
    return avail;
}

#if TARGET_OS_IPHONE
UIViewController* find_top_controller(UIViewController *root)
{
    if ([root isKindOfClass:[UINavigationController class]]) {
        return find_top_controller(((UINavigationController *)root).topViewController);
    } else if ([root isKindOfClass:[UITabBarController class]]) {
        return find_top_controller(((UITabBarController *)root).selectedViewController);
    } else if (root.presentedViewController != nil) {
        return find_top_controller(root.presentedViewController);
    }
    return root;
}
#endif

const char *L_OK;
const char *L_FAILED;
const char *L_ERR;
const char *L_NONE;
const char *L_MA;
const char *L_MAH;
const char *L_MV;
/* This function is for speed up PO generation */
void init_common_text(void) {
    static bool done = false;
    if (!done) {
        L_OK = _C("OK");
        L_FAILED = _C("Failed");
        L_ERR = _C("Error");
        L_NONE = _C("None"); // Consider "None" => "N/A"
        L_MA = _C("mA");
        L_MAH = _C("mAh");
        L_MV = _C("mV");
    }
    done = true;
}

/* Alert for multiple scene */
/* TODO: Check if program running under SSH */
bool show_alert(const char *title, const char *message, const char *button) {
    /* Please avoid using this, my knowledge does not support me to make it working well */
    /* The original design is something like getchar(), execute only after button pressed
        show_alert("Notice", "Will open URL", "OK");
        open_url(url);
       but waiting for button eventually led to UI freeze.
     */
    /* The alternative way is the following show_alert_async(), which use like:
        show_alert_async("Notice", "Will open URL", "OK", ^(bool result) {
            open_url(url);
        });
     */
    __block BOOL result = false;
    __block BOOL done = NO;

    // Call the asynchronous alert.
    show_alert_async(title, message, button, ^(bool res) {
        result = res;
        done = YES;
    });

    return result;
}

@interface OBSetupAssistantFinishedController:UIViewController
- (instancetype)initWithTitle:(NSString *)title detailText:(NSString *)detail;
- (void)setInstructionalText:(NSString *)text;
@end

void show_fatal_overlay_async(const char *title, const char *message) {
	show_alert_async(title,message,"ok",^(bool idk){
		app_exit();
	});
	return;
	NSBundle *obkit=[NSBundle bundleWithPath:@"/System/Library/PrivateFrameworks/OnBoardingKit.framework"];
	if(![obkit load]) {
		show_alert_async(title,message,"ok",^(bool idk){
			app_exit();
		});
	}
	OBSetupAssistantFinishedController *safc=[[[obkit classNamed:@"OBSetupAssistantFinishedController"] alloc] initWithTitle:[NSString stringWithUTF8String:title] detailText:[NSString stringWithUTF8String:message]];
	if(!safc) {
		show_alert_async(title,message,"ok",^(bool idk){
			app_exit();
		});
	}
	[safc setInstructionalText:@"Swipe up to exit"];
	UIWindow *keyWindow=[[UIApplication sharedApplication] keyWindow];
	keyWindow.rootViewController=safc;
}

void show_alert_async(const char *title, const char *message, const char *button, void (^completion)(bool result)) {
    DBGLOG(@"show_alert called: [%s], [%s], [%s]", title, message, button);

    /* Alert in GTK+ if under Xfce / GNOME */
    /* this check may not accurate */
    if (gtk_available() && getenv("DISPLAY")) {
        GtkWidget *dialog = gtk_message_dialog_new_ptr(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, "%s", message);
        gtk_dialog_add_button_ptr(GTK_DIALOG(dialog), button, GTK_RESPONSE_CANCEL);
        // gtk_dialog_add_button(GTK_DIALOG(dialog), "OK", GTK_RESPONSE_ACCEPT);
        
        int response = gtk_dialog_run_ptr(GTK_DIALOG(dialog));
        gtk_widget_destroy_ptr(dialog);
        if (completion) completion(response == GTK_RESPONSE_CANCEL);
    }
#if TARGET_OS_IPHONE
    /* Alert using system UIAlert */
    if (@available(iOS 9.0, *)) {
        /* dynamically allocated char* cannot survive under dispatch block */
        NSString *nstitle = [NSString stringWithUTF8String:title];
        NSString *nsmessage = [NSString stringWithUTF8String:message];
        NSString *nsbutton = [NSString stringWithUTF8String:button];
        // Use UIAlertController if iOS 10 or later
        dispatch_async(dispatch_get_main_queue(), ^{
        	extern UIWindow *gWindow;
            UIWindow *keyWindow = gWindow;
            
            UIViewController *topController = find_top_controller(keyWindow.rootViewController);

            UIAlertController *alert = [UIAlertController alertControllerWithTitle:nstitle message:nsmessage preferredStyle:UIAlertControllerStyleAlert];
            UIAlertAction *action = [UIAlertAction actionWithTitle:nsbutton style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
                if (completion) completion(true);
            }];
            [alert addAction:action];
            
            if (topController.presentedViewController) {
                [topController.presentedViewController presentViewController:alert animated:YES completion:nil];
            } else {
                [topController presentViewController:alert animated:YES completion:nil];
            }
        });
    } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        // Use UIAlertView for iOS 9 or earlier
        /* TODO: Add a delegate */
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:[NSString stringWithUTF8String:title]
                                                        message:[NSString stringWithUTF8String:message]
                                                       delegate:nil
                                              cancelButtonTitle:[NSString stringWithUTF8String:button]
                                              otherButtonTitles:@"OK", nil];
        [alert show];
        /* TODO: check button */
        if (completion) completion(true);
#pragma clang diagnostic pop
    }
#elif TARGET_OS_OSX
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:[NSString stringWithUTF8String:title]];
    [alert setInformativeText:[NSString stringWithUTF8String:message]];
    [alert addButtonWithTitle:[NSString stringWithUTF8String:button]];
    [alert runModal];
    /* TODO: check button */
    if (completion) completion(true);
#endif
}

void app_exit(void) {
    if (is_carbon()) {
#if TARGET_OS_IOS
        /* Play an animation that back to homescreen, then exit app by sceneDidEnterBackground: */
        extern void ios_app_exit(void);
        ios_app_exit();
#endif
#if TARGET_OS_OSX
        /* OSX specific App exit logic (why not just exit(0)?) */
        @autoreleasepool {
            id<NSApplicationDelegate> delegate = [NSApp delegate];
            if (delegate && [delegate respondsToSelector:@selector(applicationShouldTerminate:)]) {
                NSApplicationTerminateReply reply = [delegate applicationShouldTerminate:app];
                if (reply == NSTerminateCancel) {
                    return false;
                }
            }
            [app terminate:nil];
        }
#endif
    } else {
        // TODO: CLI & X11 logic
    }

    // Fallback to C exit
    exit(0);
}

/* UIApplicationMain/NSApplicationMain only works when App launched with NSBundle */
/* FIXME: NSBundle still exists if with Info.plist, we need a better detection */
bool is_carbon(void) {
    return ([NSBundle mainBundle] && getenv("XPC_SERVICE_NAME"));
}

void open_url(const char *url) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    /* TODO: open url inside DE */
#if TARGET_OS_IPHONE
    NSString *urlStr = [NSString stringWithUTF8String:url];
    NSURL *URL = [NSURL URLWithString:urlStr];
    if (URL) {
        // Ensure URL opening is done on the main thread.
        dispatch_async(dispatch_get_main_queue(), ^{
            [[UIApplication sharedApplication] openURL:URL];
        });
    }
#endif
#if TARGET_OS_OSX
    NSString *urlStr = [NSString stringWithUTF8String:url];
    NSURL *URL = [NSURL URLWithString:urlStr];
    if (URL) {
        [[NSWorkspace sharedWorkspace] openURL:URL];
    }
#endif
#pragma clang diagnostic pop
}

bool match_regex(const char *string, const char *pattern) {
    regex_t regex;
    if (regcomp(&regex, pattern, REG_EXTENDED) != 0)
        return 0;
    int result = regexec(&regex, string, 0, NULL, 0);
    regfree(&regex);
    return result == 0;
}

// For iOS 12 or ealier, we generate image directly from 'SF-Pro-Display-Regular.otf'
UIImage *imageForSFProGlyph(NSString *glyph, NSString *fontName, CGFloat fontSize, UIColor *tintColor) {
    UIFont *font = [UIFont fontWithName:fontName size:fontSize]
                   ?: [UIFont systemFontOfSize:fontSize];
    NSDictionary *attrs = @{
        NSFontAttributeName: font,
        NSForegroundColorAttributeName: tintColor
    };

    CGSize sz = [glyph sizeWithAttributes:attrs];
    UIGraphicsBeginImageContextWithOptions(sz, NO, 0);
    [glyph drawAtPoint:CGPointZero withAttributes:attrs];
    UIImage *img = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();

    // template so tintColor still applies if you change it later
    return [img imageWithRenderingMode:UIImageRenderingModeAlwaysTemplate];
}

// HTML operations broken on Rosetta Sims pre iOS 14
int is_rosetta(void) {
    int ret = 0;
    size_t size = sizeof(ret);
    if (sysctlbyname("sysctl.proc_translated", &ret, &size, NULL, 0) == -1) {
        if (errno == ENOENT) return 0;
        return -1;
    }
    return ret;
}

/* Consider use NSDefaults instead of file */
const char *lang_cfg_file(void) {
    char *home = getenv("HOME");
    if (match_regex(home, IOS_CONTAINER_FMT) || match_regex(home, MAC_CONTAINER_FMT)) {
        /* iOS/macOS sandboxed */
    } else if (match_regex(home, SIM_CONTAINER_FMT)) {
        /* Simulator sandboxed */
    } else {
        /* Unknown/Unsandboxed */
        return "/.config/Battman_LANG";
    }
    return "/Library/_LANG";
}

int open_lang_override(int flags,int mode) {
	const char *homedir = getenv("HOME");
	if(!homedir)
		return 0;
	char *langoverride_fn = malloc(strlen(homedir) + 20);
	stpcpy(stpcpy(langoverride_fn, homedir), lang_cfg_file());
	int fd = open(langoverride_fn, flags, mode);
	free(langoverride_fn);
	return fd;
}

static int _preferred_language_code = -1;

void preferred_language_code_clear(void) {
	_preferred_language_code = -1;
}

int preferred_language_code() {
	if (_preferred_language_code!=-1)
		return _preferred_language_code;
	int lfd = open_lang_override(O_RDONLY,0);
	if (lfd == -1) {
		char *lang = preferred_language();
		if (*(uint16_t*)lang == 0x6e7a) {
			_preferred_language_code = 1;
		} else {
			_preferred_language_code = 0;
		}
		free(lang);
		return _preferred_language_code;
	}
	int ret;
	if (read(lfd, &ret, 4) != 4) {
		close(lfd);
		_preferred_language_code = 0;
		return 0;
	}
	close(lfd);
	_preferred_language_code = ret;
	return ret;
}

const char *target_type(void) {
    static char *buf = NULL;

    if (buf != NULL)
        return buf;

    const char *name = "hw.targettype";
    size_t len = 0;

    if (sysctlbyname(name, NULL, &len, NULL, 0) != 0 || len == 0)
        return NULL;

    buf = malloc(len + 1);
    if (!buf)
        return NULL;

    if (sysctlbyname(name, buf, &len, NULL, 0) != 0) {
        free(buf);
        buf = NULL;
        return buf;
    }

    buf[len] = '\0';
    return buf;
}
