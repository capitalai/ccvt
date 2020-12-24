#ifndef str_h
#define str_h

#include "bag.h"

// str is created from str_pool, drop self when finish
// finish str_pool will also drop all str from the str_pool if no holder

typedef struct str      str;
typedef struct str_pool str_pool;

extern size_t str_def_size    (size_t new_size);
extern size_t str_add_def_rate(size_t new_rate);

extern str_pool* str_pool_init(size_t s CAN_ZERO, pobj_t h CAN_NULL);  // s = base buffer size, 0 is default size
extern void      str_pool_fini(str_pool* p);                           // finish str from pool if no holder

extern str*   str_init(str_pool* p, text_t source CAN_NULL);  // buffer size = max(base buffer size, aligned source length)
extern void   str_fini(str* s);

extern size_t str_size  (str* s);
extern char*  str_data  (str* s);
extern size_t str_length(str* s);

extern void   str_refresh(str* s);  // refresh string length
extern void   str_clear  (str* s);

extern str*   str_ask(str* s, size_t n);                              // ask string buffer size (total size)
extern str*   str_add(str* s, text_t d CAN_NULL, size_t n CAN_ZERO);  // add string, n = 0 => SIZE_MAX
extern str*   str_set(str* s, text_t d CAN_NULL, size_t n CAN_ZERO);  // set string, n = 0 => SIZE_MAX

#endif /* str_h */
