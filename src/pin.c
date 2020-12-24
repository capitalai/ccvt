#include "lib.h"
#include "pin.h"
#include "sac.h"

#if SIZE_LEVEL == 1
#define PIN_NODE_PTRS           16
#define PIN_NODE_BITS            4
#define PIN_NODE_MASK            0xf
#define PIN_NODE_DEPTH           4
#else
#if SIZE_LEVEL == 2
#define PIN_NODE_PTRS           32
#define PIN_NODE_BITS            5
#define PIN_NODE_MASK            0x1f
#define PIN_NODE_DEPTH           7
#else
#define PIN_NODE_PTRS           64
#define PIN_NODE_BITS            6
#define PIN_NODE_MASK            0x3f
#define PIN_NODE_DEPTH          11
#endif
#endif

#define ppin(o) P(o, pin)

#define pin_take(s) bag_take(o->d_base.d_conf.pick, s, 0)
#define PIN_TAKE(t) BAG_TAKE(o->d_base.d_conf.pick, t, 0)

typedef struct _pin_node _pin_node;
typedef union  _pin_x _pin_x;
typedef struct _pin _pin;

union _pin_x {

    _pin_node* node;
    cap*       data;

};

struct _pin_node {

    _pin_x ptrs[PIN_NODE_PTRS];

};

struct _pin {

    // data structure

    _pin_node* root;
    _pin_node* node;

    // internal data

    void*      p_tmp;

    _pin_node* last;
    size_t     last_node_index;

    size_t     c_item;     // item count

    // object status

    size_t     max_count;  // maximum item count of current depth
    size_t     depth;
    bool       can_swap;

};

struct pin {  // pin is an obj

    obj    d_base;    // object data
    pin_a  d_args;    // object parameters
    _pin   d_core;    // core data

};

static pin_a def_pin_a = { 0, 0, 0 };
static pin_l def_pin_l = { false, NULL, 0, NULL };

size_t pin_size(void) { return sizeof(pin); }

size_t pin_type(void) { tid_make(pin); return tid(pin); }

size_t pin_pass(parg_t a) { 

    pin_a* p = a;

    cnz(p->s_item < SIZE_LIMIT && p->s_head < SIZE_LIMIT && (p->s_item + p->s_head) < SIZE_LIMIT);

    size_t s = p->s_item ? p->s_item : data_s;

    return e_size(pin, s);  // one added item
    
}

pobj_t pin_init(pbuf_t b, parg_t a, pobj_t h) {

    pin_a* p_args = a ? a : &def_pin_a;

    size_t p_size = pin_pass(p_args); czx(p_size);

    bag*   p_pick = h && pobj_pick(h) ? pobj_pick(h) : sac_init();

    char*  p_buf  = b ? b : cap_data(bag_take(p_pick, p_size, 0));

    pin*  o = (pin*)p_buf; p_buf += t_size(pin);  // object data block

    CNF.size = sizeof(pin);
    CNF.type = pin_type();
    CNF.hold = h;
    CNF.pick = p_pick;
    CNF.fini = pin_fini;

    OBJ.p_bind = NULL;
    OBJ.o_head = NULL;
    OBJ.o_next = NULL;
    OBJ.o_prev = NULL;

    ARG = *p_args;
    
    ifz(ARG.s_item) ARG.s_item = data_s;

    // all core fields should be set

    COR.root            = NULL;
    COR.node            = NULL;
    COR.p_tmp           = p_buf;  // temporary item
    COR.last            = NULL;
    COR.last_node_index = 0;
    COR.c_item          = 0;
    COR.max_count       = PIN_NODE_PTRS;
    COR.depth           = 0;
    COR.can_swap        = true;

    if(h) obj_push(h, o);  // push to hold object

    obj_set_stat(o, OBJ_STAT_WORK);

    return o;

}

bool   pin_full     (pin* o)           { return COR.c_item == SIZE_MAX; }
bool   pin_exist    (pin* o, size_t n) { return n < COR.c_item; }
size_t pin_count    (pin* o)           { return COR.c_item; }

size_t pin_data_size(pin* o) { return ARG.s_item; }
size_t pin_data_type(pin* o) { return ARG.t_item; }
void*  pin_temp     (pin* o) { return COR.p_tmp; }

extern inline pin* new_pin(void);

pin* new_pin_c(size_t s_item, size_t t_item, size_t s_head, void* p_hold) {

    pin_a p = { s_item, t_item, s_head };

    return pin_init(NULL, &p, p_hold);

}

extern inline pin* new_pin_s(size_t s_item);

extern inline void del_pin(pin* o, void* caller);

// functions

static _pin_node* _pin_now_node(pin* o, size_t n) {  // pin object, index number, create node pack

    size_t     a = n >> PIN_NODE_BITS;
    size_t     i = 0;
    size_t     c[PIN_NODE_DEPTH];
    _pin_node* p = COR.root;

    while(i < COR.depth) { c[i] = a & PIN_NODE_MASK; a >>= PIN_NODE_BITS; i++; }
    while(i > 0) { 
        
        i--; 
        
        ifx(p->ptrs[c[i]].node) {
            
            pbuf_t t = cap_data(PIN_TAKE(_pin_node));

            DATA_WIPE(t, _pin_node);

            p->ptrs[c[i]].node = t;

        }

        p = p->ptrs[c[i]].node;
        
    }

    return p;

}

// cap => head(cap head + sac head + pin.s_head + e) + data(s)

