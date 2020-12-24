#ifndef seq_h
#define seq_h

#include "obj.h"

// seq: sequence data, can remove items, no index, data size and head size are fixed.

typedef struct seq   seq;
typedef struct seq_a seq_a;
typedef struct seq_l seq_l;

typedef bool   seq_init_f(seq* o, cap* d);
typedef void   seq_fini_f(seq* o, cap* d);
typedef size_t seq_loop_f(seq* o, seq_l* p);

struct seq_a {

    size_t      FOR_PASS s_item;  // data item size, 0 -> data_s
    size_t               t_item;  // data item type, 0 -> unknown type
    size_t      FOR_PASS s_head;  // data head size
    seq_init_f*          f_init;  // new item
    seq_fini_f*          f_fini;  // delete item

};

struct seq_l {

    parg_t    e;  // extra argument for loop function
    cap*      d;  // current data, set new value will be used on next loop, set NULL will terminate looping

};

extern size_t seq_size(void);
extern size_t seq_type(void);
extern size_t seq_pass(parg_t a);  // check arguments and return required object data size

extern pobj_t seq_init(pbuf_t b CAN_NULL, parg_t a CAN_NULL, pobj_t h CAN_NULL);
extern void   seq_fini(pobj_t x);

extern bool   seq_full     (seq* o);
extern size_t seq_count    (seq* o);

extern size_t seq_data_size(seq* o);
extern size_t seq_data_type(seq* o);

inline seq*   new_seq  (void);

extern seq*   new_seq_c(size_t s_item CAN_ZERO, 
                        size_t t_item CAN_ZERO, 
                        size_t s_head CAN_ZERO, 
                        seq_init_f* f_init CAN_NULL, 
                        seq_fini_f* f_fini CAN_NULL, 
                        pobj_t p_hold CAN_NULL);

inline seq*   new_seq_s(size_t s_item CAN_ZERO);

inline void   del_seq(seq* o, pobj_t caller CAN_NULL);

// functions

// seq_add:  get one new item
// seq_loop: return sum value of seq_loop_f function

extern cap*   seq_add (seq* o);

extern bool   seq_del (seq* o, cap* d);
extern cap*   seq_get (seq* o, cap* d);  // return NULL if deleted

extern bool   seq_put (seq* o, cap* a, cap* d);  // move d after  a
extern bool   seq_ins (seq* o, cap* b, cap* d);  // move d before b

extern cap*   seq_head(seq* o);
extern cap*   seq_tail(seq* o);

extern cap*   seq_prev(seq* o, cap* d);
extern cap*   seq_next(seq* o, cap* d);

extern size_t seq_loop(seq* o, seq_loop_f f, cap* start CAN_NULL, cap* stop CAN_NULL, parg_t extra CAN_NULL);
extern size_t seq_hoop(seq* o, seq_loop_f f, cap* start CAN_NULL, cap* stop CAN_NULL, parg_t extra CAN_NULL);  // loop backward

extern size_t seq_head_size(seq* o, cap* d);
extern pbuf_t seq_head_data(seq* o, cap* d);

extern bool   seq_check(seq* o, cap* d);  // check d is this seq item

// inline functions

inline seq*   new_seq(void) { return (seq*)seq_init(NULL, NULL, NULL); }

inline seq*   new_seq_s(size_t s_item) { return new_seq_c(s_item, 0, 0, NULL, NULL, NULL); }

inline void   del_seq(seq* o, pobj_t caller) { del_obj(seq); }

#endif /* seq_h */
