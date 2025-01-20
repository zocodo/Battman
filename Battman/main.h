//
//  main.h
//  Battman
//
//  Created by Torrekie on 2025/1/19.
//

#ifndef main_h
#define main_h

#include <libintl.h>
#include <Foundation/Foundation.h>

#ifndef _
#define _(x) cond_localize(x)
#endif

__BEGIN_DECLS

NSString *cond_localize(NSString *);

__END_DECLS

#endif /* main_h */
