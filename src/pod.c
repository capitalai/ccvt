#include "lib.h"
#include "pod.h"
#include "sac.h"
#include "seq.h"

#define ppod_a(o) P(o, pod_a)

#define POD_DROP(n) ifx(n->a) { bag_drop(CNF.pick, n->self); n = NULL; }

typedef struct _pod          _pod;
typedef struct _pod_node     _pod_node;
typedef struct _pod_work     _pod_work;

struct _pod_node {  // pod node

    cap*       self;  // self, for bag_drop()
    _pod_node* ba;    // before a
    cap*       a;
    _pod_node* bb;    // before b
    cap*       b;
    _pod_node* bc;    // before c
    cap*       c;
    _pod_node* ac;    // after  c

};

struct _pod {

    seq*       data;
    _pod_node* root;

};

struct _pod_work {

    cap*       find_prev;  // (find, del) prev to found data
    cap*       find_next;  // (find, del) next to found data

    pod_p*     p;  // (add) return position

    data_t     a;  // (del) argument for compare

    cap*       d;  // (add) data to add
    cap*       m;  // (add) child middle item
    _pod_node* n;  // (add) node after child middle item

};

struct pod {  // pin is an obj

    obj           d_base;      // object data
    pod_a         d_args;      // object parameters
    _pod          d_core;      // core data
    _pod_work     d_work;      // data for functions

};

static inline _pod_node* _pod_new_node(pod* o) {

    cap*       t = bag_take(CNF.pick, sizeof(_pod_node), 0);
    _pod_node* r = cap_data(t);

    data_wipe(r, sizeof(_pod_node));

    r->self = t;

    return r;

}

size_t pod_size(void) { return sizeof(pod); }

size_t pod_type(void) { tid_make(pod); return tid(pod); }

size_t pod_pass(parg_t a) { 
    
    pod_a* p = a;

    cxz2(p->f_comp, p->f_find);

    seq_a  x = { sizeof(cap*), 0, 0, NULL, NULL };

    size_t s = seq_pass(&x); czz(s);

    return t_size(pod) + s;

}

pobj_t pod_init(pbuf_t b, parg_t a, pobj_t h) {

    cxx(a);

    pod_a* p_args = a;

    size_t p_size = pod_pass(p_args); czx(p_size);

    bag*   p_pick = h && pobj_pick(h) ? pobj_pick(h) : sac_init();

    char*  p_buf  = b ? b : cap_data(bag_take(p_pick, p_size, 0));

    pod*   o = (pod*)p_buf; p_buf += t_size(pod);  // use p_buf for pod object data

    CNF.size = sizeof(pod);
    CNF.type = pod_type();
    CNF.hold = h;
    CNF.pick = p_pick;
    CNF.fini = pod_fini;

    ARG = *p_args;

    OBJ.p_bind = ARG.p_bind;
    OBJ.o_head = NULL;
    OBJ.o_next = NULL;
    OBJ.o_prev = NULL;

    // all core fields should be set

    seq_a x = { sizeof(cap*), 0, 0, NULL, NULL };

    COR.data   = seq_init(p_buf, &x, o);  // use p_buf for seq object data
    COR.root   = _pod_new_node(o);

    if(h) obj_push(h, o);  // push to hold object

    obj_set_stat(o, OBJ_STAT_WORK);

    return o;

}

void pod_fini(pobj_t x) {

    cne(pobj_stat(x) != OBJ_STAT_NONE);

    obj_set_stat(x, OBJ_STAT_NONE);

    obj_fini(x);

}

size_t pod_count(pod* o) { return seq_count(COR.data); }

pod* new_pod_c(pod_comp_f f_comp, pod_find_f f_find, pod_push_f* f_push, pod_pull_f* f_pull, parg_t p_bind, pobj_t p_hold) {

    pod_a a = { f_comp, f_find, f_push, f_pull, p_bind };

    return pod_init(NULL, &a, p_hold);

}

pod_p* pod_head(pod* o, pod_p* p) {

    cxx(p);

    p->r = seq_head(COR.data);

    return p;

}

pod_p* pod_tail(pod* o, pod_p* p) {

    cxx(p);

    p->r = seq_tail(COR.data);

    return p;

}

// -------------------------------------------------------------------

