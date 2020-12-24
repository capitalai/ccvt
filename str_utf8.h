#ifndef str_utf8_h
#define str_utf8_h

#include "str.h"

// utf8 functions for str

typedef struct str_utf8_l str_utf8_l;

typedef size_t str_utf8_loop_fun(str* s, str_utf8_l* p);

struct str_utf8_l {

    parg_t e;  // extra argument for loop function
    size_t s;  // start position
    size_t n;  // next  position, set new value will be used on next loop
    size_t i;  // index

};

extern str*   str_utf8_set   (str* s, text_t t, size_t n);  // set str max to n utf8 chars (ignore invalid utf8 chars)

extern bool   str_utf8_test  (str* s);                      // test all valid utf8 chars

extern size_t str_utf8_next  (str* s, size_t p);            // get next utf8 char pos, return 0 if invalid utf8 char

extern size_t str_utf8_length(str* s);                      // get length of utf8 char (ignore invalid utf8 chars)

extern size_t str_utf8_loop  (str* s, str_utf8_loop_fun f, parg_t extra CAN_NULL);

#endif /* str_utf8_h */
