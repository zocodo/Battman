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

#ifndef _
#define _(x) (@x)
#endif

#ifndef _ID_
#define _ID_(x) (x)
#endif

#ifndef BATTMAN_INTL
#define BATTMAN_INTL "battman"
#endif

__BEGIN_DECLS

extern CFStringRef localization_arr[];
NSString *cond_localize(int localize_id);
NSString *cond_localize_old(const char *);

__END_DECLS

#endif /* main_h */
