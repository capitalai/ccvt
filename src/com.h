#ifndef com_h
#define com_h

#include "inc.h"

// com: control, output and memory functions

// basic functions
//
// take: allocate memory
// drop: free memory
// note: show system message
// term: terminate program
// fail: report error and terminate program

extern pbuf_t take(size_t s);
extern void   drop(pbuf_t d);
extern int    note(text_t m);
extern void   term(int r);
extern int    fail(text_t c, text_t v, text_t f, text_t m, int l);

// user can replace these functions once

typedef pbuf_t take_f(size_t s, void* e);
typedef void   drop_f(pbuf_t d, void* e);
typedef int    note_f(text_t m, void* e);
typedef void   term_f(int    r, void* e);
typedef int    fail_f(text_t c, text_t v, text_t f, text_t m, int l, void* e);  // cause, varible, function, module, line

extern bool new_take(take_f f, void* e);
extern bool new_drop(drop_f f, void* e);
extern bool new_note(note_f f, void* e);
extern bool new_term(term_f f, void* e);
extern bool new_fail(fail_f f, void* e);

// system functions without user replacement

extern pbuf_t _take(size_t s);
extern void   _drop(void*  d);
extern int    _note(text_t m);
extern void   _term(int    r);
extern int    _fail(text_t c, text_t v, text_t f, text_t m, int l);

// typed version of function

extern pbuf_t f_take(size_t s, void* x);
extern void   f_drop(pbuf_t d, void* x);

// derived function

extern size_t note_size(size_t s);

// FAIL(cause string, varible)
// TEST(test object, cause string, varible): test null pointer on new object
// TAKE(data pointer, data size): take with test null

#define FAIL(c, v)       fail(c, #v, __func__, __FILE__, __LINE__)

#define TEST(p, c, v)    if(p == NULL) { FAIL(c, v); return NULL; }

#define TAKE(v, s)       v = take(s); TEST(v, "take", s)

// function for memory processing

// text_size: return text space size (length + 1)
//            return 0 if t == NULL
//            s = 0 => s = SIZE_MAX - 1
//            return last size if t == last t and s = 0
//            return size > s is string too long

extern size_t text_size(text_t t, size_t s);
extern char*  text_copy(char*  t, text_t d, size_t s);
extern int    text_comp(text_t a, text_t b, size_t s);
extern uint_t text_hash(text_t a);
extern size_t text_type(void);

extern pbuf_t data_copy(pbuf_t t, data_t d, size_t s);
extern pbuf_t data_wipe(pbuf_t t, size_t s);
extern int    data_comp(data_t a, data_t b, size_t s);
extern uint_t data_hash(data_t a, size_t s);
extern void   data_fill(pbuf_t a, size_t s, data_t b, size_t n);

#define DATA_WIPE(d, t) data_wipe(d, sizeof(t))

// function for memory mangement
//
// log2_m: msb(n), msb = most significant bit
// log2_n: msb(round to next power of 2)
// log2_a: adjust log2_m(n) to (r ? (r + 1) : 0), space size s can store 2^(r-1) data
// log2_b: adjust log2_n(n) to (r ? (r + 1) : 0), data  size s require   2^(r-1) space

//  i    m    n    a    b
//  0    0    0    0    0
//  1    0    0    1    1
//  2    1    1    2    2
//  3    1    2    2    3
//  4    2    2    3    3
//  5    2    3    3    4
//  6    2    3    3    4
//  7    2    3    3    4
//  8    3    3    4    4
//  9    3    4    4    5
// 10    3    4    4    5
// 11    3    4    4    5
// 12    3    4    4    5
// 13    3    4    4    5
// 14    3    4    4    5
// 15    3    4    4    5
// 16    4    4    5    5

extern int log2_m(size_t n);
inline int log2_n(size_t n) { int r = log2_m(n); return (n ^ (1 << r)) ? r + 1 : r; }
inline int log2_a(size_t s) { return s ? log2_m(s) + 1 : 0; }
inline int log2_b(size_t s) { return s ? log2_n(s) + 1 : 0; }

#endif /* com_h */
