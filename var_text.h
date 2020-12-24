#ifndef var_text_h
#define var_text_h

#include "var.h"
#include "str.h"

// load text to var and save var to str

enum { 
  
    var_save_quote_none,
    var_save_quote_text,    // quote text value
    var_save_quote_value,   // quote all types of value
    var_save_quote_all      // quote name and all types of value

};

// var_load() will load all values as text string

extern void var_load(var* data, text_t source, char quote_sign,                    char   mid_sign, char   end_sign);
extern void var_save(var* data, str*   target, char quote_sign, uint_t quote_type, text_t mid_text, text_t end_text);

#endif /* var_text_h */