static inline cap* _pod_new_item(pod* o) {

    cap*  x = ARG.f_push ? ARG.f_push(o, NULL, o->d_work.d) : o->d_work.d; cxx(x);

    cap*  t = seq_add(COR.data); 
    cap** p = cap_data(t); 

    *p = x;

    if(o->d_work.p) o->d_work.p->r = t;

    return t;

}

static inline bool _pod_set_item(pod* o, cap* t) {

    cxf(ARG.f_push);

    cap** p = cap_data(t);

    cap*  x = ARG.f_push(o, *p, o->d_work.d); cxf(x);

    *p = x;

    if(o->d_work.p) o->d_work.p->r = t;

    return true;

}

static inline _pod_node* _pod_split(pod* o, _pod_node* n) {

    _pod_node* t = _pod_new_node(o);

    t->ba = o->d_work.n; o->d_work.n = NULL;
    t->a  = n->b;        n->b        = NULL;
    t->bb = n->bc;       n->bc       = NULL;
    t->b  = n->c;        n->c        = NULL;
    t->bc = n->ac;       n->ac       = NULL;

    return t;

}

static inline int _pod_comp(pod* o, cap* d) {

    cap** p = cap_data(d);

    return ARG.f_comp(o, *p, o->d_work.d); 

}

static bool _pod_add(pod* o, _pod_node* n);

// -------------------------------------------------------------------

static inline bool _pod_add_ba(pod* o, _pod_node* n) {

    bool       t = _pod_add(o, n->ba); ifx(o->d_work.m) return t;  // add to bb

    _pod_node* x = _pod_split(o, n->ba);

    cap*       m = o->d_work.m;

    if(n->c) {

        o->d_work.n = n->bb;
        o->d_work.m = n->a;

    }
    else {

        o->d_work.m = NULL;

        if(n->b) {

            n->ac = n->bc;
            n->c  = n->b;

        }

        n->bc = n->bb;  // n->bc => n->bb => NULL
        n->b  = n->a;

    }

    n->bb = x;
    n->a  = m;

    return t;

}

static inline bool _pod_set_a(pod* o, _pod_node* n) {  // only happens when empty

    cap* t = _pod_new_item(o); cxf(t);

    n->a = t;

    return true;

}

static inline bool _pod_add_a(pod* o, _pod_node* n) {

    cap* t = _pod_new_item(o); cxf(t);

    seq_ins(COR.data, n->a, t);  // insert before a

    if(n->c) {

        o->d_work.n = n->bb;
        o->d_work.m = n->a;

    }
    else {

        if(n->b) {

            n->ac = n->bc;
            n->c  = n->b;

        }

        n->bc = n->bb;
        n->b  = n->a;

    }

    n->bb = NULL;  // n->bb => n->ba => NULL
    n->a  = t;

    return true;

}

static inline bool _pod_add_bb(pod* o, _pod_node* n) {

    bool       t = _pod_add(o, n->bb); ifx(o->d_work.m) return t;  // add to bb

    _pod_node* x = _pod_split(o, n->bb);

    if(n->c) {

        o->d_work.n  = x;  // after middle data

        // o->d_add_work.m: just move up

        return t;

    }

    if(n->b) {

        n->ac = n->bc;
        n->c  = n->b;

    }

    n->bc = x;
    n->b  = o->d_work.m; o->d_work.m = NULL;

    return t;

}

static inline bool _pod_set_b(pod* o, _pod_node* n) {  // only happens when empty

    cap* t = _pod_new_item(o); cxf(t);

    seq_put(COR.data, n->a, t);  // put after a

    n->b = t;

    return true;

}

static inline bool _pod_add_b(pod* o, _pod_node* n) {

    cap* t = _pod_new_item(o); cxf(t);

    seq_ins(COR.data, n->b, t);  // insert before b

    if(n->c) {

        o->d_work.n = NULL;  // n->bc => n->bb => NULL
        o->d_work.m = t;

    }
    else {

        n->ac = n->bc;
        n->c  = n->b;
        n->bc = NULL;  // n->bc => n->bb => NULL
        n->b  = t;

    }

    return true;

}

