#include "lib.h"
#include "txt.h"
#include "pin.h"
#include "sac.h"

typedef struct _txt      _txt;
typedef struct _txt_work _txt_work;

struct _txt_work {

    txt_l*      d_loop;     // (loop) loop data
    txt_loop_f* f_loop;     // (loop) user function for loop
    txt_comp_f* f_comp;     // (comp) user function for comp

};

struct _txt {

    pin*      data;
    str_pool* pool;

};

struct txt {

    obj       d_base;    // object data
    _txt      d_core;    // core data
    _txt_work d_work;    // work data

};

size_t txt_size(void) {  return sizeof(txt); }
size_t txt_type(void) { tid_make(txt); return tid(txt); }

size_t txt_pass(parg_t a) {

    pin_a  pin_x = { 0, 0, 0 };

    size_t pin_s = pin_pass(&pin_x); czz(pin_s);

    return t_size(txt) + pin_s;

}

pobj_t txt_init(pbuf_t b CAN_NULL, parg_t a CAN_NULL, pobj_t h CAN_NULL) {

    size_t p_size = txt_pass(a); czx(p_size);

    bag*   p_pick = h && pobj_pick(h) ? pobj_pick(h) : sac_init();

    char*  p_buf  = b ? b : cap_data(bag_take(p_pick, p_size, 0));

    txt* o = (txt*)p_buf; p_buf += t_size(txt);  // use p_buf for txt object data

    CNF.size = sizeof(txt);
    CNF.type = txt_type();
    CNF.hold = h;
    CNF.pick = p_pick;
    CNF.fini = txt_fini;

    OBJ.p_bind = NULL;
    OBJ.o_head = NULL;
    OBJ.o_next = NULL;
    OBJ.o_prev = NULL;

    // all core fields should be set

    pin_a x = { 0, 0, 0 };

    COR.data   = pin_init(p_buf, &x, o);  // use p_buf for pin object data
    COR.pool   = str_pool_init(0, o);

    if(h) obj_push(h, o);  // push to hold object

    obj_set_stat(o, OBJ_STAT_WORK);

    return o;

}

void txt_fini(pobj_t x) {

    cne(pobj_stat(x) != OBJ_STAT_NONE);

    obj_set_stat(x, OBJ_STAT_NONE);

    txt* o = (txt*)x;

    str_pool_fini(COR.pool);

    obj_fini(x);

}

bool txt_full(txt* o) { return pin_full(COR.data); }

bool txt_exist(txt* o, size_t n) { return pin_exist(COR.data, n); }

size_t txt_count(txt* o) { return pin_count(COR.data); }

extern inline txt* new_txt(void);
extern inline void del_txt(txt* o, pobj_t caller);

str* txt_add(txt* o, size_t* n, text_t t) {

    cap*  d = pin_add(COR.data, n); cxx(d);
    str** x = cap_data(d);

    *x = str_init(COR.pool, t);

    return *x;

}

str* txt_get(txt* o, size_t n) { 
    
    cap*  c = pin_get(COR.data, n); cxx(c);
    str** x = cap_data(c);

    return *x;

}

bool txt_swap(txt* o, size_t a, size_t b) { return pin_swap(COR.data, a, b); }

static int _txt_comp(pin* x, data_t a, data_t b, parg_t e) {

    txt* o = pobj_bind(x);

    str* const* sa = a;
    str* const* sb = b;

    return o->d_work.f_comp(o, *sa, *sb, e);

}

void txt_sort(txt* o, txt_comp_f f, parg_t e) {

    o->d_work.f_comp = f;

    return pin_sort(COR.data, _txt_comp, e);

}

static size_t _txt_loop(pin* x, pin_l* p) {

    txt*   o = p->e;
    cap*   c = p->d;
    str**  d = cap_data(c);
    size_t r;

    o->d_work.d_loop->i = p->i;
    o->d_work.d_loop->d = *d;

    r = o->d_work.f_loop(o, o->d_work.d_loop);

    p->t = o->d_work.d_loop->t;
    p->i = o->d_work.d_loop->i;

    return r;

}

size_t txt_loop(txt* o, txt_loop_f f, txt_l* p) {

    pin_l d = { false, o, 0, NULL };
    txt_l b = { false, NULL, 0, NULL };

    o->d_work.f_loop = f;

    if(p) {

        d.t = p->t;
        d.i = p->i;

        o->d_work.d_loop = p;

    }
    else o->d_work.d_loop = &b;

    return pin_loop(COR.data, _txt_loop, &d);

}

size_t txt_hoop(txt* o, txt_loop_f f, txt_l* p) {

    pin_l d = { false, o, 0, NULL };
    txt_l b = { false, NULL, 0, NULL };

    o->d_work.f_loop = f;

    if(p) {

        d.t = p->t;
        d.i = p->i;
        
        o->d_work.d_loop = p;

    }
    else o->d_work.d_loop = &b;

    return pin_hoop(COR.data, _txt_loop, &d);

}
