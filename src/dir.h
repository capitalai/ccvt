#ifndef dir_h
#define dir_h

#include "inc.h"

typedef struct dir_l dir_l;

typedef size_t dir_loop_f(dir_l* p);

struct dir_l {

    parg_t extra;
    text_t name;
    bool   is_dir;
    bool   is_file;

};

extern bool   dir_exist(text_t dir_name);

extern size_t dir_loop (text_t dir_name, dir_loop_f f, parg_t extra CAN_NULL);

#endif /* dir_h */
