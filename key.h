#ifndef key_h
#define key_h

#include "obj.h"

// key: sequence and unique text key varibles, can remove

typedef struct key   key;
typedef struct key_a key_a;
typedef struct key_l key_l;

typedef bool   key_init_f(key* o, cap* d);
typedef void   key_fini_f(key* o, cap* d);
typedef size_t key_loop_f(key* o, key_l* p);

struct key_a {

    size_t      s_item;  // data item size, 0 -> data_s
    size_t      t_item;  // data item type, 0 -> unknown type
    key_init_f* f_init;  // new item
    key_fini_f* f_fini;  // delete item

};

struct key_l {

    parg_t    e;  // extra argument for loop function
    cap*      d;  // current data, set new value will be used on next loop, set NULL will terminate looping

};

extern size_t key_size(void);
extern size_t key_type(void);
extern size_t key_pass(parg_t a);  // check arguments and return required object data size

extern pobj_t key_init(pbuf_t b CAN_NULL, parg_t a CAN_NULL, pobj_t h CAN_NULL);
extern void   key_fini(pobj_t x);

extern bool   key_full     (key* o);
extern size_t key_count    (key* o);

extern size_t key_data_size(key* o);
extern size_t key_data_type(key* o);

inline key*   new_key  (void);

extern key*   new_key_c(size_t s_item CAN_ZERO, 
                        size_t t_item CAN_ZERO, 
                        key_init_f* f_init CAN_NULL, 
                        key_fini_f* f_fini CAN_NULL, 
                        pobj_t p_hold CAN_NULL);

inline key*   new_key_s(size_t s_item CAN_ZERO);

inline void   del_key(key* o, pobj_t caller CAN_NULL);

extern cap*   key_get (key* o, text_t v);
extern cap*   key_add (key* o, text_t v);  // new data node then add key, use key_get() before if too much same key names

extern bool   key_del (key* o, text_t k);

extern bool   key_put (key* o, text_t a, text_t d);  // move d after  a
extern bool   key_ins (key* o, text_t b, text_t d);  // move d before b

extern cap*   key_head(key* o);
extern cap*   key_tail(key* o);

extern cap*   key_next(key* o, cap* p);
extern cap*   key_prev(key* o, cap* p);

extern text_t key_name(key* o, cap* p);

extern size_t key_loop(key* o, key_loop_f f, text_t start CAN_NULL, text_t stop CAN_NULL, parg_t extra CAN_NULL);
extern size_t key_hoop(key* o, key_loop_f f, text_t start CAN_NULL, text_t stop CAN_NULL, parg_t extra CAN_NULL);

extern bool   key_check(key* o, cap* d);  // check d is this key item

// inline functions

inline key* new_key  (void)          { return new_key_c(0,      0, NULL, NULL, NULL); }
inline key* new_key_s(size_t s_item) { return new_key_c(s_item, 0, NULL, NULL, NULL); }

inline void del_key(key* o, pobj_t caller) { del_obj(key); }

#endif /* key_h */
