#include "lib.h"
#include "seq.h"
#include "sac.h"
#include "pin.h"

typedef struct _seq      _seq;
typedef struct _seq_node _seq_node;

#define SEQ_STAT_USED 0
#define SEQ_STAT_FREE 1
#define SEQ_STAT_FAIL 2

#define SEQ_NODE(p) ((_seq_node*)pin_head_data(COR.data, p))

struct _seq_node {

    cap*   prev;
    cap*   next;
    size_t stat;  // 0 = used, 1 = free, 2 = init fail

};

static const size_t seq_n_s = t_size(_seq_node);

struct _seq {

    pin*   data;
    cap*   head;
    cap*   tail;
    cap*   free;  // free list only has next, no prev
    size_t count;

};

struct seq {  // pin is an obj

    obj    d_base;    // object data
    seq_a  d_args;    // object parameters
    _seq   d_core;    // core data

};

static seq_a def_seq_a = { 0, 0, 0, NULL, NULL };

size_t seq_size(void) { return sizeof(seq); }

size_t seq_type(void) { tid_make(seq); return tid(seq); }

size_t seq_pass(parg_t a) { 

    seq_a* p = a;

    pin_a  x = { p->s_item, 0, p->s_head };

    size_t s = pin_pass(&x); czz(s);

    return t_size(seq) + s;

}

pobj_t seq_init(pbuf_t b, parg_t a, pobj_t h) {

    seq_a* p_args = a ? a : &def_seq_a;

    size_t p_size = seq_pass(p_args); czx(p_size);

    bag*   p_pick = h && pobj_pick(h) ? pobj_pick(h) : sac_init();

    char*  p_buf  = b ? b : cap_data(bag_take(p_pick, p_size, 0));

    seq* o = (seq*)p_buf; p_buf += t_size(seq);  // use p_buf for seq object data

    CNF.size = sizeof(seq);
    CNF.type = seq_type();
    CNF.hold = h;
    CNF.pick = p_pick;
    CNF.fini = seq_fini;

    OBJ.p_bind = NULL;
    OBJ.o_head = NULL;
    OBJ.o_next = NULL;
    OBJ.o_prev = NULL;

    ARG = *p_args;

    ifz(ARG.s_item) ARG.s_item = data_s;

    // all core fields should be set

    pin_a x = { ARG.s_item, ARG.t_item, seq_n_s + ARG.s_head };

    COR.data   = pin_init(p_buf, &x, o);  // use p_buf for pin object data
    COR.head   = NULL;
    COR.tail   = NULL;
    COR.free   = NULL;
    COR.count  = 0;

    if(h) obj_push(h, o);  // push to hold object

    obj_set_stat(o, OBJ_STAT_WORK);

    return o;

}

static size_t _seq_loop_fini(pin* x, pin_l* p) {

    _seq_node* t = pin_head_data(x, p->d); cnz(t->stat == SEQ_STAT_USED);
  
    seq*       o = (seq*)pobj_hold(x);
    
    ARG.f_fini(o, p->d); 

    return 1;

}

void seq_fini(pobj_t x) {

    cne(pobj_stat(x) != OBJ_STAT_NONE);

    obj_set_stat(x, OBJ_STAT_NONE);

    seq* o = (seq*)x;

    if(ARG.f_fini) {

        pin_l a = { false, NULL, SIZE_MAX, NULL };  // finish items from last created

        pin_hoop(COR.data, _seq_loop_fini, &a);   

    }

    obj_fini(x);

}

bool   seq_full     (seq* o) { return COR.count == SIZE_MAX; }
size_t seq_count    (seq* o) { return COR.count; }
size_t seq_data_size(seq* o) { return ARG.s_item; }
size_t seq_data_type(seq* o) { return ARG.t_item; }

seq* new_seq_c(size_t s_item, size_t t_item, size_t s_head, seq_init_f* f_init, seq_fini_f* f_fini, pobj_t p_hold) {

    seq_a p = { s_item, t_item, s_head, f_init, f_fini };

    return seq_init(NULL, &p, p_hold);

}

// functions