static inline bool _pod_add_bc(pod* o, _pod_node* n) {

    bool       t = _pod_add(o, n->bc); ifx(o->d_work.m) return t;  // add to bb

    _pod_node* x = _pod_split(o, n->bc);

    if(n->c) {

        cap*       m = o->d_work.m;

        o->d_work.n  = n->bc;  // after middle data
        o->d_work.m  = n->b;   // middle data

        n->bc            = x;
        n->b             = m;

        return t;

    }

    n->ac = x;
    n->c  = o->d_work.m; o->d_work.m = NULL;

    return t;

}

static inline bool _pod_set_c(pod* o, _pod_node* n) {

    cap* t = _pod_new_item(o); cxf(t);

    seq_put(COR.data, n->b, t);  // put after b

    n->c = t;

    return true;

}

static inline bool _pod_add_c(pod* o, _pod_node* n) {

    cap* t = _pod_new_item(o); cxf(t);

    seq_ins(COR.data, n->c, t);  // insert before c

    o->d_work.n  = n->bc;  // after middle data
    o->d_work.m  = n->b;   // middle data

    n->b         = t;      // n->bc is always NULL

    return true;

}

static inline bool _pod_add_ac(pod* o, _pod_node* n) {

    bool       t = _pod_add(o, n->ac); ifx(o->d_work.m) return t;  // add to bb

    _pod_node* x = _pod_split(o, n->ac);

    cap*       m = o->d_work.m;

    o->d_work.n  = n->bc;  // after middle data
    o->d_work.m  = n->b;   // middle data

    n->bc        = n->ac;
    n->b         = n->c;

    n->ac        = x;
    n->c         = m;

    return t;

}

static inline bool _pod_add_d(pod* o, _pod_node* n) {

    cap* t = _pod_new_item(o); cxf(t);

    seq_put(COR.data, n->c, t);  // put after c

    o->d_work.n  = n->bc;  // after middle data
    o->d_work.m  = n->b;   // middle data

    n->b         = n->c;
    n->bc        = n->ac;

    n->c         = t;

    return true;

}

// -------------------------------------------------------------------

static bool _pod_add(pod* o, _pod_node* n) {

    int        c;

    if(n->b) {

        c = _pod_comp(o, n->b);

        ifz(c) return _pod_set_item(o, n->b);  // equal to b

        if(c < 0) {  // b < d

            ifx(n->c) return n->bc ? _pod_add_bc(o, n) : _pod_set_c(o, n); 

            c = _pod_comp(o, n->c);

            ifz(c) return _pod_set_item(o, n->c);  // equal to c

            if(c < 0) return n->ac ? _pod_add_ac(o, n) : _pod_add_d(o, n);  // c < d

            return n->bc ? _pod_add_bc(o, n) : _pod_add_c(o, n); 

        }

    }
    
    ifx(n->a) return _pod_set_a(o, n);

    c = _pod_comp(o, n->a);

    ifz(c) return _pod_set_item(o, n->a);  // equal to a

    if(c < 0) return n->bb ? _pod_add_bb(o, n) : (n->b ? _pod_add_b(o, n) : _pod_set_b(o, n));  // a < d

    return n->ba ? _pod_add_ba(o, n) : _pod_add_a(o, n);

}

bool pod_add(pod* o, cap* d, pod_p* p) {

    cxf(d);

    o->d_work.p = p;
    o->d_work.d = d;
    o->d_work.m = NULL;
    o->d_work.n = NULL;

    bool t = _pod_add(o, COR.root); ifx(o->d_work.m) return t;

    _pod_node* x = _pod_split(o, COR.root);

    // create new root node

    _pod_node* n = _pod_new_node(o);

    n->bb = x;
    n->a  = o->d_work.m; o->d_work.m = NULL;
    n->ba = COR.root;

    COR.root = n;

    return t;

}

cap* pod_get(pod* o, pod_p* p) {

    cnx(p && p->r && seq_check(COR.data, p->r));

    cap** r = cap_data(p->r);

    return *r;

}

static cap* _pod_move_min(pod* o, _pod_node* n) {

    cap* r;

    if(n->ba) { r    = _pod_move_min(o, n->ba); POD_DROP(n->ba); return r; }
    
    r = n->a;

    if(n->bb) { n->a = _pod_move_min(o, n->bb); POD_DROP(n->bb); return r; }

    ifx(n->b) { n->a = NULL; return r; }

    n->a  = n->b;
    n->bb = n->bc;

    if(n->c) {

        n->b  = n->c;
        n->bc = n->ac;
        n->c  = NULL;
        n->ac = NULL;

    }
    else {

        n->b  = NULL;
        n->bc = NULL;

    }

    return r;

}

