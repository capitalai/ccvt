#ifndef pod_h
#define pod_h

#include "pin.h"

// pod: seq data with search and order

typedef struct pod   pod;
typedef struct pod_a pod_a;
typedef struct pod_l pod_l;
typedef struct pod_p pod_p;

// pod_find_f: return 0 -> found, > 0 bigger, < 0 smaller

typedef int    pod_comp_f(pod* o, cap*   a, cap* b);
typedef int    pod_find_f(pod* o, data_t a, cap* b);
typedef cap*   pod_push_f(pod* o, cap*   a, cap* b);
typedef cap*   pod_pull_f(pod* o, data_t a, cap* b);
typedef size_t pod_loop_f(pod* o, pod_l* p);
typedef void   pod_dump_f(cap* a, size_t d);

// pod_l: loop data

struct pod_l {

    parg_t    e;  // extra argument for loop function
    pod_p*    p;  // current position, set new value will be used on next loop, set NULL will terminate looping

};

// pod_a: arguments for creating pod items, no function to set default

struct pod_a {

    pod_comp_f* FOR_PASS f_comp;   // compare items
    pod_find_f* FOR_PASS f_find;   // find or delete item (return like f_comp)
    pod_push_f*          f_push;   // work if equal item existed
    pod_pull_f*          f_pull;   // work if equal item existed
    parg_t               p_bind;   // binding data for extended functions

};

// pod_p: position

struct pod_p {

    cap* r;  // current seq item, internal use only

};

extern size_t pod_size(void);
extern size_t pod_type(void);
extern size_t pod_pass(parg_t a);  // check arguments and return required object data size

extern pobj_t pod_init(pbuf_t b CAN_NULL, parg_t a, pobj_t h CAN_NULL);
extern void   pod_fini(pobj_t x);

extern size_t pod_count(pod* o);

inline pod*   new_pod  (pod_comp_f f_comp, pod_find_f f_find);
extern pod*   new_pod_c(pod_comp_f f_comp, pod_find_f f_find, pod_push_f* f_push CAN_NULL, pod_pull_f* f_pull CAN_NULL, parg_t p_bind CAN_NULL, pobj_t p_hold CAN_NULL);

inline void   del_pod(pod* o, pobj_t caller CAN_NULL);

// functions

extern pod_p* pod_head (pod* o, pod_p* p);
extern pod_p* pod_tail (pod* o, pod_p* p);

extern bool   pod_add  (pod* o, cap*   d, pod_p* p CAN_NULL);  // add data to sequence, fail when compared equal item existed
extern cap*   pod_get  (pod* o, pod_p* p);

extern bool   pod_prev (pod* o, pod_p* p);
extern bool   pod_next (pod* o, pod_p* p);

extern bool   pod_del  (pod* o, data_t a);

extern bool   pod_find     (pod* o, pod_p* p, data_t a);
extern int    pod_find_prev(pod* o, pod_p* p, data_t a);  // return -1 = not found, 0 = found, 1 = found prev
extern int    pod_find_next(pod* o, pod_p* p, data_t a);  // return -1 = not found, 0 = found, 1 = found next

extern size_t pod_loop(pod* o, pod_loop_f f, data_t start CAN_NULL, data_t stop CAN_NULL, parg_t extra CAN_NULL);
extern size_t pod_hoop(pod* o, pod_loop_f f, data_t start CAN_NULL, data_t stop CAN_NULL, parg_t extra CAN_NULL);  // loop backward

extern bool   pod_check(pod* o, pod_p* p);

extern void   pod_dump (pod* o, pod_dump_f f);  // for debugging only, travel pod internal tree with depth information

// inline functions

inline pod*   new_pod(pod_comp_f f_comp, pod_find_f f_find) { return new_pod_c(f_comp, f_find, NULL, NULL, NULL, NULL); }

inline void   del_pod(pod* o, pobj_t caller CAN_NULL) { del_obj(pod); }

#endif /* pod_h */
