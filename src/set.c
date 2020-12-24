#include "lib.h"
#include "set.h"
#include "pod.h"
#include "pin.h"
#include "sac.h"

#define pset_a(o) P(o, set_a)

typedef struct _set          _set;
typedef struct _set_work     _set_work;
typedef struct _set_node     _set_node;
typedef struct _set_hash     _set_hash;

typedef int _set_comp_f(data_t a, data_t b, size_t s);
typedef int _set_hash_f(pod*   x, cap*   a, cap*   b);
typedef int _set_find_f(pod*   x, data_t a, cap*   b);

struct _set_work {

    set_l*      d_loop;     // (loop) loop data
    set_loop_f* f_loop;     // (loop) user function for loop
    parg_t      del_index;  // (del)  index for del

};

union _set_hash_type {

    uint_t hash;
    char   c;
    uint_t u;
    long   l;
    float  f;

};

typedef union _set_hash_type _set_hash_type;

struct _set_node {

    _set_hash_type hash;
    cap*           hash_next;
    cap*           data_next;  // circular two-way list, link to self if one item on PUSH and LIST mode
    cap*           data_prev;

    char           index[];

};

struct _set {

    pin*         data;
    pod*         hash;
    size_t       count;
    _set_comp_f* f_comp;
    cap*         p_free;   // list of deleted nodes

};

struct set {  // pin is an obj

    obj       d_base;  // object data
    set_a     d_args;  // object parameters
    _set      d_core;  // core data
    _set_work d_work;  // data for functions

};

static int _set_comp_hash (pod* x, cap* a, cap* b);
static int _set_comp_char (pod* x, cap* a, cap* b);
static int _set_comp_uint (pod* x, cap* a, cap* b);
static int _set_comp_long (pod* x, cap* a, cap* b);
static int _set_comp_float(pod* x, cap* a, cap* b);

static int _set_find_hash (pod* x, data_t a, cap* b);
static int _set_find_char (pod* x, data_t a, cap* b);
static int _set_find_uint (pod* x, data_t a, cap* b);
static int _set_find_long (pod* x, data_t a, cap* b);
static int _set_find_float(pod* x, data_t a, cap* b);

static cap* _set_push(pod* x, cap*   a, cap* b);
static cap* _set_pull(pod* x, data_t a, cap* b);

static int _set_comp_text(data_t a, data_t b, size_t s);
static int _set_comp_data(data_t a, data_t b, size_t s);

size_t set_size(void) { return sizeof(set); }

size_t set_type(void) { tid_make(set); return tid(set); }

size_t set_pass(parg_t a) { 
    
    set_a* p = a;

    pin_a  pin_x = { p->s_data, 0, sizeof(_set_node) };
    pod_a  pod_x = { _set_comp_hash, _set_find_hash, NULL, NULL, NULL };

    size_t pin_s = pin_pass(&pin_x); czz(pin_s);
    size_t pod_s = pod_pass(&pod_x); czz(pod_s);

    return t_size(set) + pin_s + pod_s;

}

pobj_t set_init(pbuf_t b, parg_t a, pobj_t h) {

    cxx(a);

    set_a* p_args = a;

    size_t p_size = set_pass(p_args); czx(p_size);

    bag*   p_pick = h && pobj_pick(h) ? pobj_pick(h) : sac_init();

    char*  p_buf  = b ? b : cap_data(bag_take(p_pick, p_size, 0));

    set* o = (set*)p_buf; p_buf += t_size(set);  // use p_buf for set object data

    CNF.size = sizeof(set);
    CNF.type = set_type();
    CNF.hold = h;
    CNF.pick = p_pick;
    CNF.fini = set_fini;

    OBJ.p_bind = NULL;
    OBJ.o_head = NULL;
    OBJ.o_next = NULL;
    OBJ.o_prev = NULL;

    ARG = *p_args;

    // all core fields should be set

    pin_a pin_x = { ARG.s_data, 0, sizeof(_set_node) };

    COR.data   = pin_init(p_buf, &pin_x, o); p_buf += pin_pass(&pin_x);  // use p_buf for pin object data
    COR.count  = 0;
    COR.p_free = NULL;

    _set_hash_f* f_hash;
    _set_find_f* f_find;

    switch(ARG.s_index) {

        case SET_INDEX_TEXT:  f_hash = _set_comp_hash;  f_find = _set_find_hash;  COR.f_comp = _set_comp_text; break;
        case SET_INDEX_CHAR:  f_hash = _set_comp_char;  f_find = _set_find_char;  COR.f_comp = NULL;           break;
        case SET_INDEX_UINT:  f_hash = _set_comp_uint;  f_find = _set_find_uint;  COR.f_comp = NULL;           break;
        case SET_INDEX_LONG:  f_hash = _set_comp_long;  f_find = _set_find_long;  COR.f_comp = NULL;           break;
        case SET_INDEX_FLOAT: f_hash = _set_comp_float; f_find = _set_find_float; COR.f_comp = NULL;           break;
        default:              f_hash = _set_comp_hash;  f_find = _set_find_hash;  COR.f_comp = _set_comp_data; break;

    }

    pod_a pod_x = { f_hash, f_find, _set_push, _set_pull, o }; 

    COR.hash    = pod_init(p_buf, &pod_x, o);  // use p_buf for pod object data

    if(h) obj_push(h, o);  // push to hold object

    obj_set_stat(o, OBJ_STAT_WORK);

    return o;

}

