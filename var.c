#include "lib.h"
#include "var.h"
#include "key.h"
#include "str.h"
#include "sac.h"

typedef struct _var      _var;
typedef struct _var_node _var_node;
typedef struct _var_work _var_work;

struct _var_node {

    uint_t x_type;

    obj*   p_obj;
    cap*   p_bin;
    str*   p_str;
    val    d_val;

};

struct _var_work {

    var_l*      d_loop;     // (loop) loop data
    var_loop_f* f_loop;     // (loop) user function for loop

};

static const size_t _var_node_s = t_size(_var_node);

struct _var {

    key*      data;
    str_pool* pool;

};

struct var {

    obj       d_base;    // object data
    _var      d_core;    // core data
    _var_work d_work;    // work data

};

size_t var_size(void) {  return sizeof(var); }
size_t var_type(void) { tid_make(var); return tid(var); }

size_t var_pass(parg_t a) {

    key_a  key_x = { sizeof(_var_node), 0, NULL, NULL };

    size_t key_s = key_pass(&key_x); czz(key_s);

    return t_size(var) + key_s;

}

static bool _var_item_init(key* x, cap* d) { 

    data_wipe(cap_data(d), cap_size(d));

    return true;

}

static void _var_item_fini(key* x, cap* d) { 
  
    _var_node* p = cap_data(d);

    if(p->p_obj) obj_fini(p->p_obj);
  
}

pobj_t var_init(pbuf_t b CAN_NULL, parg_t a CAN_NULL, pobj_t h CAN_NULL) {

    size_t p_size = var_pass(a); czx(p_size);

    bag*   p_pick = h && pobj_pick(h) ? pobj_pick(h) : sac_init();

    char*  p_buf  = b ? b : cap_data(bag_take(p_pick, p_size, 0));

    var* o = (var*)p_buf; p_buf += t_size(var);  // use p_buf for var object data

    CNF.size = sizeof(var);
    CNF.type = var_type();
    CNF.hold = h;
    CNF.pick = p_pick;
    CNF.fini = var_fini;

    OBJ.p_bind = NULL;
    OBJ.o_head = NULL;
    OBJ.o_next = NULL;
    OBJ.o_prev = NULL;

    // all core fields should be set

    key_a x = { _var_node_s, 0, _var_item_init, _var_item_fini };

    COR.data   = key_init(p_buf, &x, o);  // use p_buf for pin object data
    COR.pool   = str_pool_init(0, o);

    if(h) obj_push(h, o);  // push to hold object

    obj_set_stat(o, OBJ_STAT_WORK);

    return o;

}

void var_fini(pobj_t x) {

    cne(pobj_stat(x) != OBJ_STAT_NONE);

    obj_set_stat(x, OBJ_STAT_NONE);

    var* o = (var*)x;

    str_pool_fini(COR.pool);

    obj_fini(x);

}

static _var_node* _var_set(var* o, text_t k) {

    cap*       c = key_get(COR.data, k);
    _var_node* p;

    if(c) {

        p = cap_data(c);

        switch(p->x_type) {

            case var_type_bool: p->d_val.v_bool = false;  break;
            case var_type_int:  p->d_val.v_ulli = 0;      break;
            case var_type_num:  p->d_val.v_real = 0.0;    break;
            case var_type_char: p->d_val.v_utf8.c[0] = 0; 
                                p->d_val.v_utf8.c[1] = 0; 
                                p->d_val.v_utf8.c[2] = 0; 
                                p->d_val.v_utf8.c[3] = 0; 
                                p->d_val.v_utf8.c[4] = 0; break;
            case var_type_ptr:  p->d_val.v_type = 0; 
                                p->d_val.v_data = NULL;   break;
            case var_type_str:  str_fini(p->p_str);           p->p_str = NULL; break;
            case var_type_bin:  bag_drop(CNF.pick, p->p_bin); p->p_bin = NULL; break;
            case var_type_obj:  obj_fini(p->p_obj);           p->p_obj = NULL; break;

        }

    }
    else {

        c = key_add(COR.data, k); cxx(c);
        p = cap_data(c);

    }

    return p;

}

bool var_set_obj(var* o, text_t k, pobj_t a) {

    _var_node* n = _var_set(o, k); cxf(n);

    n->p_obj  = a;
    n->x_type = var_type_obj;

    return true;

}

bool var_set_bin(var* o, text_t k, size_t s, data_t a) {

    czf(s); cxf(a);

    _var_node* n = _var_set(o, k); cxf(n);

    n->p_bin  = bag_take(CNF.pick, s, 0);

    data_copy(cap_data(n->p_bin), a, s);

    n->x_type = var_type_bin;

    return true;

}

bool var_set_str (var* o, text_t k, text_t a) {

    _var_node* n = _var_set(o, k); cxf(n);

    n->p_str  = str_init(COR.pool, a);
    n->x_type = var_type_str;

    return true;

}

