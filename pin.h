#ifndef pin_h
#define pin_h

#include "obj.h"

// pin: data space provider for same size items (use sac)

typedef struct pin   pin;
typedef struct pin_a pin_a;
typedef struct pin_l pin_l;

typedef int    pin_comp_f(pin* o, data_t a, data_t b, parg_t e CAN_NULL);
typedef size_t pin_loop_f(pin* o, pin_l* p);

// pin_args: arguments for creating pin items

struct pin_a {

    size_t FOR_PASS s_item;  // data item size, 0 -> data_s
    size_t          t_item;  // data item type, 0 -> unknown type
    size_t FOR_PASS s_head;  // data head size

};

struct pin_l {

    bool      t;  // set true will use index on start or terminate loop
    parg_t    e;  // extra argument for loop function
    size_t    i;  // current index (can set new value as next index)
    cap*      d;  // current data

};

extern size_t pin_size(void);
extern size_t pin_type(void);

extern size_t pin_pass(parg_t a);  // check arguments and return required object data size

extern pobj_t pin_init(pbuf_t b CAN_NULL, parg_t a CAN_NULL, pobj_t h CAN_NULL);
extern void   pin_fini(pobj_t o);

extern bool   pin_full     (pin* o);
extern bool   pin_exist    (pin* o, size_t n);
extern size_t pin_count    (pin* o);

extern size_t pin_data_size(pin* o);
extern size_t pin_data_type(pin* o);
extern void*  pin_temp     (pin* o);

inline pin*   new_pin  (void);
extern pin*   new_pin_c(size_t s_item CAN_ZERO, size_t t_item CAN_ZERO, size_t s_head CAN_ZERO, pobj_t p_hold CAN_NULL);
inline pin*   new_pin_s(size_t s_item CAN_ZERO);

inline void   del_pin(pin* o, pobj_t caller CAN_NULL);

// functions

// pin_ask:  get one new item with data and extra head size, and set pin id to n (if n is given)
// pin_add:  get one new item
// pin_loop: return sum value of pin_loop_f function

extern cap*   pin_ask (pin* o, size_t* n CAN_NULL, size_t s CAN_ZERO, size_t e CAN_ZERO);
inline cap*   pin_add (pin* o, size_t* n CAN_NULL);

extern cap*   pin_get (pin* o, size_t n);
extern bool   pin_swap(pin* o, size_t a, size_t b);

extern void   pin_sort(pin* o, pin_comp_f f, parg_t e CAN_NULL);
extern size_t pin_loop(pin* o, pin_loop_f f, pin_l* p CAN_NULL);
extern size_t pin_hoop(pin* o, pin_loop_f f, pin_l* p CAN_NULL);  // loop backward

inline bool   pin_check    (pin* o, cap* d);
inline size_t pin_head_size(pin* o, cap* d);
inline pbuf_t pin_head_data(pin* o, cap* d);

// inline functions

inline pin*   new_pin(void) { return (pin*)pin_init(NULL, NULL, NULL); }

inline pin*   new_pin_s(size_t s_item) { return new_pin_c(s_item, 0, 0, NULL); }

inline void   del_pin(pin* o, pobj_t caller) { del_obj(pin); }

inline cap*   pin_add(pin* o, size_t* n) { return pin_ask(o, n, 0, 0); }

inline bool   pin_check    (pin* o, cap* d) { return d && cap_hold(d) == o; }
inline size_t pin_head_size(pin* o, cap* d) { return bag_head_size(pobj_pick(o), d); }
inline pbuf_t pin_head_data(pin* o, cap* d) { return bag_head_data(pobj_pick(o), d); }

#endif /* pin_h */