void set_fini(pobj_t x) {

    cne(pobj_stat(x) != OBJ_STAT_NONE);

    obj_set_stat(x, OBJ_STAT_NONE);

    obj_fini(x);

}

static int _set_comp_hash(pod* x, cap* a, cap* b) { 

    set*       o  = pobj_bind(x);
    _set_node* na = pin_head_data(COR.data, a);
    _set_node* nb = pin_head_data(COR.data, b);

    return na->hash.hash > nb->hash.hash ? 1 : (na->hash.hash < nb->hash.hash ? -1 : 0);

}

static int _set_comp_char(pod* x, cap* a, cap* b) { 

    set*       o  = pobj_bind(x);
    _set_node* na = pin_head_data(COR.data, a);
    _set_node* nb = pin_head_data(COR.data, b);

    byte_t ba = (byte_t)na->hash.c;
    byte_t bb = (byte_t)nb->hash.c;
    
    return ba > bb ? 1 : (ba < bb ? -1 : 0);

}

static int _set_comp_uint(pod* x, cap* a, cap* b) { 

    set*       o  = pobj_bind(x);
    _set_node* na = pin_head_data(COR.data, a);
    _set_node* nb = pin_head_data(COR.data, b);

    return na->hash.u > nb->hash.u ? 1 : (na->hash.u < nb->hash.u ? -1 : 0);

}

static int _set_comp_long(pod* x, cap* a, cap* b) { 

    set*       o  = pobj_bind(x);
    _set_node* na = pin_head_data(COR.data, a);
    _set_node* nb = pin_head_data(COR.data, b);

    return na->hash.l > nb->hash.l ? 1 : (na->hash.l < nb->hash.l ? -1 : 0);

}

static int _set_comp_float(pod* x, cap* a, cap* b) { 

    set*       o  = pobj_bind(x);
    _set_node* na = pin_head_data(COR.data, a);
    _set_node* nb = pin_head_data(COR.data, b);
    
    return na->hash.f > nb->hash.f ? 1 : (na->hash.f < nb->hash.f ? -1 : 0);

}

static int _set_find_hash(pod* x, data_t a, cap* b) {

    set*          o  = pobj_bind(x);
    const uint_t* ha = (const uint_t*)a;
    _set_node*    nb = pin_head_data(COR.data, b);

    return *ha > nb->hash.hash ? 1 : (*ha < nb->hash.hash ? -1 : 0);

}

static int _set_find_char(pod* x, data_t a, cap* b) {

    set*                  o  = pobj_bind(x);
    const _set_hash_type* h  = a;
    _set_node*            nb = pin_head_data(COR.data, b);

    byte_t ca = (byte_t)h->c;
    byte_t cb = (byte_t)nb->hash.c;
    
    return ca > cb ? 1 : (ca < cb ? -1 : 0);

}

static int _set_find_uint(pod* x, data_t a, cap* b) {

    set*                  o  = pobj_bind(x);
    const _set_hash_type* h  = a;
    _set_node*            nb = pin_head_data(COR.data, b);

    return h->u > nb->hash.u ? 1 : (h->u < nb->hash.u ? -1 : 0);

}

