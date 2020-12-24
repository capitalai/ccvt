#include "lib.h"
#include "bag.h"
#include "sac.h"

static bag_init_f* def_bag_init = sac_init;

void   set_bag_init(bag_init_f f) { def_bag_init = f; }

bag*   bag_init(void) { return def_bag_init(); }

extern inline cap*   bag_take(bag* b, size_t s, size_t h);
extern inline bool   bag_drop(bag* b, cap*   p);
extern inline void   bag_fini(bag* b);
extern inline size_t bag_type(bag* b);

extern inline void*  bag_add_bin(bag* b, size_t s, data_t d);

void* bag_add_str(bag* b, size_t s, text_t t) {

    size_t s1; if(t) { s1 = text_size(t, 0); if(s1 > s) s = s1; } else s1 = s;

    return data_copy(cap_data(bag_take(b, s, 0)), t, s1);

}

extern inline pbuf_t cap_data(cap* p);
extern inline size_t cap_size(cap* p);
extern inline pbag_t cap_pick(cap* p);

extern inline void   cap_set_hold(cap* p, pobj_t o);
extern inline pobj_t cap_hold(cap* p);

extern inline size_t bag_head_size(bag* b, cap* p);
extern inline pbuf_t bag_head_data(bag* b, cap* p);
