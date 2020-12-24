#include "lib.h"
#include "sac.h"
#include "lot.h"

#define psac(o) P(o, sac)

#if     SIZE_LEVEL == 1

#define DEF_SAC_PACK_SIZE   4096
#define MIN_SAC_PACK_SIZE    256

#else
#if     SIZE_LEVEL == 2

#define DEF_SAC_PACK_SIZE  32768
#define MIN_SAC_PACK_SIZE    512

#else

#define DEF_SAC_PACK_SIZE 262144
#define MIN_SAC_PACK_SIZE   4096

#endif
#endif

#define SAC_UNIT    data_s
#define FREE_COUNT  SIZE_BITS
#define TINY_SIZE   SIZE_BITS

typedef struct sac       sac;
typedef struct _sac      _sac;
typedef struct _sac_node _sac_node;

struct _sac_node {

    cap        d_cap;

    _sac_node* pack_n;   // next      _sac_node of pack
    _sac_node* pack_p;   // previous  _sac_node of pack
    _sac_node* free_n;   // next free _sac_node of slot
    _sac_node* free_p;   // next free _sac_node of slot
    
    size_t     free_id;  // non-zero is free

};

static const size_t _sac_node_s = t_size(_sac_node);

struct sac {  // sac is a bag

    bag         d_bag;

    bag*        pick;
    _sac_node*  slot[FREE_COUNT];  // slot[n] -> 2^n unit size free space node stack
    size_t      pack_size;
    size_t      map;               // slot map on bits
    size_t      now;               // now node
    size_t      count;             // free pack count

};

static size_t def_sac_pack_size = DEF_SAC_PACK_SIZE;

size_t sac_type(void) { tid_make(sac); return tid(sac); }

static inline size_t _sac_slot_a (size_t s_space) { return log2_a(s_space) - 1; }   // s_space cannot be zero
static inline size_t _sac_slot_b (size_t s_data)  { return log2_b(s_data)  - 1; }   // s_data  cannot be zero

static cap*   sac_take(bag* b, size_t s, size_t h);
static bool   sac_drop(bag* b, cap*   p);
static void   sac_fini(bag* b);

bag*   sac_init_c(size_t s_pack) {

    bag* pick = lot_init();

    ifz(s_pack) s_pack = def_sac_pack_size;

    s_pack = MAX(s_pack, MIN_SAC_PACK_SIZE);

    sac* b = cap_data(bag_take(pick, t_size(sac), 0));

    b->d_bag.take   = sac_take;
    b->d_bag.drop   = sac_drop;
    b->d_bag.fini   = sac_fini;
    b->d_bag.type   = sac_type();
    b->d_bag.s_base = _sac_node_s;

    b->pick      = pick;

    data_wipe(b->slot, sizeof(_sac_node*) * FREE_COUNT);
    
    b->pack_size = s_pack;
    b->map       = 0;
    b->now       = FREE_COUNT - 1;
    b->count     = 0;

    return (bag*)b;

}

bag* sac_init(void) { return sac_init_c(0); }

size_t sac_pack_size(bag* b) { 
    
    cnz(is_sac(b));

    return psac(b)->pack_size; 
    
}

size_t sac_pack_def_size(size_t new_size) {

    if(new_size) def_sac_pack_size = MAX(new_size, MIN_SAC_PACK_SIZE);

    return def_sac_pack_size;

}

extern inline bool is_sac(bag* b);

static void _sac_del_slot(sac* o, _sac_node* p) {  // remove pack in free list and slot

    if(p->free_p) {  
        
        p->free_p->free_n = p->free_n;

        if(p->free_n) p->free_n->free_p = p->free_p;

    } 
    else {  // no prev free -> head of free list -> slot

        size_t c = _sac_slot_a(p->d_cap.s_data);

        o->slot[c] = p->free_n;

        if(p->free_n) p->free_n->free_p = NULL; else o->map &= ~(1 << c);

    }

    p->free_n = NULL;
    p->free_p = NULL;

}

