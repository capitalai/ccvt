
#include <string.h>
#include "ccl.h"
#include "ccvt.h"

#define CONVERT_FILE       "ccvt.dat"
#define CORRECT_FILE_EXT   ".conf"
#define SKIP_WORD_FILE     "skip.txt"
#define MAX_CORRECT_LENGTH 16

static var* convert;
static var* correct;
static var* correct_len;

static str* conf_path;
static str* file_path;
static str* file_data;
static str* source_str;
static str* target_str;
static str* correct_str;
static str* key_word;
static str* source_word;
static str* skip_word;
static str* correct_word[MAX_CORRECT_LENGTH + 1];

void ccvt_init(const char* data_dir);
void ccvt_fini(void);
void ccvt_work(const char* file);

static void   load_convert(void);
static size_t load_correct(dir_l* p);
static void   load_correct_len(void);
static void   load_skip_word(void);
static void   correct_words(str* source, str* target);

static size_t _load_correct_len(var* o, var_l* p);
static bool   _is_skip_word(text_t s);
static int    _str_cmp_end(text_t a, text_t b);

void ccvt_init(const char* data_dir) {
    
    convert     = new_var();
    correct     = new_var();
    correct_len = new_var();

    str_pool* pool = var_str_pool(convert);
    
    conf_path   = str_init(pool, NULL);
    file_path   = str_init(pool, NULL);
    file_data   = str_init(pool, NULL);
    source_str  = str_init(pool, NULL);
    target_str  = str_init(pool, NULL);
    correct_str = str_init(pool, NULL);
    key_word    = str_init(pool, NULL);
    source_word = str_init(pool, NULL);
    skip_word   = str_init(pool, NULL);
    
    for(int i = 0; i < MAX_CORRECT_LENGTH + 1; i++) correct_word[i] = str_init(pool, NULL);

    str_set(conf_path, data_dir, 0);
    str_add(conf_path, "/", 0);

    load_convert();

    dir_loop(data_dir, load_correct, NULL);
    
    load_correct_len();
    
    load_skip_word();

}

void ccvt_fini() {
    
    del_var(correct_len, NULL);
    del_var(correct, NULL);
    del_var(convert, NULL);
    
}

void ccvt_work(const char* filename) {
    
    str_load(source_str, filename);

    str*   target_word;
    
    char*  source = str_data(source_str);
    
    size_t p = 0;
    size_t q = 0;
    
    while(source[p]) {
        
        q = str_utf8_next(source_str, p);

        if(q == 0) { p++; continue; } // broken utf8 char

        if(q - p > 1) {

            str_set(source_word, source + p, q - p);

            target_word = var_get_str(convert, str_data(source_word));
            
            if(target_word) str_add(target_str, str_data(target_word), 0);
            else            str_add(target_str, source + p, q - p);

        }
        else str_add(target_str, source + p, 1);
        
        p = q;
        
    }
    
    correct_words(target_str, correct_str);
    
    str_save(correct_str, NULL);

}

static void load_convert(void) {
    
    str_set(file_path, str_data(conf_path), 0);
    str_add(file_path, CONVERT_FILE, 0);
    
    str_load(file_data, str_data(file_path));
    
    if(str_length(file_data) == 0) {
        
        note("convert data file '");
        note(str_data(file_path));
        note("' is not found. you can set ccvt env as convert data file's directory\n");
        
        term(0);
        
    }

    var_load(convert, str_data(file_data), '\'', ':', '\n');
    
}

static size_t load_correct(dir_l* p) {

    if(_str_cmp_end(p->name, CORRECT_FILE_EXT) == 0) {
        
        str_set(file_path, str_data(conf_path), 0);
        str_add(file_path, p->name, 0);
        
        // note("load "); note(str_data(file_path)); note("\n");

        if(str_load(file_data, str_data(file_path))) var_load(correct, str_data(file_data), '\'', ':', '\n');
        
    }
    
    return 1;
    
}

static void load_correct_len() {
    
    var_loop(correct, _load_correct_len, NULL);

}

static void load_skip_word(void) {
    
    str_set(file_path, str_data(conf_path), 0);
    str_add(file_path, SKIP_WORD_FILE, 0);

    // note("load "); note(str_data(file_path)); note("\n");

    str_load(skip_word, str_data(file_path));
    
}

static void correct_words(str* source, str* target) {

    char*  d  = str_data(source);
    size_t p  = 0;
    str*   w  = correct_word[0];
    str*   w1 = correct_word[1];
    text_t x  = str_data(w);

    while(d[p]) {
        
        size_t q = str_utf8_next(source, p);
        
        if(q == 0) { p++; continue; }  // broken utf8 char

        str_set(w, d + p, q - p);  // one utf8 char
        
        if(_is_skip_word(x)) { str_add(target, x, 0); p = q; continue; }
        
        val* v = var_get_int(correct_len, x);
        
        if(v == NULL)        { str_add(target, x, 0); p = q; continue; }
        
        size_t len_max = (size_t)v->v_ulli; if(len_max > MAX_CORRECT_LENGTH) len_max = MAX_CORRECT_LENGTH;
        size_t len_now = 2;

        for(size_t i = len_now; i <= len_max; i++) str_set(correct_word[i], x, 0);  // make a, a, a ...
        
        size_t c = q;
        
        while(d[c] && len_now <= len_max) {
            
            q = str_utf8_next(source, c);

            if(q == 0) break;  // broken utf8 char

            str_set(w1, d + c, q - c);

            text_t x1 = str_data(w1);
            
            if(_is_skip_word(x1)) break;
            
            for(size_t i = len_now; i <= len_max; i++) str_add(correct_word[i], x1, 0);  // make ab, abc, abcd ...
            
            c = q;
            
            len_now++;
            
        }
        
        len_now--;
        
        str*   r = NULL;
        size_t n = len_now;
        
        while(n > 1) {
            
            r = var_get_str(correct, str_data(correct_word[n]));
            
            if(r) break;
            
            n--;
            
        }
        
        if(r) {
            
            str_add(target, str_data(r), 0); p += str_length(correct_word[n]);
            
            // note("add "); note(str_data(correct_word[n])); note(" => "); note(str_data(r)); note("\n");
            
        }
        else {
            
            str_add(target, x, 0); p += str_length(w); // note("add "); note(x); note("\n");
            
        }
        
    }
     
    // str_set(target, str_data(source), 0);
    
}

static size_t _load_correct_len(var* o, var_l* p) {
    
    str_set(key_word, p->n, 0);

    size_t p1 = str_utf8_next(key_word, 0); if(p1 == 0) return 1;

    char   k[5]; text_copy(k, p->n, p1); k[p1] = 0;  // p1 < 5

    size_t v = str_utf8_length(key_word);

    val*   x = var_get_int(correct_len, k);
   
    if(x == NULL || x->v_ulli < v) {
        
        var_set_int(correct_len, k, v);
    
    }
        
    return 1;
    
}

static int _str_cmp_end(text_t a, text_t b) {
    
    size_t la = strlen(a);
    size_t lb = strlen(b);
    
    if(lb > la) return b[0];
    
    return strcmp(a + la - lb, b);
    
}

static bool _is_skip_word(text_t s) {
    
    return strstr(str_data(skip_word), s) != NULL;
    
}
