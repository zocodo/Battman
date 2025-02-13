//
//  main.h
//  Battman
//
//  Created by Torrekie on 2025/1/19.
//

#ifndef main_h
#define main_h

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <Foundation/Foundation.h>

#if __has_include(<SoftLinking/WeakLinking.h>)
#include <SoftLinking/WeakLinking.h>
#else
#define WEAK_LINK_FORCE_IMPORT(sym) extern __attribute__((weak_import)) __typeof__(sym) sym
#endif

#ifdef _
#undef _
#endif

#ifndef USE_GETTEXT
#ifndef _
// TODO:
#define _(x) [NSString stringWithCString:x encoding:NSUTF8StringEncoding]
#endif
#else
#define _(x) cond_localize(x)
#endif

#ifndef _ID_
#define _ID_(x) (x)
#endif

#ifndef BATTMAN_TEXTDOMAIN
#define BATTMAN_TEXTDOMAIN "battman"
#endif

__BEGIN_DECLS

#ifndef USE_GETTEXT
NSString *cond_localize(unsigned long long localize_id);
#else
NSString *cond_localize(const char *str);
#endif

__END_DECLS

#endif /* main_h */
