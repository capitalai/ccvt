#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ccl.h"
#include "ccvt.h"

#ifndef CCVT_DATA_DIR
#define CCVT_DATA_DIR "/usr/local/share/ccvt"
#endif

#define DIR_SIZE 1024

static void usage(void);
static bool is_id(text_t n);
static bool find_data_dir(char* data_dir, text_t convert_mode);

int main(int argc, const char * argv[]) {

    text_t convert_mode = "tc";
    str*   convert_mode_str;
    char   data_dir[DIR_SIZE];
    bool   console_mode = false;
    size_t file_count   = 0;

    // arguments processing

    arg_init();
    arg_add_set("m");
    arg_load(argc, argv);

    if(arg_opt("c")) console_mode = true;

    file_count = arg_ext_count();

    if(arg_opt("h") || (console_mode == false && file_count == 0)) { usage(); term(0); }

    convert_mode_str = arg_set("m"); 
    
    if(convert_mode_str) {
        
        convert_mode = str_data(convert_mode_str);

        if(is_id(convert_mode) == false) { note("convert mode '"); note(convert_mode); note("'is invalid\n"); term(0); }

    }

    if(find_data_dir(data_dir, convert_mode) == false) { note("convert mode '"); note(convert_mode); note("' is not existed"); term(0); }

    // main work

    ccvt_init(data_dir);

    if(console_mode) ccvt_work(NULL);
    
    else for(size_t i = 0; i < file_count; i++) {

        str*   file_name_str  = arg_ext(i);

        if(file_name_str == NULL) continue;

        text_t file_name     = str_data(file_name_str);

        if(file_name) ccvt_work(file_name);

    }

    ccvt_fini();

    arg_fini();

    return 0;
}

static void usage(void) { 

    note("chinese converter for utf-8 encoding\n\n");
    note("ccvt [-h] [-c] [-m convert_mode] [ files... ]\n");
    note("  -h: help\n");
    note("  -c: input text from console\n");

}

static bool is_id(text_t n) {

    text_t check = "abcdefghijklmnopqrstuvwxyz0123456789_";

    if(n == NULL || *n == 0) return false;

    int i = 0;

    while(i < 32 && *n && strchr(check, *n)) { n++; i++; }

    if(*n) return false;

    return true;

}

static bool find_data_dir(char* data_dir, text_t convert_mode) {

    text_t env = getenv("ccvt");

    text_copy(data_dir, env ? env : CCVT_DATA_DIR, DIR_SIZE);

    size_t len = strlen(data_dir);
    
    data_dir[len] = '/';

    text_copy(data_dir + len + 1, convert_mode, DIR_SIZE - len - 1);

    return dir_exist(data_dir);

}

