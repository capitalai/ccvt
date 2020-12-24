
#include <sys/stat.h>
#include <stdio.h>

#include "lib.h"
#include "com.h"
#include "str_file.h"

#define STR_LOAD_BUFFER 1024

static void _str_load_stdin(str* s) {

    char b[STR_LOAD_BUFFER + 1];

    run {

        size_t r = fread(b, 1, STR_LOAD_BUFFER, stdin); czb(r);

        b[r] = 0;

        str_add(s, b, 0);

        if(r < STR_LOAD_BUFFER) break;

    }

}

bool str_load(str* s, text_t source_file) {

    if(source_file == NULL) {

        str_clear(s);

        _str_load_stdin(s);

        return true;

    }

    struct stat st;

    if(stat(source_file, &st) < 0 || (st.st_mode & S_IFREG) == 0 || st.st_size == 0) return false;

    FILE* f = fopen(source_file, "r"); if(f == NULL) return false;

    str_clear(s);

    str_ask(s, st.st_size);

    char* d = str_data(s);

    fread(d, 1, st.st_size, f); 
    
    d[st.st_size] = 0;

    fclose(f);

    str_refresh(s);

    return true;

}

bool str_save(str* s, text_t target_file) { 

    FILE* f;

    if(target_file) { 
      
        f = fopen(target_file, "w"); 
    
        if(f == NULL) return false;

    }
    else f = stdout;

    size_t r = fwrite(str_data(s), str_length(s), 1, f);

    if(f != stdout) fclose(f);

    return r > 0;

}
