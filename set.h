#ifndef set_h
#define set_h

#include "pin.h"

// set: data collection with index (delete item will not free memory)

typedef struct set   set;
typedef struct set_a set_a;
typedef struct set_l set_l;

typedef uint_t set_hash_f(set* o, parg_t a);
typedef bool   set_find_f(set* o, parg_t a, cap*   b);
typedef cap*   set_make_f(pin* p, parg_t i, parg_t d);
typedef size_t set_loop_f(set* o, set_l* p);

// set_l: loop data

enum set_loop_mode {

    SET_LOOP_DEF,
    SET_LOOP_ALL,
    SET_LOOP_IDX,   // only loop with items which have same index
    SET_LOOP_DEL    // loop with removed items

};

typedef enum set_loop_mode set_loop_mode;

struct set_l {

    set_loop_mode m;
    parg_t        e;  // extra argument
    parg_t        i;  // current data index
    cap*          d;  // current data

};

// set_a: arguments for creating set items, no function to set default

enum set_add_mode {

    SET_ADD_LAST,     // remove old item with same index (default)
    SET_ADD_ONCE,     // fail when add item with same index
    SET_ADD_PUSH,     // can add item with same index and can get new data
    SET_ADD_LIST      // can add item with same index, and can loop with index as list

};

typedef enum set_add_mode set_add_mode;

enum set_index_type {

    SET_INDEX_TEXT  = 0,
    SET_INDEX_CHAR  = 1,
    SET_INDEX_UINT  = 2,
    SET_INDEX_LONG  = 3,
    SET_INDEX_FLOAT = 4

};

enum set_data_type {

    SET_DATA_TEXT = 0

};

typedef enum set_index_type set_index_type;
typedef enum set_data_type  set_data_type;

struct set_a {

    size_t                s_index;   // index size, 0 ~ 4 is special type index
    size_t       FOR_PASS s_data;    // data  size, 0 is text string
    set_add_mode          add_mode;

};

extern size_t set_size(void);
extern size_t set_type(void);
extern size_t set_pass(parg_t a);  // check arguments and return required object data size

extern pobj_t set_init(pbuf_t b CAN_NULL, parg_t a, pobj_t h CAN_NULL);
extern void   set_fini(pobj_t x);

extern size_t set_count(set* o);

inline set*   new_set  (size_t s_index CAN_ZERO, size_t s_data CAN_ZERO);
extern set*   new_set_c(size_t s_index CAN_ZERO, size_t s_data CAN_ZERO, set_add_mode add_mode, pobj_t p_hold CAN_NULL);

inline void   del_set  (set* o, pobj_t caller CAN_NULL);

// functions

extern bool   set_add  (set* o, parg_t i, parg_t d);  // add data to set, fail when index existed
extern cap*   set_get  (set* o, parg_t i);
extern bool   set_del  (set* o, parg_t i);

extern size_t set_loop    (set* o, set_loop_f f, parg_t e);
extern size_t set_loop_all(set* o, set_loop_f f, parg_t e);
extern size_t set_loop_del(set* o, set_loop_f f, parg_t e);
extern size_t set_loop_idx(set* o, set_loop_f f, parg_t i, parg_t e);

// no set_hoop() because set is not ordered collection

// inline functions

inline set*   new_set(size_t s_index, size_t s_data) { return new_set_c(s_index, s_data, SET_ADD_LAST, NULL); }

inline void   del_set(set* o, pobj_t caller CAN_NULL) { del_obj(set); }

#endif /* set_h */
