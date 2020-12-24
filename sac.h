#ifndef sac_h
#define sac_h

#include "bag.h"

// sac: simple growable storage (use lot)

// sac_init: head_size is extended head size for user

extern bag*   sac_init  (void);
extern bag*   sac_init_c(size_t s_pack);

extern size_t sac_pack_size(bag* b);

extern size_t sac_type(void);

inline bool is_sac(bag* b) { return bag_type(b) == sac_type(); }

// sac_pack_def_size(): set new default sac pack size
//                      if new_size = 0, just get current default sac pack size  

extern size_t sac_pack_def_size(size_t new_size);  

#endif /* sac_h */