static cap* _pod_move_max(pod* o, _pod_node* n) {

    cap* r;

    if(n->ac) { r = _pod_move_max(o, n->ac); POD_DROP(n->ac); return r; }

    if(n->c)  { r = n->c; n->c = NULL; return r; }

    if(n->bc) { r = _pod_move_max(o, n->bc); POD_DROP(n->bc); return r; }

    if(n->b)  { r = n->b; n->b = NULL; return r; }

    if(n->bb) { r = _pod_move_max(o, n->bb); POD_DROP(n->bb); return r; }

    r = n->a; 

    if(n->ba) { n->a = _pod_move_max(o, n->ba); POD_DROP(n->ba); return r; } 
    
    n->a = NULL;

    return r;

}

static inline int _pod_find(pod* o, data_t a, cap* d) {

    cap** p = cap_data(d);
    
    int r = ARG.f_find(o, a, *p); 

    if     (r > 0) o->d_work.find_prev = d;
    else if(r < 0) o->d_work.find_next = d;

    return r;

}

static inline cap* _pod_pull(pod* o, data_t a, cap* b) {

    cap** p = cap_data(b);
    cap*  r = ARG.f_pull(o, a, *p);

    if(r) *p = r;

    return r;

}

static bool _pod_del(pod* o, _pod_node* n) {

    int  c;
    bool r;

    o->d_work.find_prev = NULL;
    o->d_work.find_next = NULL;

    if(n->b) {

        c = _pod_find(o, o->d_work.a, n->b);

        ifz(c) { 

            if(ARG.f_pull && _pod_pull(o, o->d_work.a, n->b)) return true;

            r = seq_del(COR.data, n->b);

            if(n->bc)      { n->b = _pod_move_min(o, n->bc); POD_DROP(n->bc); }
            else if(n->bb) { n->b = _pod_move_max(o, n->bb); POD_DROP(n->bb); }
            else           { n->b = n->c; n->bc = n->ac; n->c = NULL; n->ac = NULL; }

            return r;

        }

        if(c > 0) {

            if(n->c) {

                c = _pod_find(o, o->d_work.a, n->c);

                ifz(c) { 

                    if(ARG.f_pull && _pod_pull(o, o->d_work.a, n->c)) return true;

                    r = seq_del(COR.data, n->c);

                    if(n->ac)      { n->c = _pod_move_min(o, n->ac); POD_DROP(n->ac); }
                    else if(n->bc) { n->c = _pod_move_max(o, n->bc); POD_DROP(n->bc); }
                    else             n->c = NULL;

                    return r;

                }

                if(c > 0) { 

                    cxf(n->ac);

                    cnf(_pod_del(o, n->ac));

                    POD_DROP(n->ac);

                    return true;

                }

            }
            
            cxf(n->bc);

            cnf(_pod_del(o, n->bc));

            POD_DROP(n->bc);

            return true;

        }

    }

    cxf(n->a);

    c = _pod_find(o, o->d_work.a, n->a);

    ifz(c) { 

        if(ARG.f_pull && _pod_pull(o, o->d_work.a, n->a)) return true;

        r = seq_del(COR.data, n->a);

        if(n->bb)      { n->a = _pod_move_min(o, n->bb); POD_DROP(n->bb); }
        else if(n->ba) { n->a = _pod_move_max(o, n->ba); POD_DROP(n->ba); }
        else if(n->b) {

            n->ba = n->bb;
            n->a  = n->b;
            n->bb = n->bc;

            if(n->c) {

                n->b  = n->c;  
                n->bc = n->ac; 
                n->c  = NULL;
                n->ac = NULL;

            }
            else {

                n->b  = NULL;
                n->bc = NULL;

            }

        }
        else n->a = NULL;

        return r;

    }

    if(c > 0) {

        cxf(n->bb);
        
        cnf(_pod_del(o, n->bb));

        POD_DROP(n->bb);

        return true;

    }

    cxf(n->ba);
    
    cnf(_pod_del(o, n->ba));

    POD_DROP(n->ba);

    return true;

}

bool pod_del(pod* o, data_t a) {

    o->d_work.a = a;

    return _pod_del(o, COR.root);

}

