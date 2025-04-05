//
//  license_check.h
//  Battman
//
//  Created by Torrekie on 2025/4/4.
//

#ifndef license_check_h
#define license_check_h

#include <stdbool.h>
#include <os/base.h>

__BEGIN_DECLS

bool has_accepted_terms(void);
void save_terms_acceptance(void);

__END_DECLS

#endif /* license_check_h */