cap* pin_ask(pin* o, size_t* n, size_t s, size_t e) {

    ccx(pin_full(o));

    size_t c = COR.c_item & PIN_NODE_MASK;

    ifx(COR.root) {
        
        COR.root = COR.node = cap_data(PIN_TAKE(_pin_node));

        DATA_WIPE(COR.root, _pin_node);

    }
    else ifz(c) {  // add node

        if(COR.c_item == COR.max_count) {

            COR.max_count <<= PIN_NODE_BITS;  // overflow: can add till full
            COR.depth++;

            _pin_node* t           = COR.root;
            COR.root               = cap_data(PIN_TAKE(_pin_node)); DATA_WIPE(COR.root, _pin_node);
            COR.root->ptrs[0].node = t;

        }

        COR.node = _pin_now_node(o, COR.c_item);

    }

    size_t s1 = MAX(ARG.s_item, s); if(s1 > ARG.s_item) COR.can_swap = false;

    cap*    x = bag_take(CNF.pick, s1, ARG.s_head + e);

    COR.node->ptrs[c].data = x;

    if(n) *n = COR.c_item; 
    
    COR.c_item++;

    if(pin_full(o)) obj_set_stat(o, OBJ_STAT_FULL);

    cap_set_hold(x, o);

    return x;

}

cap* pin_get(pin* o, size_t n) {

    cnx(n < COR.c_item);

    ifz(COR.depth) return COR.root->ptrs[n].data;  // work when node(n) == 0

    size_t a = n >> PIN_NODE_BITS;
    size_t b = n  & PIN_NODE_MASK;

    if(COR.last && a == COR.last_node_index) return COR.last->ptrs[b].data;  // work when node(n) == node(last get item)

    _pin_node* p = COR.root;
    size_t     i = 0;
    size_t     c[PIN_NODE_DEPTH];

    while(i < COR.depth) { c[i] = a & PIN_NODE_MASK; a >>= PIN_NODE_BITS; i++; }

    while(i > 0) { i--; p = p->ptrs[c[i]].node; }

    COR.last            = p;
    COR.last_node_index = n >> PIN_NODE_BITS;

    return p->ptrs[b].data;

}

static bool _pin_swap(pin* o, size_t a, size_t b) {

    if(a == b) return true;

    void* pc = COR.p_tmp;
    void* pa = cap_data(pin_get(o, a)); cxf(pa);
    void* pb = cap_data(pin_get(o, b)); cxf(pb);

    data_copy(pc, pa, ARG.s_item);
    data_copy(pa, pb, ARG.s_item);
    data_copy(pb, pc, ARG.s_item);

    return true;

}

bool pin_swap(pin* o, size_t a, size_t b) {

    cnf(COR.can_swap);
    cnf(a < COR.c_item);
    cnf(b < COR.c_item);
    cxf(COR.p_tmp);

    return _pin_swap(o, a, b);

}

static void _pin_qsort(pin* o, pin_comp_f f, size_t a, size_t b, parg_t e) {

    if(a < b) {

        void* pb = cap_data(pin_get(o, b));  // use last one as partition item

        size_t c = a - 1;

        for(size_t i = a; i < b; i++) if(f(o, cap_data(pin_get(o, i)), pb, e) <= 0) _pin_swap(o, ++c, i);

        _pin_swap(o, ++c, b);

        if(c) _pin_qsort(o, f, a, c - 1, e);  // c may be 0
        _pin_qsort(o, f, c + 1, b, e);

    }

}

void pin_sort(pin* o, pin_comp_f f, parg_t e) { 
    
    cne(COR.c_item > 1);
    cne(COR.can_swap);
    cxe(COR.p_tmp);
    
    _pin_qsort(o, f, 0, COR.c_item - 1, e); 
    
}

size_t pin_loop(pin* o, pin_loop_f f, pin_l* p) {

    pin_l  d;

    ifx(p) { d = def_pin_l; p = &d; }

    size_t r = 0;
    size_t i;
    
    if(p->t) { i = p->i; p->t = false; } else i = 0;

    while(i < COR.c_item && p->t == false) {

        p->d = pin_get(o, i);

        r += f(o, p);

        if(i == p->i) {

            i++;

            p->i = i;

        }
        else i = p->i;

    }

    return r;

}

size_t pin_hoop(pin* o, pin_loop_f f, pin_l* p) {

    pin_l  d;

    ifx(p) { d = def_pin_l; p = &d; }

    size_t r = 0;
    size_t i; 
    
    if(p->t) { i = p->i; p->t = false; } else { czz(COR.c_item); i = COR.c_item - 1; }

    while(i < COR.c_item && p->t == false) {

        p->d = pin_get(o, i);

        r += f(o, p);

        if(i == p->i) {

            czb(i); 
            
            i--;

            p->i = i;

        }
        else i = p->i;

    }

    return r;

}

void pin_fini(pobj_t o) {

    cne(pobj_stat(o) != OBJ_STAT_NONE);

    obj_set_stat(o, OBJ_STAT_NONE);

    obj_fini(o);

}

extern inline pin*  new_pin  (void);
extern inline pin*  new_pin_s(size_t s_item);
extern inline void  del_pin  (pin* o, pobj_t caller);
extern inline cap*  pin_add  (pin* o, size_t* n);

extern inline size_t pin_head_size(pin* o, cap* d);
extern inline pbuf_t pin_head_data(pin* o, cap* d);