cap* seq_add(seq* o) {

    cap*       r;
    _seq_node* p; 

    if(COR.free) {

        r = COR.free;

        p = SEQ_NODE(r);

        COR.free = p->next;

    }
    else {

        r = pin_ask(COR.data, NULL, 0, sizeof(_seq_node));

        p = SEQ_NODE(r);

    }

    p->next = NULL;
    p->prev = COR.tail;
    p->stat = SEQ_STAT_USED;

    ifx(COR.head) COR.head = r;

    if(COR.tail) SEQ_NODE(COR.tail)->next = r;

    COR.tail = r;

    COR.count++;

    if(ARG.f_init && ARG.f_init(o, r) == false) {

        p->stat = SEQ_STAT_FAIL;
        
        seq_del(o, r);

        return NULL;

    }

    cap_set_hold(r, o);

    return r;  

}

static bool _seq_remove(seq* o, cap* d) {

    _seq_node* p = SEQ_NODE(d);

    ccf(p->stat == SEQ_STAT_FREE);  // already free

    if(p->prev) SEQ_NODE(p->prev)->next = p->next; else COR.head = p->next;
    if(p->next) SEQ_NODE(p->next)->prev = p->prev; else COR.tail = p->prev;

    return true;

}

bool seq_del(seq* o, cap* d) {

    cxf(d);

    _seq_node* p = SEQ_NODE(d);

    if(ARG.f_fini && p->stat == SEQ_STAT_USED) ARG.f_fini(o, d);

    _seq_remove(o, d);

    p->prev = NULL;
    p->next = COR.free;
    p->stat = SEQ_STAT_FREE;

    COR.free = d;

    COR.count--;

    return true;

}

cap* seq_get(seq* o, cap* d) {

    cxx(d); 
    
    _seq_node* p = SEQ_NODE(d);

    cnx(p->stat == SEQ_STAT_USED);

    return d;

}

bool seq_put(seq* o, cap* a, cap* d) {

    cnf(a && d && _seq_remove(o, d));

    _seq_node *p = SEQ_NODE(a),
              *q = SEQ_NODE(d);
    
    q->next = p->next; 
    
    if(q->next) SEQ_NODE(q->next)->prev = d;
    
    q->prev = a;
    p->next = d;

    ifx(q->next) COR.tail = d;

    return true;

}

bool seq_ins(seq* o, cap* b, cap* d) {

    cnf(b && d && _seq_remove(o, d));

    _seq_node *p = SEQ_NODE(b),
              *q = SEQ_NODE(d);
    
    q->next = b;
    q->prev = p->prev;

    if(q->prev) SEQ_NODE(q->prev)->next = d;

    p->prev = d;

    ifx(q->prev) COR.head = d;

    return true;

}

cap* seq_head(seq* o) { return COR.head; }
cap* seq_tail(seq* o) { return COR.tail; }

cap* seq_prev (seq* o, cap* d) {

    cxx(d);

    _seq_node* p = SEQ_NODE(d); cnx(p->stat == SEQ_STAT_USED);

    return p->prev;

}

cap* seq_next (seq* o, cap* d) {

    cxx(d);

    _seq_node* p = SEQ_NODE(d); cnx(p->stat == SEQ_STAT_USED);

    return p->next;

}

size_t seq_loop (seq* o, seq_loop_f f, cap* start, cap* stop, parg_t extra) {

    seq_l  d = { extra, start };
    size_t r = 0;

    if(d.d == NULL) d.d = COR.head;

    while(d.d) {

        cap* t = d.d;

        r += f(o, &d);

        ccb(t == stop);

        if(t == d.d) {

          _seq_node* h = SEQ_NODE(t);

          d.d = h->next;

        }

    }

    return r;

}

size_t seq_hoop (seq* o, seq_loop_f f, cap* start, cap* stop, parg_t extra) {

    seq_l  d = { extra, start };

    size_t r = 0;

    if(d.d == NULL) d.d = COR.tail;

    while(d.d) {

        cap* t = d.d;

        r += f(o, &d);

        ccb(t == stop);
        
        if(t == d.d) {

          _seq_node* h = SEQ_NODE(t);

          d.d = h->prev;

        }

    }

    return r;

}

size_t seq_head_size(seq* o, cap* d) {

    cxz(d);

    return pin_head_size(COR.data, d) - seq_n_s;

}

pbuf_t seq_head_data(seq* o, cap* d) {

    cxx(d);

    return (char*)SEQ_NODE(d) + seq_n_s;

}

bool seq_check(seq* o, cap* d) {

    cnf(d && cap_hold(d) == o);

    _seq_node* p = SEQ_NODE(d); 
    
    return p->stat == SEQ_STAT_USED;

}

extern inline seq* new_seq(void);
extern inline seq* new_seq_s(size_t s_item);
extern inline void del_seq(seq* o, pobj_t caller);
