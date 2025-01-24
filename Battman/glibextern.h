//
//  glibextern.h
//  Battman
//
//  Created by Torrekie on 2025/1/21.
//

#ifndef glibextern_h
#define glibextern_h

#if __has_include(<glib-object.h>) && __has_include(<glib.h>)
#include <glib-object.h>
#include <glib.h>
#else

#include <os/base.h>

#define _G_TYPE_CIC(ip, gt, ct) ((ct*) (void *) ip)
#define G_TYPE_CHECK_INSTANCE_CAST(instance, g_type, c_type)    (_G_TYPE_CIC ((instance), (g_type), c_type))

__BEGIN_DECLS

typedef unsigned long gsize;
typedef gsize GType;

typedef struct _GTypeClass GTypeClass;
typedef struct _GTypeInstance GTypeInstance;
typedef struct _GData GData;

struct _GTypeInstance
{
    GTypeClass *g_class;
};

struct _GTypeClass
{
    GType g_type;
};

struct _GObject
{
    GTypeInstance g_type_instance;
    unsigned int ref_count;
    GData *quota;
};

typedef struct _GObject GInitiallyUnowned;

__END_DECLS

#endif /* __has_include */
#endif /* glibextern_h */
