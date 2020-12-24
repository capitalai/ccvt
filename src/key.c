#include "lib.h"
#include "key.h"
#include "seq.h"
#include "pod.h"
#include "sac.h"

typedef struct _key           _key;
typedef struct _key_node      _key_node;
typedef struct _key_find_node _key_find_node;
typedef struct _key_work      _key_work;

struct _key_work {

    key_l*      d_loop;     // (loop) loop data
    key_loop_f* f_loop;     // (loop) user function for loop

};

struct _key_node {

    uint hash;
    cap* text;

};

struct _key_find_node {

    uint   hash;
    text_t text;

};

struct _key {

    seq*  data;
    pod*  name;

};

struct key {

    obj       d_base;    // object data
    key_a     d_args;    // object parameters
    _key      d_core;    // core data
    _key_work d_work;    // work data

};

static key_a def_key_a = { 0, 0, NULL, NULL };

static int _key_comp(pod* x, cap*   a, cap* b);
static int _key_find(pod* x, data_t a, cap* b);

size_t key_size(void) {  return sizeof(key); }
size_t key_type(void) { tid_make(key); return tid(key); }

size_t key_pass(parg_t a) { 

    key_a* p = a;

    size_t s = p->s_item ? p->s_item : data_s;

    seq_a  seq_x = { s, 0, sizeof(_key_node), NULL, NULL };
    pod_a  pod_x = { _key_comp, _key_find, NULL, NULL, NULL };

    size_t seq_s = seq_pass(&seq_x); czz(seq_s);
    size_t pod_s = pod_pass(&pod_x); czz(pod_s);

    return t_size(key) + seq_s + pod_s; 

}

static bool _key_item_init(seq* x, cap* d) { key* o = pobj_hold(x); return ARG.f_init ? ARG.f_init(o, d) : true; }
static void _key_item_fini(seq* x, cap* d) { key* o = pobj_hold(x); if(ARG.f_fini)      ARG.f_fini(o, d); }

static inline void _key_remove(pod* x, cap* d) {

    key*       o = pobj_hold(x);
    _key_node* n = seq_head_data(COR.data, d);

    seq_del(COR.data, d);

    bag_drop(CNF.pick, n->text);  // remove index text after seq_del() for seq item fini function

}

static cap* _key_push(pod* x, cap* a, cap* b) {

    ifx(a) return b;

    _key_remove(x, b);

    return a;

}

static cap* _key_pull(pod* x, data_t a, cap* b) {

    _key_remove(x, b);

    return NULL;

}

pobj_t key_init(pbuf_t b, parg_t a, pobj_t h) {

    key_a* p_args = a ? a : &def_key_a;

    size_t p_size = key_pass(p_args); czx(p_size);

    bag*   p_pick = h && pobj_pick(h) ? pobj_pick(h) : sac_init();

    char*  p_buf  = b ? b : cap_data(bag_take(p_pick, p_size, 0));

    key* o = (key*)p_buf; p_buf += t_size(key);  // use p_buf for seq object data

    CNF.size = sizeof(key);
    CNF.type = seq_type();
    CNF.hold = h;
    CNF.pick = p_pick;
    CNF.fini = key_fini;

    OBJ.p_bind = NULL;
    OBJ.o_head = NULL;
    OBJ.o_next = NULL;
    OBJ.o_prev = NULL;

    ARG = *p_args;

    ifz(ARG.s_item) ARG.s_item = data_s;

    // all core fields should be set

    seq_a seq_x = { ARG.s_item, ARG.t_item, sizeof(_key_node), _key_item_init, _key_item_fini };

    COR.data = seq_init(p_buf, &seq_x, o); p_buf += seq_pass(&seq_x);  // use p_buf for seq object data

    pod_a pod_x = { _key_comp, _key_find, _key_push, _key_pull, o };

    COR.name = pod_init(p_buf, &pod_x, o);

    if(h) obj_push(h, o);  // push to hold object

    obj_set_stat(o, OBJ_STAT_WORK);

    return o;

}

void key_fini(pobj_t x) {

    cne(pobj_stat(x) != OBJ_STAT_NONE);

    obj_set_stat(x, OBJ_STAT_NONE);

    obj_fini(x);

}

