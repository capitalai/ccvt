#ifndef str_file_h
#define str_file_h

#include "str.h"

// load from file and save to file

// source is null will input from standard input  device (console)
// target is null will output to  standard output device (console)

extern bool str_load(str* s, text_t source_file CAN_NULL);  // load data from file
extern bool str_save(str* s, text_t target_file CAN_NULL);  // save data to   file

#endif /* str_file_h */
