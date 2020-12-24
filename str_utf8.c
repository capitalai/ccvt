#include "lib.h"
#include "str_utf8.h"

#define UTF8_S2 0xC0
#define UTF8_E2 0xDF
#define UTF8_S3 0xE0
#define UTF8_E3 0xEF
#define UTF8_S4 0xF0
#define UTF8_E4 0xF7
#define UTF8_SX 0x80
#define UTF8_EX 0xBF

static size_t utf8_next(text_t t, size_t p) {

    t += p;

    byte_t  c = (byte_t)*t;  // byte 0
    byte_t  e;

    if(c == 0) return p;

    p++;

    if(c <  UTF8_SX || c > UTF8_E4) return p;  // as ASCII character

    if(c <= UTF8_EX) return 0;  // c >= UTF8_S2 if not return

    t++;

    e = (byte_t)*t; if(e < UTF8_SX || e > UTF8_EX) return 0;  // byte 1

    p++;

    if(c <= UTF8_E2) return p;  // c >= UTF8_S3 if not return

    t++;

    e = (byte_t)*t; if(e < UTF8_SX || e > UTF8_EX) return 0;  // byte 2

    p++;

    if(c <= UTF8_E3) return p;  // c >= UTF8_S4 if not return

    t++;

    e = (byte_t)*t; if(e < UTF8_SX || e > UTF8_EX) return 0;  // byte 3

    p++;

    return p;

}

str* str_utf8_set(str* s, text_t t, size_t n) {

    size_t x = str_size(s) - 1;
    char*  d = str_data(s);

    size_t p = 0;
    size_t q = 0;
    size_t i = 0;
    size_t j = 0;

    while(i < n && j < x) {

        q = utf8_next(t, p); ccb(p == q);

        if(q) {
        
            i++;

            while(p < q) { d[j] = t[p]; j++; p++; }

        }
        else p++;
        
    }

    d[j] = 0;

    str_refresh(s);

    return s;

}

bool str_utf8_test(str* s) {

    size_t x = str_length(s);
    char*  d = str_data(s);

    size_t p = 0;

    while(p < x) {

        p = utf8_next(d, p); czf(p);

    }

  return true;

}

size_t str_utf8_next(str* s, size_t p) {

    size_t x = str_length(s); 
    
    if(p >= x) return x;
    
    return utf8_next(str_data(s), p);

}

size_t str_utf8_length(str* s) {

    size_t x = str_length(s);
    char*  d = str_data(s);

    size_t p = 0;
    size_t q = 0;
    size_t i = 0;

    while(p < x) {

        q = utf8_next(d, p); 
        
        if(q) { p = q; i++; } else p++;

    }

    return i;

}

size_t str_utf8_loop(str* s, str_utf8_loop_fun f, parg_t extra) {

    str_utf8_l l = { extra };
    size_t     x = str_length(s);
    char*      d = str_data(s);

    size_t p = 0;
    size_t q = 0;
    size_t i = 0;
    size_t r = 0;

    while(p < x) {

        q = utf8_next(d, p); 
        
        if(q) {

            l.s = p;
            l.n = q;
            l.i = i;

            r += f(s, &l);

            p = l.n;

            i++;

        }
        else p++;

    }

    return r;

}
