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
#ifdef __OBJC__
#include <Foundation/Foundation.h>
#endif

#if __has_include(<SoftLinking/WeakLinking.h>)
#include <SoftLinking/WeakLinking.h>
#else
#define WEAK_LINK_FORCE_IMPORT(sym) extern __attribute__((weak_import)) __typeof__(sym) sym
#endif

#ifdef _
#undef _
#endif

#define _(x) cond_localize(x)

#ifndef _ID_
#define _ID_(x) (x)
#endif

#ifndef BATTMAN_TEXTDOMAIN
#define BATTMAN_TEXTDOMAIN "battman"
#endif

__BEGIN_DECLS

#ifdef __OBJC__
NSString *cond_localize(const char *str);
#endif

const char *cond_localize_c(const char *str);

__END_DECLS

#endif /* main_h */
