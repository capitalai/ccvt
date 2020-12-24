#include "lib.h"
#include "lot.h"

#define plot(o) P(o, lot)

typedef struct lot lot;
typedef struct _lot_node _lot_node;

struct _lot_node {

    cap        d_cap;

    _lot_node* prev;   // previous _lot_node
    _lot_node* next;   // next _lot_node

};

static const size_t _lot_node_s = t_size(_lot_node);

struct lot {  // lot is a bag

    bag        d_bag;  // bag  data

    _lot_node* head;   // head of cap list

};

static cap* lot_take(bag* b, size_t s, size_t h);
static bool lot_drop(bag* b, cap*   p);
static void lot_fini(bag* b);

bag* lot_init(void) {

    lot* b; TAKE(b, sizeof(lot));

    b->d_bag.take   = lot_take;
    b->d_bag.drop   = lot_drop;
    b->d_bag.fini   = lot_fini;
    b->d_bag.type   = lot_type();
    b->d_bag.s_base = _lot_node_s;
    b->head         = NULL;

    return (bag*)b;

}

static void lot_fini(bag* b) {

    lot* o = (lot*)b;

    while(o->head) {
    
        _lot_node* p = o->head;

        o->head = p->next;

        drop(p);
    
    }

    drop(b);  // drop itself

}

size_t lot_type(void) { tid_make(lot_type); return tid(lot_type); }

extern inline bool is_lot(bag* b);

// functions

static cap* lot_take(bag* b, size_t s, size_t h) {

    czx(s);

    size_t s1 = d_size(s);
    size_t s2 = s1 + b->s_base + h;

    _lot_node* p; TAKE(p, s2);

    lot*    o = (lot*)b;

    p->d_cap.s_head = b->s_base + h;
    p->d_cap.s_data = s1;
    p->d_cap.pick   = b;

    p->next = o->head; if(p->next) p->next->prev = p;
    p->prev = NULL;
    o->head = p;

    return (cap*)p;

}

static bool lot_drop(bag* b, cap* p) {

    cxf(p); 

    ccf(p->pick != b);

    _lot_node* t = (_lot_node*)p;

    lot*    o = (lot*)b;

    if(t->prev) t->prev->next = t->next;
    if(t->next) t->next->prev = t->prev;

    if(o->head == t) o->head = t->next;

    drop(p);

    return true;

}
