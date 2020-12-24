#ifndef arg_h
#define arg_h

#include "key.h"
#include "var.h"

// arg: arguments processing

extern void   arg_init(void);

extern void   arg_add_arg(text_t n);  // add named argument before arg_load()
extern void   arg_add_set(text_t n);  // add named setting  before arg_load()

extern void   arg_load(int argc, const char* argv[]);  // processing arguments

extern str*   arg_arg(text_t n);    // named argument
extern str*   arg_set(text_t n);    // -name value
extern bool   arg_opt(text_t n);    // -name
extern str*   arg_ext(size_t n);    // get arguments without name
extern size_t arg_ext_count(void);  // get count of arguments without name

extern void   arg_fini(void);

#endif /* arg_h */