static int _set_find_long(pod* x, data_t a, cap* b) {

    set*                  o  = pobj_bind(x);
    const _set_hash_type* h  = a;
    _set_node*            nb = pin_head_data(COR.data, b);

    return h->l > nb->hash.l ? 1 : (h->l < nb->hash.l ? -1 : 0);

}

static int _set_find_float(pod* x, data_t a, cap* b) {

    set*                  o  = pobj_bind(x);
    const _set_hash_type* h  = a;
    _set_node*            nb = pin_head_data(COR.data, b);

    return h->f > nb->hash.f ? 1 : (h->f < nb->hash.f ? -1 : 0);

}

static inline void _set_free_node(set* o, cap* a, _set_node* na) { 

    na->hash_next = COR.p_free;

    COR.p_free    = a;
    COR.count--;

}

static cap* _set_push(pod* x, cap* a, cap* b) {

    ifx(a) return b;

    set*   o = pobj_bind(x);
    pin*   p = COR.data;
    size_t s = pin_head_size(p, b) - sizeof(_set_node);
    cap*   r = a;
    cap*   t = a;
    cap*   c;

    _set_node* na = pin_head_data(p, a);
    _set_node* nb = pin_head_data(p, b);
    _set_node* nc;
    _set_node* nt;

    if(COR.f_comp) while(COR.f_comp(na->index, nb->index, s)) {   // not same index

        ifx(na->hash_next) {

            na->hash_next = b;  // add to hash list tail when no same index

            return r;

        }

        t  = a;
        a  = na->hash_next;
        na = pin_head_data(p, a);

    }

    // same index

    switch(ARG.add_mode) {
        
        case SET_ADD_ONCE: 

            _set_free_node(o, b, nb);

            r = NULL;  // fail to add
            
            break;

        case SET_ADD_PUSH:  // push to top

            // t - a - x  =>  t - b - x
            //                    |
            //                    a

            c  = na->data_prev;
            nc = pin_head_data(p, c);

            nc->data_next = b;

            nb->data_prev = c;
            nb->data_next = a;

            na->data_prev = b;
            if(na->data_next == a) na->data_next = b;

            nb->hash_next = na->hash_next; 
            na->hash_next = NULL;

            if(t == a) { r = b; break; }
            
            nt = pin_head_data(p, t); 
            
            nt->hash_next = b; 
            
            break;

        case SET_ADD_LIST:  // push to prev

            //                    b
            //                    |
            // t - a - x  =>  t - a - x

            c  = na->data_prev;
            nc = pin_head_data(p, c);

            nc->data_next = b;

            nb->data_prev = c;
            nb->data_next = a;

            na->data_prev = b;
            if(na->data_next == a) na->data_next = b;

            break;

        default:  // SET_ADD_LAST

            _set_free_node(o, a, na);

            if(t == a) { r = b; break; }

            nt = pin_head_data(p, t); 
            
            nt->hash_next = b; 

            break;            

    }

    return r;

}

static inline void _set_data_pull(set* o, cap* a, _set_node* na) {

    cap*       b  = na->data_next;
    _set_node* nb = pin_head_data(COR.data, b);

    if(nb->data_next == a) {  // only two elements

        nb->data_next = b;
        nb->data_prev = b;

    }
    else {

        nb->data_prev = na->data_prev;

    }

    nb->hash_next = na->hash_next;

    na->data_prev = NULL;
    na->data_next = NULL;

    _set_free_node(o, a, na);

}