bool pod_prev(pod* o, pod_p* p) {

    cnf(p && p->r);

    cap* t = seq_prev(COR.data, p->r); cxf(t);

    p->r = t;

    return true;

}

bool pod_next(pod* o, pod_p* p) {

    cnf(p && p->r);

    cap* t = seq_next(COR.data, p->r); cxf(t);

    p->r = t;

    return true;

}

bool pod_find(pod* o, pod_p* p, data_t a) {

    cxf(ARG.f_find);

    cap*       t = NULL;
    _pod_node* n = COR.root;
    int        x;

    o->d_work.find_prev = NULL;
    o->d_work.find_next = NULL;

    while(n) {

        if(n->b) {

            x = _pod_find(o, a, n->b); ifz(x) { t = n->b; break; }

            if(x > 0) { 

                if(n->c) {

                    x = _pod_find(o, a, n->c); ifz(x) { t = n->c; break; }

                    if(x > 0) { n = n->ac; continue; }

                }

                n = n->bc; continue; 
                
            }

        }

        cxb(n->a);

        x = _pod_find(o, a, n->a); ifz(x) { t = n->a; break; }

        n = x > 0 ? n->bb : n->ba; continue;

    }

    if(t) { if(p) p->r = t; return true; }

    return false;

}

int pod_find_prev(pod* o, pod_p* p, data_t a) {

    ccz(pod_find(o, p, a));

    ifx(o->d_work.find_prev) return -1;
    
    if(p) p->r = o->d_work.find_prev;

    return 1;

}

int pod_find_next(pod* o, pod_p* p, data_t a) {

    ccz(pod_find(o, p, a));

    ifx(o->d_work.find_next) return -1;
    
    if(p) p->r = o->d_work.find_next;

    return 1;

}

size_t pod_loop(pod* o, pod_loop_f f, data_t start, data_t stop, parg_t extra) {

    pod_p  p1 = { NULL };
    pod_p  p2 = { NULL };
    pod_l  l  = { extra, &p1 };
    size_t r  = 0;

    if(start) pod_find_next(o, &p1, start); else pod_head(o, &p1);
    if(stop)  pod_find_prev(o, &p2, stop);  else pod_tail(o, &p2);

    bool   c = seq_check(COR.data, p1.r);

    while(c) {

        cap* t = l.p->r;

        r += f(o, &l); 
        
        cxb(l.p);
        ccb(t == p2.r);  // at stop position

        if(t != l.p->r) c = pod_check(o, l.p);
        else            c = pod_next (o, l.p);

    }

    return r;

}

size_t pod_hoop(pod* o, pod_loop_f f, data_t start, data_t stop, parg_t extra) {

    pod_p  p1 = { NULL };
    pod_p  p2 = { NULL };
    pod_l  l  = { extra, &p1 };
    size_t r  = 0;

    if(start) pod_find_prev(o, &p1, start); else pod_tail(o, &p1);
    if(stop)  pod_find_next(o, &p2, stop);  else pod_head(o, &p2);

    bool   c = seq_check(COR.data, p1.r);

    while(c) {

        cap* t = l.p->r;

        r += f(o, &l); 
        
        cxb(l.p);
        ccb(t == p2.r);  // at stop position

        if(t != l.p->r) c = pod_check(o, l.p);
        else            c = pod_prev (o, l.p);

    }

    return r;

}

bool pod_check(pod* o, pod_p* p) { return p && seq_check(COR.data, p->r); }

static void _pod_dump(pod* o, pod_dump_f f, _pod_node* n, size_t d) {

    cap** p;

    if(n->ba) _pod_dump(o, f, n->ba, d + 1);
    if(n->a) { p = cap_data(n->a); f(*p, d); }
    if(n->bb) _pod_dump(o, f, n->bb, d + 1);
    if(n->b) { p = cap_data(n->b); f(*p, d); }
    if(n->bc) _pod_dump(o, f, n->bc, d + 1);
    if(n->c) { p = cap_data(n->c); f(*p, d); }
    if(n->ac) _pod_dump(o, f, n->ac, d + 1);
    f(NULL, d);

}

void pod_dump(pod* o, pod_dump_f f) {

    _pod_dump(o, f, COR.root, 0);

}

extern inline pod* new_pod(pod_comp_f f_comp, pod_find_f f_find);
extern inline void del_pod(pod* o, pobj_t caller);
