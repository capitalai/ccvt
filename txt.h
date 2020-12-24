#ifndef txt_h
#define txt_h

#include "obj.h"
#include "str.h"

// txt: string array

typedef struct txt   txt;
typedef struct txt_l txt_l;

typedef int    txt_comp_f(txt* o, str* a, str* b, parg_t e CAN_NULL);
typedef size_t txt_loop_f(txt* o, txt_l* p);

struct txt_l {

    bool      t;  // set true will use index on start or terminate loop
    parg_t    e;  // extra argument for loop function
    size_t    i;  // current index (can set new value as next index)
    str*      d;  // current str

};

extern size_t txt_size(void);
extern size_t txt_type(void);

extern size_t txt_pass(parg_t a);  // check arguments and return required object data size

extern pobj_t txt_init(pbuf_t b CAN_NULL, parg_t a CAN_NULL, pobj_t h CAN_NULL);
extern void   txt_fini(pobj_t o);

extern bool   txt_full (txt* o);
extern bool   txt_exist(txt* o, size_t n);
extern size_t txt_count(txt* o);

inline txt*   new_txt(void);
inline void   del_txt(txt* o, pobj_t caller CAN_NULL);

// functions

extern str*   txt_add (txt* o, size_t* n CAN_NULL, text_t t);
extern str*   txt_get (txt* o, size_t  n);
extern bool   txt_swap(txt* o, size_t  a, size_t b);

extern void   txt_sort(txt* o, txt_comp_f f, parg_t e CAN_NULL);
extern size_t txt_loop(txt* o, txt_loop_f f, txt_l* p CAN_NULL);
extern size_t txt_hoop(txt* o, txt_loop_f f, txt_l* p CAN_NULL);  // loop backward

// inline functions

inline txt*   new_txt(void) { return (txt*)txt_init(NULL, NULL, NULL); }

inline void   del_txt(txt* o, pobj_t caller) { del_obj(txt); }

#endif /* txt_h */
