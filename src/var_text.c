
#include <stdio.h>

#include "lib.h"
#include "var_text.h"

typedef struct var_save_data var_save_data;

struct var_save_data {

    str*   target;
    char   quote_sign;
    uint_t quote_type;
    text_t mid_text;
    text_t end_text;

};

static size_t _var_load_quote(str* t, text_t s, size_t p, char q) {

    size_t i = p + 1;
    size_t n = 0;

    while(s[i] && s[i] != q) {

        if(s[i] == '\\') { i++; n++; }

        if(s[i] != 0) i++;

    }

    i++;

    size_t c = i - p - n - 2 + 1;

    str_clear(t);

    str_ask(t, c);

    char*  b = str_data(t);
    text_t x = s + p + 1;

    while(*x && *x != q) {

        if(*x == '\\') x++; 

        if(*x != 0) { *b = *x; b++; x++; }

    }

    *b = 0;

    str_refresh(t);

    return i;

}

static size_t _var_load_value(str* t, text_t s, size_t p, char e) {

    size_t i = p;

    while(s[i] && s[i] != e && s[i] != ' ' && s[i] != '\t' && s[i] != '\n') i++;

    if(i > p) str_set(t, s + p, i - p); else str_clear(t);

    return i;

}

void var_load(var* data, text_t source, char quote_sign, char mid_sign, char end_sign) {

    str_pool* p = var_str_pool(data);
    str*      n = str_init(p, NULL);
    str*      v = str_init(p, NULL);
    size_t    i = 0;

    while(source[i]) {

        while(source[i] == ' ' || source[i] == '\t' || source[i] == '\n') i++;

        if(source[i] == quote_sign) i = _var_load_quote(n, source, i, quote_sign);
        else                        i = _var_load_value(n, source, i, mid_sign);

        while(source[i] && source[i] != mid_sign && source[i] != end_sign) i++; czb(source[i]); 

        if(source[i] == end_sign) { i++; continue; }
        
        i++;

        while(source[i] == ' ' || source[i] == '\t' || source[i] == '\n') i++;

        if(source[i] == quote_sign) i = _var_load_quote(v, source, i, quote_sign);
        else                        i = _var_load_value(v, source, i, end_sign);

        var_set_str(data, str_data(n), str_data(v));

        while(source[i] && source[i] != end_sign) i++; czb(source[i]); 

        i++;

    }

    str_fini(v);
    str_fini(n);

}

static void _var_save_quote(str* t, text_t d, char q) {

    size_t n = 0;
    size_t i = 0;

    while(d[i]) { if(d[i] == q || d[i] == '\\') n++; i++; }

    size_t len = str_length(t);

    str_ask(t, len + n + i + 2 + 1);

    char*  b = str_data(t);
    size_t p = len;

    b[p] = q; p++;

    for(size_t j = 0; d[j]; j++) {
        
        if(d[j] == q || d[j] == '\\') { b[p] = '\\'; p++; }

        b[p] = d[j]; p++;

    }

    b[p] = q; p++;
    b[p] = 0;

    str_refresh(t);

}

static size_t _var_save_loop(var* o, var_l* p) {

    var_save_data* d = p->e;

    char q[] = { d->quote_sign,  0 };

    if(d->quote_type == var_save_quote_all) {

        str_add(d->target, q,    0);
        str_add(d->target, p->n, 0);
        str_add(d->target, q,    0);

    }
    else str_add(d->target, p->n, 0);

    str_add(d->target, d->mid_text, 0);

    bool qv = d->quote_type == var_save_quote_all || d->quote_type == var_save_quote_value;
    bool qs = d->quote_type != var_save_quote_none;

    char b[64];

    switch(p->t) {

        case var_type_bool: 

            if(qv) str_add(d->target, q, 0);
            str_add(d->target, p->v->v_bool ? "true" : "false", 0);
            if(qv) str_add(d->target, q, 0);
        
        break;

        case var_type_int: 

            if(qv) str_add(d->target, q, 0);
            sprintf(b, "%llu", p->v->v_ulli);
            str_add(d->target, b, 0);
            if(qv) str_add(d->target, q, 0);
        
        break;

        case var_type_num: 

            if(qv) str_add(d->target, q, 0);
            sprintf(b, "%Lf", p->v->v_real);
            str_add(d->target, b, 0);
            if(qv) str_add(d->target, q, 0);
        
        break;

        case var_type_str:

            if(qs) _var_save_quote(d->target, str_data(p->s), d->quote_sign);
            else   str_add(d->target, str_data(p->s), 0);

        break;

    }

    str_add(d->target, d->end_text, 0);

    return 1;

}

void var_save(var* data, str* target, char quote_sign, uint_t quote_type, text_t mid_text, text_t end_text) {

    var_save_data d = { target, quote_sign, quote_type, mid_text ? mid_text : ": ", end_text ? end_text : "\n" };

    var_loop(data, _var_save_loop, &d);

}
