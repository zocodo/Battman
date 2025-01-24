#include <TargetConditionals.h>
#include <Foundation/Foundation.h>

#if TARGET_OS_IPHONE
#include <UIKit/UIKit.h>
extern UIWindow *gWindow;
#elif TARGET_OS_OSX
#include <Cocoa/Cocoa.h>
#endif

#include "common.h"
#include "intlextern.h"
#include "gtkextern.h"

/* Consider make this a standalone header */
#define SYM_EXIST(...) check_ptr(__VA_ARGS__)

#define PTR_TYPE_NAME(x, y) x ## _ptr = y
#define PTR_TYPE(x) PTR_TYPE_NAME(x, x)
#define PTR_TYPE_NAME_DLSYM(handle, y, x) ((y ## _ptr = (typeof(x)*)dlsym(handle, #x)) != NULL)
#define PTR_TYPE_DLSYM(handle, x) PTR_TYPE_NAME_DLSYM(handle, x, x)

/* https://developer.apple.com/library/archive/documentation/MacOSX/Conceptual/BPFrameworks/Concepts/WeakLinking.html
    Note: When checking for the existence of a symbol, you must explicitly compare it to NULL or nil in your code. You cannot use the negation operator ( ! ) to negate the address of the symbol.
*/
bool check_ptr(void* ptr1, ...) {
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

    if (SYM_EXIST(gettext, bindtextdomain, textdomain)) {
        avail = true;
        PTR_TYPE(gettext);
        PTR_TYPE(bindtextdomain);
        PTR_TYPE(textdomain);
    } else if (SYM_EXIST(libintl_gettext, libintl_bindtextdomain, libintl_textdomain)) {
        avail = true;
        PTR_TYPE_NAME(gettext, libintl_gettext);
        PTR_TYPE_NAME(bindtextdomain, libintl_bindtextdomain);
        PTR_TYPE_NAME(textdomain, libintl_textdomain);
    }

    if (!avail) {
        if (!libintl_handle) {
            for (int i = 0; libintl_paths[i] != NULL; i++) {
                libintl_handle = dlopen(libintl_paths[i], RTLD_LAZY);
                if (libintl_handle) break;
            }
        }

        if (libintl_handle) {
            if (PTR_TYPE_DLSYM(libintl_handle, gettext) &&
                PTR_TYPE_DLSYM(libintl_handle, bindtextdomain) &&
                PTR_TYPE_DLSYM(libintl_handle, textdomain)) avail = true;
            else if (PTR_TYPE_NAME_DLSYM(libintl_handle, gettext, libintl_gettext) &&
                     PTR_TYPE_NAME_DLSYM(libintl_handle, bindtextdomain, libintl_bindtextdomain) &&
                     PTR_TYPE_NAME_DLSYM(libintl_handle, textdomain, libintl_textdomain)) avail = true;
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

    if (SYM_EXIST(gtk_dialog_get_type, gtk_message_dialog_new, gtk_dialog_add_button, gtk_dialog_run, gtk_widget_destroy)) {
        avail = true;
        PTR_TYPE(gtk_dialog_get_type);
        PTR_TYPE(gtk_message_dialog_new);
        PTR_TYPE(gtk_dialog_add_button);
        PTR_TYPE(gtk_dialog_run);
        PTR_TYPE(gtk_widget_destroy);
    }

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

/* Alert for multiple scene */
/* TODO: Check if program running under SSH */
bool show_alert(const char *title, const char *message, const char *cancel_button_title) {

    /* Alert in GTK+ if under X Window */
    if (gtk_available() && getenv("DISPLAY")) {
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_NONE, "%s", message);
        gtk_dialog_add_button(GTK_DIALOG(dialog), cancel_button_title, GTK_RESPONSE_CANCEL);
        // gtk_dialog_add_button(GTK_DIALOG(dialog), "OK", GTK_RESPONSE_ACCEPT);
        
        int response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return response == GTK_RESPONSE_CANCEL;
    }
#if TARGET_OS_IPHONE
    /* Alert using system UIAlert */
    if (@available(iOS 9.0, *)) {
        // Use UIAlertController if iOS 10 or later
        UIAlertController *alertController = [UIAlertController alertControllerWithTitle:[NSString stringWithUTF8String:title]
                                                                                 message:[NSString stringWithUTF8String:message]
                                                                          preferredStyle:UIAlertControllerStyleAlert];
        UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:[NSString stringWithUTF8String:cancel_button_title] style:UIAlertActionStyleCancel handler:^(UIAlertAction * action) {}];
        [alertController addAction:cancelAction];

        UIViewController *rootViewController = gWindow.rootViewController;
        [rootViewController presentViewController:alertController animated:YES completion:nil];

        /* TODO: check button */
        return true;
    } else {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        // Use UIAlertView for iOS 9 or earlier
        /* TODO: Add a delegate */
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:[NSString stringWithUTF8String:title]
                                                        message:[NSString stringWithUTF8String:message]
                                                       delegate:nil
                                              cancelButtonTitle:[NSString stringWithUTF8String:cancel_button_title]
                                              otherButtonTitles:@"OK", nil];
        [alert show];
        /* TODO: check button */
        return true;
#pragma clang diagnostic pop
    }
#elif TARGET_OS_OSX
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:[NSString stringWithUTF8String:title]];
    [alert setInformativeText:[NSString stringWithUTF8String:message]];
    [alert addButtonWithTitle:[NSString stringWithUTF8String:cancel_button_title]];
    [alert runModal];
    /* TODO: check button */
    return true;
#endif
}