static cap* _set_pull(pod* x, data_t a, cap* b) {

    set*       o  = pobj_bind(x);
    pin*       p  = COR.data;
    parg_t     i  = o->d_work.del_index;
    _set_node* nb = pin_head_data(p, b);
    size_t     sb = pin_head_size(p, b) - sizeof(_set_node);
    cap*       t  = NULL;
    cap*       r  = b;

    if(COR.f_comp) while(COR.f_comp(i, nb->index, sb)) {

        t  = b;
        b  = nb->hash_next; ifx(b) return r;
        nb = pin_head_data(COR.data, b);
        sb = pin_head_size(p, b) - sizeof(_set_node);

    }

    ifx(t) {  // first item of hash node

        if(nb->hash_next == NULL && nb->data_next == NULL) {

            _set_free_node(o, b, nb);

            return NULL;  // remove this pod node

        }

        if(nb->data_next == NULL || nb->data_next == b) {
            
            t = nb->hash_next; 
            
            _set_free_node(o, b, nb);

            return t;  // remove b, replaced by nb->hash_next

        }

        t = nb->data_next;

        _set_data_pull(o, b, nb);

        return t;  // remove b, replaced by nb->data_next

    }

    if(nb->data_next == NULL || nb->data_next == b) {
        
        _set_node* nt = pin_head_data(COR.data, t);

        nt->hash_next = nb->hash_next; 
        
        _set_free_node(o, b, nb);

        return r;

    }

    _set_data_pull(o, b, nb);

    return r;

}

static int _set_comp_text(data_t a, data_t b, size_t s) { return text_comp(a, b, s); }
static int _set_comp_data(data_t a, data_t b, size_t s) { return data_comp(a, b, s); }

size_t set_count(set* o) { return COR.count; }

set*   new_set_c(size_t s_index, size_t s_data, set_add_mode add_mode, pobj_t p_hold) {

    set_a a = { s_index, s_data, add_mode };

    return set_init(NULL, &a, p_hold);

}

bool set_add(set* o, parg_t i, parg_t d) {

    size_t s_index;
    size_t s_data  = ARG.s_data == SET_DATA_TEXT ? text_size(d, 0) : ARG.s_data;

    switch(ARG.s_index) {

        case SET_INDEX_TEXT:  s_index = text_size(i, 0); break;
        case SET_INDEX_CHAR:  
        case SET_INDEX_UINT:  
        case SET_INDEX_LONG:  
        case SET_INDEX_FLOAT: s_index = 0;                      break;
        default:              s_index = ARG.s_index;            break;

    }

    cap*       b = pin_ask(COR.data, NULL, s_data, s_index);
    _set_node* n = pin_head_data(COR.data, b);

    switch(ARG.s_index) {

        case SET_INDEX_TEXT:  n->hash.hash = text_hash(i); text_copy(n->index, i, s_index); break;
        case SET_INDEX_CHAR:  n->hash.c    = * (char*)   i; break;
        case SET_INDEX_UINT:  n->hash.u    = * (uint_t*) i; break;
        case SET_INDEX_LONG:  n->hash.l    = * (long*)   i; break;
        case SET_INDEX_FLOAT: n->hash.f    = * (float*)  i; break;
        default:              n->hash.hash = data_hash(i, s_index); data_copy(n->index, i, s_index); break;

    }

    n->hash_next = NULL;

    switch(ARG.add_mode) {

        case SET_ADD_PUSH:
        case SET_ADD_LIST:

            n->data_next = b;
            n->data_prev = b;

            break;

        default:

            n->data_next = NULL;
            n->data_prev = NULL;

            break;

    }


    pbuf_t t = cap_data(b);

    switch(ARG.s_data) {
    
        case SET_DATA_TEXT: text_copy(t, d, s_data); break;
        default:            data_copy(t, d, s_data); break;

    }

    COR.count++;

    return pod_add(COR.hash, b, NULL);

}

static _set_hash_type _set_hash_value(set* o, parg_t i) {

   _set_hash_type h;

    switch(ARG.s_index) {

        case SET_INDEX_TEXT:  h.hash = text_hash(i);  break;
        case SET_INDEX_CHAR:  h.c    = * (char*)   i; break;
        case SET_INDEX_UINT:  h.u    = * (uint_t*) i; break;
        case SET_INDEX_LONG:  h.l    = * (long*)   i; break;
        case SET_INDEX_FLOAT: h.f    = * (float*)  i; break;
        default:              h.hash = data_hash(i, ARG.s_index); break;

    }

    return h;

}

cap* set_get(set* o, parg_t i) {

    pod_p          p;
    _set_hash_type h = _set_hash_value(o, i);

    cnx(pod_find(COR.hash, &p, &h));

    cap*       a  = pod_get(COR.hash, &p);
    _set_node* an = pin_head_data(COR.data, a);
    size_t     as = pin_head_size(COR.data, a) - sizeof(_set_node);


    if(COR.f_comp) while(COR.f_comp(an->index, i, as)) {

        a  = an->hash_next; cxx(a);
        an = pin_head_data(COR.data, a);
        as = pin_head_size(COR.data, a) - sizeof(_set_node);

    }

    return a;

}

