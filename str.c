#include "lib.h"
#include "str.h"
#include "sac.h"
#include "obj.h"

#if     SIZE_LEVEL == 1

#define DEF_STR_SIZE 16

#else
#if     SIZE_LEVEL == 2

#define DEF_STR_SIZE 32

#else

#define DEF_STR_SIZE 64

#endif
#endif

#define DEF_STR_ADD_RATE 4

struct str_pool {

    bag*   b;
    size_t s;
    pobj_t h;

};

struct str {

    cap*   data;
    cap*   self;
    size_t length;

};

static size_t def_str_size     = DEF_STR_SIZE;
static size_t def_str_add_rate = DEF_STR_ADD_RATE;

size_t str_def_size(size_t new_size) {

    if(new_size) def_str_size = MAX(new_size, DEF_STR_SIZE);

    return def_str_size;

}

size_t str_add_def_rate(size_t new_rate) {

  if(new_rate) def_str_add_rate = new_rate;

  return def_str_add_rate;

}

str_pool* str_pool_init(size_t s, pobj_t h) {

    bag* pick = h ? pobj_pick(h) : sac_init();

    str_pool* t = cap_data(bag_take(pick, t_size(str_pool), 0));

    t->b = pick;
    t->s = MAX(s, def_str_size);
    t->h = h;

    return t;

}

void str_pool_fini(str_pool* p) {

    cce(p->h);

    bag_fini(p->b);

}

str* str_init(str_pool* p, text_t source) {

    cap* x = bag_take(p->b, t_size(str), 0);
    str* s = cap_data(x);

    size_t n = source ? text_size(source, SIZE_MAX) : 0; if(n < p->s) n = p->s;

    s->data = bag_take(p->b, d_size(n), 0);
    s->self = x;

    cap_set_hold(s->data, p->h);

    char* t = cap_data(s->data);

    if(source) { 
        
        text_copy(t, source, n);
        s->length = n - 1;

    }
    else {

        t[0] = 0;
        s->length = 0;

    }

    return s;

}

void str_fini(str* s) {

    bag* p = cap_pick(s->data);

    bag_drop(p, s->data);
    bag_drop(p, s->self);

}

size_t str_size   (str* s) { return cap_size(s->data); }
char*  str_data   (str* s) { return cap_data(s->data); }
size_t str_length (str* s) { return s->length; }

void   str_refresh(str* s) { s->length = text_size(cap_data(s->data), cap_size(s->data)) - 1; }
void   str_clear  (str* s) { char* d = cap_data(s->data); d[0] = 0; s->length = 0; }

str* str_ask(str* s, size_t n) {

    size_t m  = str_size(s); ifn(n > m) return s;

    size_t n1 = d_size(n);

    bag*   b  = cap_pick(s->data);

    cap*   t  = bag_take(b, n1, 0);

    text_copy(cap_data(t), cap_data(s->data), n1);

    cap_set_hold(t, cap_hold(s->data));

    bag_drop(b, s->data);

    s->data = t;

    return s;

}

str* str_add(str* s, text_t d, size_t n) {

    ifx(d) return s;

    size_t bs = cap_size(s->data);
    size_t s1 = s->length;
    size_t s2 = text_size(d, n);
    size_t ss = s1 + s2;

    if(ss > bs) {

        size_t n = CEIL(ss, def_str_size); n += n / def_str_add_rate;

        str_ask(s, n * def_str_size);

    }

    char*  b = cap_data(s->data);

    text_copy(b + s1, d, s2);

    s->length = ss - 1;

    return s;

}

str* str_set(str* s, text_t d, size_t n) {

    ifx(d) { str_clear(s); return s; }

    size_t bs  = cap_size(s->data);
    size_t ts  = text_size(d, n);
    size_t len = ts - 1;

    if(ts > bs) {

        str_clear(s);

        size_t c = CEIL(ts, def_str_size); c += c / def_str_add_rate;

        str_ask(s, c * def_str_size);

    }

    char*  b = cap_data(s->data);

    text_copy(b, d, len);

    b[len] = 0;

    s->length = len;

    return s;

}