bool var_set_ptr (var* o, text_t k, uint_t t, pbuf_t p) {

    _var_node* n = _var_set(o, k); cxf(n);

    n->d_val.v_type = t;
    n->d_val.v_data = p;
    n->x_type       = var_type_ptr;

    return true;

}

bool var_set_char(var* o, text_t k, utf8_t v) {

    _var_node* n = _var_set(o, k); cxf(n);

    n->d_val.v_utf8.c[0] = v.c[0];
    n->d_val.v_utf8.c[1] = v.c[1];
    n->d_val.v_utf8.c[2] = v.c[2];
    n->d_val.v_utf8.c[3] = v.c[3];
    n->d_val.v_utf8.c[4] = v.c[4];

    n->x_type            = var_type_char;

    return true;

}

bool var_set_num (var* o, text_t k, real_t v) {

    _var_node* n = _var_set(o, k); cxf(n);

    n->d_val.v_real = v;
    n->x_type       = var_type_num;

    return true;

}

bool var_set_int (var* o, text_t k, ulli_t v) {

    _var_node* n = _var_set(o, k); cxf(n);

    n->d_val.v_ulli = v;
    n->x_type       = var_type_int;

    return true;

}

bool var_set_bool(var* o, text_t k, bool v) {

    _var_node* n = _var_set(o, k); cxf(n);

    n->d_val.v_bool = v;
    n->x_type       = var_type_bool;

    return true;

}

pobj_t var_get_obj(var* o, text_t k) {

  cap*       c = key_get(COR.data, k); cxx(c);
  _var_node* n = cap_data(c);          cnx(n->x_type == var_type_obj);

  return n->p_obj;

}

cap* var_get_bin(var* o, text_t k) {

  cap*       c = key_get(COR.data, k); cxx(c);
  _var_node* n = cap_data(c);          cnx(n->x_type == var_type_bin);

  return n->p_bin;

}

str* var_get_str(var* o, text_t k) {

  cap*       c = key_get(COR.data, k); cxx(c);
  _var_node* n = cap_data(c);          cnx(n->x_type == var_type_str);

  return n->p_str;

}

val* var_get_ptr(var* o, text_t k) {

  cap*       c = key_get(COR.data, k); cxx(c);
  _var_node* n = cap_data(c);          cnx(n->x_type == var_type_ptr);

  return &n->d_val;

}

val* var_get_char(var* o, text_t k) {

  cap*       c = key_get(COR.data, k); cxx(c);
  _var_node* n = cap_data(c);          cnx(n->x_type == var_type_char);

  return &n->d_val;

}

val* var_get_num(var* o, text_t k) {

  cap*       c = key_get(COR.data, k); cxx(c);
  _var_node* n = cap_data(c);          cnx(n->x_type == var_type_num);

  return &n->d_val;

}

val* var_get_int(var* o, text_t k) {

  cap*       c = key_get(COR.data, k); cxx(c);
  _var_node* n = cap_data(c);          cnx(n->x_type == var_type_int);

  return &n->d_val;

}

val* var_get_bool(var* o, text_t k) {

  cap*       c = key_get(COR.data, k); cxx(c);
  _var_node* n = cap_data(c);          cnx(n->x_type == var_type_bool);

  return &n->d_val;

}

static size_t _var_loop(key* x, key_l* p) {

    var*       o = p->e;
    _var_node* n = cap_data(p->d);

    o->d_work.d_loop->n = key_name(x, p->d);
    o->d_work.d_loop->t = n->x_type;

    switch(n->x_type) {
        case var_type_bool:
        case var_type_int:
        case var_type_num:
        case var_type_char:
        case var_type_ptr:   o->d_work.d_loop->v = &n->d_val; break;
        case var_type_str:   o->d_work.d_loop->s = n->p_str;  break;
        case var_type_bin:   o->d_work.d_loop->b = n->p_bin;  break;
        case var_type_obj:   o->d_work.d_loop->o = n->p_obj;  break;
    }

    return o->d_work.f_loop(o, o->d_work.d_loop);

}

size_t var_loop(var* o, var_loop_f f, parg_t extra) {

    var_l d = { extra };

    o->d_work.f_loop = f;
    o->d_work.d_loop = &d;

    return key_loop(COR.data, _var_loop, NULL, NULL, o);

}

size_t var_hoop(var* o, var_loop_f f, parg_t extra) {

    var_l d = { extra };

    o->d_work.f_loop = f;
    o->d_work.d_loop = &d;

    return key_hoop(COR.data, _var_loop, NULL, NULL, o);

}

str_pool* var_str_pool(var* o) { return COR.pool; }

extern inline var* new_var(void);
extern inline var* new_var_c(pobj_t p_hold);
extern inline void del_var(var* o, pobj_t caller);