bool set_del(set* o, parg_t i) { 

    _set_hash_type h = _set_hash_value(o, i);

    o->d_work.del_index = i;
    
    return pod_del(COR.hash, &h); 
    
}

static inline size_t _set_loop_data(set* o, cap* a, _set_node* na) { 

    o->d_work.d_loop->d = a;
    o->d_work.d_loop->i = na->index;

    return o->d_work.f_loop(o, o->d_work.d_loop);

}

static size_t _set_loop_list(set* o, cap* a, _set_node* na) { 

    size_t r = _set_loop_data(o, a, na);

    cxr(na->data_next);

    cap*       b  = a; 
    _set_node* nb = na;

    while(nb->data_next != a) {

        b  = nb->data_next;
        nb = pin_head_data(COR.data, b);

        r += _set_loop_data(o, b, nb);

    }

    return r;

}

static size_t _set_loop_node(set* o, cap* a) {

    _set_node* na = pin_head_data(COR.data, a);

    size_t     r  = _set_loop_list(o, a, na);

    while(na->hash_next) {

        a  = na->hash_next;
        na = pin_head_data(COR.data, a);

        r += _set_loop_list(o, a, na);

    }

    return r;

}

static size_t _set_loop_hash(set* o, cap* a) {

    _set_node* na = pin_head_data(COR.data, a);

    size_t     r  = _set_loop_data(o, a, na);

    while(na->hash_next) {

        a  = na->hash_next;
        na = pin_head_data(COR.data, a);

        r += _set_loop_data(o, a, na);

    }

    return r;

}

static size_t _set_loop_index(set* o, cap* a) {

    pin*       p  = COR.data;
    _set_node* na = pin_head_data(p, a);
    size_t     sa = pin_head_size(p, a) - sizeof(_set_node);

    if(COR.f_comp) while(COR.f_comp(na->index, o->d_work.d_loop->i, sa)) {

        cxz(na->hash_next);

        a  = na->hash_next;
        na = pin_head_data(p, a);
        sa = pin_head_size(p, a) - sizeof(_set_node);

    }

    return _set_loop_list(o, a, na);

}

static size_t _set_pod_loop_def(pod* x, pod_l* p) {

    set* o = p->e;
    cap* a = pod_get(x, p->p);

    return _set_loop_hash(o, a);

}

static size_t _set_pod_loop_all(pod* x, pod_l* p) {

    set* o = p->e;
    cap* a = pod_get(x, p->p);

    return _set_loop_node(o, a);

}

size_t set_loop(set* o, set_loop_f f, parg_t e) {

    set_l p  = { SET_LOOP_DEF, e, NULL, NULL };

    o->d_work.f_loop = f;
    o->d_work.d_loop = &p;

    return pod_loop(COR.hash, _set_pod_loop_def, NULL, NULL, o);

}

size_t set_loop_all(set* o, set_loop_f f, parg_t e) {

    set_l p  = { SET_LOOP_ALL, e, NULL, NULL };

    o->d_work.f_loop = f;
    o->d_work.d_loop = &p;

    return pod_loop(COR.hash, _set_pod_loop_all, NULL, NULL, o);

}

size_t set_loop_del(set* o, set_loop_f f, parg_t e) {

    set_l p  = { SET_LOOP_DEL, e, NULL, NULL };

    cxz(COR.p_free);

    o->d_work.f_loop = f;
    o->d_work.d_loop = &p;

    return _set_loop_node(o, COR.p_free);

}

size_t set_loop_idx(set* o, set_loop_f f, parg_t i, parg_t e) {

    set_l p  = { SET_LOOP_IDX, e, i, NULL };
    cap*  a  = set_get(o, i);

    cxz(a);

    o->d_work.f_loop = f;
    o->d_work.d_loop = &p;

    return _set_loop_index(o, a);

}

// inline functions

extern inline set* new_set(size_t s_index, size_t s_data);

extern inline void del_set(set* o, pobj_t caller CAN_NULL);

