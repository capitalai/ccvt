#ifndef obj_h
#define obj_h

#include "bag.h"

// obj: virtual object base class

typedef struct obj   obj;       // object, obj is a sec
typedef struct obj_a obj_a;     // arguments for object

// for function parameters

typedef pobj_t init_f(pbuf_t b, parg_t a, pobj_t h);  // initialize object with memory m, arguments a and holder h
typedef size_t pass_f(parg_t a);                      // check arguments and return required object data size

typedef void   obj_fini_f(pobj_t o);                      // for object destruction

struct obj_a {  // fill on object construction

    size_t      size;    // object size
    size_t      type;    // object type
    pobj_t      hold;    // object holder, can be null
    bag*        pick;    // memory allocator
    obj_fini_f* fini;    // this object destructor

};

struct obj {

    obj_a   d_conf;    // object configuration, fill on object initialization
    parg_t  p_bind;    // binding data for extended functions, can be null
    pobj_t  o_head;    // head of child object list
    pobj_t  o_prev;    // previous object
    pobj_t  o_next;    // next object
    size_t  o_stat;    // object status, reference only

};

extern bool   obj_set_bind(pobj_t o, parg_t p);  // assign binding data  to object if not assigned
extern bool   obj_set_stat(pobj_t o, size_t s);  // assign status to object

extern void   obj_push(pobj_t o, pobj_t p); 
extern bool   obj_pull(pobj_t o, pobj_t p);

extern void   obj_fini(pobj_t o);                // default function for finishing object

// object status:
//
//  OBJ_STAT_NONE: not initialized / fail to initialize / finished / corrupted
//  OBJ_STAT_HALT: error / finishing / can only get status
//  OBJ_STAT_FULL: can not add, set, write or send data / read only
//  OBJ_STAT_WORK: everything ok
//

#define OBJ_STAT_MASK 3

#define OBJ_STAT_NONE 0
#define OBJ_STAT_HALT 1
#define OBJ_STAT_FULL 2
#define OBJ_STAT_WORK 3

// macros for inline functions

#define POBJ(o)      ((obj*)o)

#define pobj_conf(o)  POBJ(o)->d_conf
#define pobj_size(o)  POBJ(o)->d_conf.size
#define pobj_type(o)  POBJ(o)->d_conf.type
#define pobj_hold(o)  POBJ(o)->d_conf.hold
#define pobj_pick(o)  POBJ(o)->d_conf.pick
#define pobj_work(o)  POBJ(o)->d_conf.work
#define pobj_fini(o)  POBJ(o)->d_conf.fini
#define pobj_bind(o)  POBJ(o)->p_bind
#define pobj_head(o)  POBJ(o)->o_head
#define pobj_prev(o)  POBJ(o)->o_prev
#define pobj_next(o)  POBJ(o)->o_next
#define pobj_stat(o)  POBJ(o)->o_stat

#define del_obj(t) if(caller == pobj_hold(o)) t ## _fini(o)

// inline functions

inline size_t obj_size(pobj_t o) { return pobj_size(o); }
inline size_t obj_type(pobj_t o) { return pobj_type(o); }

// macros for library

#ifdef CCL_CODE

// macros for object access

#define OBJ_STAT(o)          (pobj_stat(o) & OBJ_STAT_MASK)

#define obj_stat_none(o)     (OBJ_STAT(o) == OBJ_STAT_NONE)

#define obj_set_stat_none(o) obj_set_stat(o, OBJ_STAT_NONE)
#define obj_set_stat_halt(o) obj_set_stat(o, OBJ_STAT_HALT)

#endif // CCL_CODE

#endif /* obj_h */
