//
//  gtkextern.h
//  Battman
//
//  Created by Torrekie on 2025/1/21.
//

#ifndef gtkextern_h
#define gtkextern_h

#if __has_include(<gtk/gtk.h>)
#include <gtk/gtk.h>
#else

#include <os/base.h>
#include "main.h"
#include "glibextern.h"

#define GTK_TYPE_DIALOG                  (gtk_dialog_get_type ())
#define GTK_DIALOG(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_DIALOG, GtkDialog))

__BEGIN_DECLS

struct _GtkWidget
{
    GInitiallyUnowned parent_instance;
    void *priv;
};

typedef struct _GtkWidget GtkWidget;

typedef enum
{
    GTK_DIALOG_MODAL                = 1 << 0,
    GTK_DIALOG_DESTROY_WITH_PARENT  = 1 << 1,
    GTK_DIALOG_USE_HEADER_BAR       = 1 << 2,
} GtkDialogFlags;

typedef enum
{
    GTK_MESSAGE_INFO,
    GTK_MESSAGE_WARNING,
    GTK_MESSAGE_QUESTION,
    GTK_MESSAGE_ERROR,
    GTK_MESSAGE_OTHER,
} GtkMessageType;

typedef enum
{
    GTK_BUTTONS_NONE,
    GTK_BUTTONS_OK,
    GTK_BUTTONS_CLOSE,
    GTK_BUTTONS_CANCEL,
    GTK_BUTTONS_YES_NO,
    GTK_BUTTONS_OK_CANCEL,
} GtkButtonsType;

typedef struct _GtkDialog GtkDialog;
typedef struct _GtkWindow GtkWindow;
typedef struct _GtkBin GtkBin;
typedef struct _GtkContainer GtkContainer;

struct _GtkContainer
{
    GtkWidget widget;
    void *priv;
};

struct _GtkBin
{
    GtkContainer container;
    void *priv;
};

struct _GtkWindow
{
    GtkBin bin;
    void *priv;
};

struct _GtkDialog
{
    GtkWindow window;
    void *priv;
};

typedef enum
{
    GTK_RESPONSE_NONE         = -1,
    GTK_RESPONSE_REJECT       = -2,
    GTK_RESPONSE_ACCEPT       = -3,
    GTK_RESPONSE_DELETE_EVENT = -4,
    GTK_RESPONSE_OK           = -5,
    GTK_RESPONSE_CANCEL       = -6,
    GTK_RESPONSE_CLOSE        = -7,
    GTK_RESPONSE_YES          = -8,
    GTK_RESPONSE_NO           = -9,
    GTK_RESPONSE_APPLY        = -10,
    GTK_RESPONSE_HELP         = -11,
} GtkResponseType;

GType gtk_dialog_get_type(void);

GtkWidget* gtk_message_dialog_new(GtkWindow *window, GtkDialogFlags flags, GtkMessageType type, GtkButtonsType buttons, const char *message_format, ...);
GtkWidget* gtk_dialog_add_button(GtkDialog *dialog, const char *button_text, int response_id);
int gtk_dialog_run(GtkDialog *dialog);
void gtk_widget_destroy(GtkWidget *widget);

__END_DECLS

#endif /* __has_include */

__BEGIN_DECLS

static const char *libgtk3_paths[] = {
    "/usr/lib/libgtk-3.dylib",
    "/usr/local/lib/libgtk-3.dylib",
    "/var/jb/usr/lib/libgtk-3.dylib",
    "/var/jb/usr/local/lib/libgtk-3.dylib",
    "/opt/local/lib/libgtk-3.dylib",
    "/opt/homebrew/lib/libgtk-3.dylib",
    "libgtk-3.dylib",
    "GTK3.framework/GTK3",
    NULL,
};

WEAK_LINK_FORCE_IMPORT(gtk_dialog_get_type);
WEAK_LINK_FORCE_IMPORT(gtk_message_dialog_new);
WEAK_LINK_FORCE_IMPORT(gtk_dialog_add_button);
WEAK_LINK_FORCE_IMPORT(gtk_dialog_run);
WEAK_LINK_FORCE_IMPORT(gtk_widget_destroy);

extern typeof(gtk_dialog_get_type) *gtk_dialog_get_type_ptr;
extern typeof(gtk_message_dialog_new) *gtk_message_dialog_new_ptr;
extern typeof(gtk_dialog_add_button) *gtk_dialog_add_button_ptr;
extern typeof(gtk_dialog_run) *gtk_dialog_run_ptr;
extern typeof(gtk_widget_destroy) *gtk_widget_destroy_ptr;

__END_DECLS

#endif /* gtkextern_h */
