#include "lib.h"
#include "com.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define FAIL_SPACE  256
#define NOTE_SPACE  256

static take_f* fp_take = NULL;
static drop_f* fp_drop = NULL;
static note_f* fp_note = NULL;
static term_f* fp_term = NULL;
static fail_f* fp_fail = NULL;

static void*   dp_take = NULL;
static void*   dp_drop = NULL;
static void*   dp_note = NULL;
static void*   dp_term = NULL;
static void*   dp_fail = NULL;

bool new_take(take_f f, void* e) { ccf(fp_take); fp_take = f; dp_take = e; return true; }
bool new_drop(drop_f f, void* e) { ccf(fp_drop); fp_drop = f; dp_drop = e; return true; }
bool new_note(note_f f, void* e) { ccf(fp_note); fp_note = f; dp_note = e; return true; }
bool new_term(term_f f, void* e) { ccf(fp_term); fp_term = f; dp_term = e; return true; }
bool new_fail(fail_f f, void* e) { ccf(fp_fail); fp_fail = f; dp_fail = e; return true; }

pbuf_t take(size_t s) { return fp_take ? fp_take(s, dp_take) : _take(s); }
void   drop(pbuf_t d) { return fp_drop ? fp_drop(d, dp_drop) : _drop(d); }
int    note(text_t m) { return fp_note ? fp_note(m, dp_note) : _note(m); }
void   term(int r)    { return fp_term ? fp_term(r, dp_term) : _term(r); }

int    fail(text_t c, text_t v, text_t f, text_t m, int l) { return fp_fail ? fp_fail(c, v, f, m, l, dp_fail) : _fail(c, v, f, m, l); }

pbuf_t _take(size_t s) {

    czx(s);

    pbuf_t r = malloc(s); cxx(r);  // malloc() will not clear memory content

    return r;

}

void _drop(pbuf_t d) {

    free(d);  // do nothing if d == NULL

}

int _note(text_t m) {

    if(m) fputs(m, stderr);

    return 0;

}

void _term(int e) {

    exit(e);

}

int _fail(text_t c, text_t v, text_t f, text_t m, int l) {

    char b[FAIL_SPACE];

    size_t s = FAIL_SPACE - 1;

    if(errno) {

        size_t n = snprintf(b, s, "system error %d on %s (%s) in %s() at %s:%d - ", errno, c, v, f, m, l);

        strerror_r(errno, b + n, s - n);

        n = strnlen(b, s);

        if(n < s) { b[n] = '\n'; n++; b[n] = 0; }

    }
    else {

        snprintf(b, s, "program error on %s (%s) in %s() at %s:%d\n", c, v, f, m, l);

    }

    note(b);
    term(errno);

    return errno;

}

pbuf_t f_take(size_t s, void* x) { return take(s); }
void   f_drop(pbuf_t d, void* x) { return drop(d); }

size_t note_size(size_t s) {

    char b[NOTE_SPACE];

    snprintf(b, NOTE_SPACE - 1, "%zu", s);

    note(b);

    return s;

}

size_t text_size(text_t t, size_t s) {

    cxz(t);

    return strnlen(t, s ? s : SIZE_MAX - 1) + 1;

}

char* text_copy(char* t, text_t d, size_t s) {

    cxx(t);

    ifq(s, d) return t;

    strncpy(t, d, s);

    return t;

}

int text_comp(text_t a, text_t b, size_t s) {

    ifz(a) return b ? -1 : 0;
    ifz(b) return 1;

    return strncmp(a, b, s);

}

pbuf_t data_copy(pbuf_t t, data_t d, size_t s) {

    cxx(t);

    ifz(s) return t;

    if(d) return memmove(t, d, s);

    bzero(t, s);

    return t;

}

pbuf_t data_wipe(pbuf_t t, size_t s) {

    cxx(t);

    ifz(s) return t;

    bzero(t, s);

    return t;

}

int data_comp(data_t a, data_t b, size_t s) {

    ifz(a) return b ? -1 : 0;
    ifz(b) return 1;

    return memcmp(a, b, s);

}

// data_hash() is generic non-secure fast hash function for text string and binary data
// 
// reference: https://github.com/rurban/smhasher/issues/73 - Hash_Pippip()
//

#define _PADr_KAZE(x, n) (((x) << (n)) >> (n))

uint_t data_hash(data_t a, size_t s) {

    const uint_t PRIME  = 591798841;
    uint_t       hash32 = 2166136261;
    ulli_t       hash64 = 14695981039346656037ull;
    const char   *p     = a;
    
    size_t i, Cycles, NDhead;

    if(s > 8) {
    
        Cycles = ((s - 1) >> 4) + 1;
        NDhead = s - (Cycles << 3);

        for(i = 0; i < Cycles; i++) {
	    
            hash64 = (hash64 ^ (*(ulli_t *)(p))) * PRIME;        
	        hash64 = (hash64 ^ (*(ulli_t *)(p + NDhead))) * PRIME;        
	        p += 8;

        }

    } 
    else if(s < 8) {

        ulli_t t = 0; data_copy(&t, p, s);  // use data_copy() for safety
        
        hash64 = (hash64 ^ _PADr_KAZE(t, (8 - s) << 3)) * PRIME;        

    }
    else hash64 = (hash64 ^ *(ulli_t *)p) * PRIME;        

    hash32 = (uint_t)(hash64 ^ (hash64 >> 32));

    return hash32 ^ (hash32 >> 16);

}

void data_fill(pbuf_t a, size_t s, data_t b, size_t n) {

    cze(n);

    while(s >= n) { data_copy(a, b, n); a += n; s -= n; }

    if(s) data_copy(a, b, n - s);

}

// HASH_SEED:
//
//   for one of simple text hash function h(x + 1) = h(x) * HASH_SEED
//   most number in ((0 or 2 ^ n) + (1 or prime > 2)) > 12 are good hash seeds.
//   if only english dictionary words, 13 is good enough.
//   17 (x17), 31 or 131 (BKDR), 33 (DJB2) and 65599 (SDBM) is most fequently used.
//   in one test result, 273 and 1741 is better than these hash seeds and most other hash functions.
//

#define HASH_SEED 1741

uint_t text_hash(text_t a) {

    cxz(a);

    uint_t r = 0;

    // for very short text

    if(*a) {          r  = *a; a++; }
    if(*a) { r <<= 8; r += *a; a++; }
    if(*a) { r <<= 8; r += *a; a++; }
    if(*a) { r <<= 8; r += *a; a++; }

    while(*a) { r += *a * HASH_SEED; a++; }

    return r;

}

size_t text_type(void) {

    static size_t t = 0; ifz(t) t = text_hash("char*");

    return t;

}

int log2_m(size_t n) {

    // reference from bit-twiddling hacks

    int r = 0;

#if SIZE_LEVEL == 3
    if(n & 0xFFFFFFFF00000000) { n >>= 32; r |= 32; }
#endif

#if SIZE_LEVEL >= 2
    if(n & 0xFFFF0000)         { n >>= 16; r |= 16; }
#endif

    if(n & 0xFF00)             { n >>=  8; r |=  8; }
    if(n & 0xF0)               { n >>=  4; r |=  4; }
    if(n & 0xC)                { n >>=  2; r |=  2; }
    if(n & 0x2)                {           r |=  1; }

    return r;

}

extern inline int log2_n(size_t n);
extern inline int log2_a(size_t s);
extern inline int log2_b(size_t s);