static void _sac_add_slot(sac* o, _sac_node* p) {  // push pack into free list and slot

    size_t c = _sac_slot_a(p->d_cap.s_data);

    _sac_node* n = o->slot[c];
    
    p->free_n  = n;
    p->free_p  = NULL;
    p->free_id = ++o->count;

    if(n) n->free_p = p;

    o->map      |= 1 << c;
    o->now      = c;
    o->slot[c]  = p;

}

static void _sac_make_free(sac* o, _sac_node* a, size_t p, size_t s) {

    _sac_node* b = (_sac_node*)((char*)a + p);

    b->d_cap.s_head = o->d_bag.s_base;
    b->d_cap.s_data = s;
    b->d_cap.pick   = o;

    b->pack_n = a->pack_n;
    
    if(a->pack_n) a->pack_n->pack_p = b;

    b->pack_p = a;
    a->pack_n = b;

    _sac_add_slot(o, b);  // it will fill free_n and free_p

}

static cap* _sac_take(sac* o, size_t s, size_t h) {

    size_t     s1 = s + h; 
    size_t     s2 = s1 + o->d_bag.s_base;
    size_t     s3 = MAX(s2, o->pack_size);

    cap*       p = bag_take(o->pick, o->d_bag.s_base + (s3 > s2 ? s3 : s1), 0);
    _sac_node* t = cap_data(p);

    t->d_cap.s_head = o->d_bag.s_base + h;
    t->d_cap.s_data = s;
    t->d_cap.pick   = o;
    
    t->pack_n  = NULL;
    t->pack_p  = NULL;
    t->free_n  = NULL;
    t->free_p  = NULL;
    t->free_id = 0;     // used

    if(s3 > s2) _sac_make_free(o, t, s2, s3 - s2);

    return (cap*)t;

}

static cap* sac_take(bag* b, size_t s, size_t h) {

    czx(s);

    sac* o = (sac*)b;

    s = d_size(s);
    h = d_size(h);

    size_t s1 = s + h;
   
    _sac_node* p = s1 > TINY_SIZE ? o->slot[o->now] : NULL;

    if(p == NULL || p->d_cap.s_data < s1) {

        size_t n = _sac_slot_b(s1);
        size_t c = 1 << n;

        while((c & o->map) == 0 && n < FREE_COUNT) { n++; c <<= 1; }  // find slot

        if(n == FREE_COUNT) return _sac_take(o, s, h); 
        
        p = o->slot[n];

    }

    _sac_del_slot(o, p);

    // split

    size_t s2 = s1 + o->d_bag.s_base;
    size_t s3 = p->d_cap.s_data;

    p->d_cap.s_head += h;

    if(s3 > s2) {
        
        p->d_cap.s_data  = s;

        _sac_make_free(o, p, s2, s3 - s2);
    
    }
    else p->d_cap.s_data -= h;

    p->free_id = 0;  // used

    return (cap*)p;

}

static inline void _sac_merge_next(_sac_node* p) {

    p->d_cap.s_data += p->pack_n->d_cap.s_head + p->pack_n->d_cap.s_data;

    p->pack_n = p->pack_n->pack_n;

    if(p->pack_n) p->pack_n->pack_p = p;

}

static bool sac_drop(bag* b, cap* p) {

    cnf(p && p->pick == b);

    _sac_node* t = (_sac_node*)p; ccf(t->free_id);  // id != 0 -> free

    sac*    o = (sac*)b;

    if(t->d_cap.s_head > o->d_bag.s_base) {  // restore head size

        t->d_cap.s_data += t->d_cap.s_head - o->d_bag.s_base;
        t->d_cap.s_head  = o->d_bag.s_base;

    }

    while(t->pack_n && t->pack_n->free_id) {  // next pack is free

        _sac_del_slot(o, t->pack_n);  // remove pack in free list

        _sac_merge_next(t);

    }

    while(t->pack_p && t->pack_p->free_id) {  // previous pack is free

        t = t->pack_p;

        _sac_del_slot(o, t);  // remove pack in free list

        _sac_merge_next(t);

    }

    _sac_add_slot(o, t);  // push to new slot list

    return true;

}

static void sac_fini(bag* b) { 
    
    bag_fini(psac(b)->pick);

}