static int _key_comp(pod* x, cap*   a, cap* b) {

    key* o = pobj_bind(x);

    _key_node* an = seq_head_data(COR.data, a);
    _key_node* bn = seq_head_data(COR.data, b);

    if(an->hash > bn->hash) return  1;
    if(an->hash < bn->hash) return -1;

    return text_comp(cap_data(an->text), cap_data(bn->text), cap_size(an->text));

}

static int _key_find(pod* x, data_t a, cap* b) {

    key*                  o  = pobj_bind(x);
    const _key_find_node* an = a;
    _key_node*            bn = seq_head_data(COR.data, b);

    if(an->hash > bn->hash) return  1;
    if(an->hash < bn->hash) return -1;

    return text_comp(an->text, cap_data(bn->text), cap_size(bn->text));

}

bool   key_full (key* o) { return seq_full (COR.data); }
size_t key_count(key* o) { return seq_count(COR.data); }

size_t key_data_size(key* o) { return ARG.s_item; }
size_t key_data_type(key* o) { return ARG.t_item; }

key* new_key_c(size_t s_item, size_t t_item, key_init_f* f_init, key_fini_f* f_fini, pobj_t p_hold) {

    key_a p = { s_item, t_item, f_init, f_fini };

    return key_init(NULL, &p, p_hold);

}

cap* key_get(key* o, text_t v) {

    cxx(v); czx(*v);

    pod_p          p = { NULL };
    _key_find_node f = { text_hash(v), v };

    cnx(pod_find(COR.name, &p, &f));

    return pod_get(COR.name, &p);

}

cap* key_add(key* o, text_t v) {

    cxx(v); czx(*v);

    size_t     s = text_size(v, 0);
    cap*       r = seq_add(COR.data);
    _key_node* n = seq_head_data(COR.data, r);

    n->hash = text_hash(v);
    n->text = bag_take(CNF.pick, s, 0);

    text_copy(cap_data(n->text), v, s);

    if(pod_add(COR.name, r, NULL)) return r;

    return NULL;

}

bool key_del(key* o, text_t k) {

    cxf(k); czx(*k);

    _key_find_node f = { text_hash(k), k };

    return pod_del(COR.name, &f);

}

bool key_put(key* o, text_t a, text_t d) {

    cap* ad = key_get(o, a); cxf(ad);
    cap* dd = key_get(o, d); cxf(dd);

    return seq_put(COR.data, ad, dd);

}

bool key_ins(key* o, text_t b, text_t d) {

    cap* bd = key_get(o, b); cxf(bd);
    cap* dd = key_get(o, d); cxf(dd);

    return seq_ins(COR.data, bd, dd);

}

cap* key_head(key* o) { return seq_head(COR.data); }
cap* key_tail(key* o) { return seq_tail(COR.data); }

cap* key_next(key* o, cap* p) { return seq_next(COR.data, p); }
cap* key_prev(key* o, cap* p) { return seq_prev(COR.data, p); }

text_t key_name(key* o, cap* p) {

    cnz(seq_check(COR.data, p));

    _key_node* n = seq_head_data(COR.data, p);

    return cap_data(n->text);

}

static size_t _key_loop(seq* x, seq_l* p) {

    key* o = p->e;

    o->d_work.d_loop->d = p->d;

    return o->d_work.f_loop(o, o->d_work.d_loop);

}

size_t key_loop(key* o, key_loop_f f, text_t start, text_t stop, parg_t extra) {

    cap*  a = start ? key_get(o, start) : NULL; ifx(a) a = key_head(o);
    cap*  b = stop  ? key_get(o, stop)  : NULL; ifx(a) b = key_tail(o);

    key_l d = { extra, NULL };

    o->d_work.f_loop = f;
    o->d_work.d_loop = &d;

    return seq_loop(COR.data, _key_loop, a, b, o);

}

size_t key_hoop(key* o, key_loop_f f, text_t start, text_t stop, parg_t extra) {

    cap*  a = start ? key_get(o, start) : NULL; ifx(a) a = key_tail(o);
    cap*  b = stop  ? key_get(o, stop)  : NULL; ifx(a) b = key_head(o);

    key_l d = { extra, NULL };

    o->d_work.f_loop = f;
    o->d_work.d_loop = &d;

    return seq_hoop(COR.data, _key_loop, a, b, o);

}

bool key_check(key* o, cap* d) { return seq_check(COR.data, d); }

extern inline key* new_key  (void);
extern inline key* new_key_s(size_t s_item);
extern inline void del_key(key* o, pobj_t caller);
