//
//  intlextern.h
//  Battman
//
//  Created by Torrekie on 2025/1/21.
//

#ifndef intlextern_h
#define intlextern_h

#define _C(x) cond_localize_c(x)

#if __has_include(<libintl.h>)
#include <libintl.h>
#else

#include "main.h"
#include <os/base.h>

__BEGIN_DECLS

char *gettext (const char *__msgid);
char *libintl_gettext (const char *__msgid);
char *textdomain (const char *__domainname);
char *libintl_textdomain (const char *__domainname);
char *bindtextdomain (const char *__domainname, const char *__dirname);
char *libintl_bindtextdomain (const char *__domainname, const char *__dirname);
char *bind_textdomain_codeset (const char *__domainname, const char *__codeset);
char *libintl_bind_textdomain_codeset (const char *__domainname, const char *__codeset);

__END_DECLS

#endif /* __has_include */

__BEGIN_DECLS

static const char *libintl_paths[] = {
    "/usr/lib/libintl.8.dylib",
    "/usr/local/lib/libintl.8.dylib",
    "/var/jb/usr/lib/libintl.8.dylib",
    "/var/jb/usr/local/lib/libintl.8.dylib",
    "/opt/local/lib/libintl.8.dylib",
    "/opt/homebrew/lib/libintl.8.dylib",
    "libintl.8.dylib",
    "libintl.dylib",
    "intl.framework/intl",
    NULL,
};

WEAK_LINK_FORCE_IMPORT(gettext);
WEAK_LINK_FORCE_IMPORT(libintl_gettext);
WEAK_LINK_FORCE_IMPORT(bindtextdomain);
WEAK_LINK_FORCE_IMPORT(libintl_bindtextdomain);
WEAK_LINK_FORCE_IMPORT(textdomain);
WEAK_LINK_FORCE_IMPORT(libintl_textdomain);
WEAK_LINK_FORCE_IMPORT(bind_textdomain_codeset);
WEAK_LINK_FORCE_IMPORT(libintl_bind_textdomain_codeset);

#if 0
static char * (*gettext_ptr) (const char *__msgid);
static char * (*textdomain_ptr) (const char *__domainname);
static char * (*bindtextdomain_ptr) (const char *__domainname, const char *__dirname);
#else
extern typeof(gettext) *gettext_ptr;
extern typeof(textdomain) *textdomain_ptr;
extern typeof(bindtextdomain) *bindtextdomain_ptr;
extern typeof(bind_textdomain_codeset) *bind_textdomain_codeset_ptr;
#endif

__END_DECLS

#endif /* intlextern_h */
