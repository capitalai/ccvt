#include "lib.h"
#include "arg.h"
#include "txt.h"

typedef struct arg arg;

struct arg {

    bool loaded;

    txt* ext;

    key* arg_name;
    key* set_name;

    var* arg;
    var* set;
    var* opt;

};

static arg arg_data = { false, NULL, NULL, NULL, NULL, NULL };

void arg_init(void) {

    cce(arg_data.ext);

    arg_data.loaded   = false;
    arg_data.ext      = new_txt();
    arg_data.arg_name = key_init(NULL, NULL, arg_data.ext);
    arg_data.set_name = key_init(NULL, NULL, arg_data.ext);
    arg_data.arg      = var_init(NULL, NULL, arg_data.ext);
    arg_data.set      = var_init(NULL, NULL, arg_data.ext);
    arg_data.opt      = var_init(NULL, NULL, arg_data.ext);

}

void arg_add_arg(text_t n) {

    cce(arg_data.loaded);

    key_add(arg_data.arg_name, n);

}

void arg_add_set(text_t n) {

    cce(arg_data.loaded);

    key_add(arg_data.set_name, n);

}

void arg_load(int argc, const char* argv[]) {

    cce(arg_data.loaded);

    cap*   p = key_head(arg_data.arg_name);
    text_t n = p ? key_name(arg_data.arg_name, p) : NULL;

    for(int i = 1; i < argc; i++) {

        if(argv[i][0] == '-') {

            if(key_get(arg_data.set_name, argv[i] + 1) && i < argc - 1 && argv[i + 1][0] != '-') {

                var_set_str(arg_data.set, argv[i] + 1, argv[i + 1]);

                i++;

            }
            else var_set_bool(arg_data.opt, argv[i] + 1, true);

        }
        else if(n) {

            var_set_str(arg_data.arg, n, argv[i]);

            p = key_next(arg_data.arg_name, p);
            n = p ? key_name(arg_data.arg_name, p) : NULL;

        }
        else txt_add(arg_data.ext, NULL, argv[i]);

    }

    arg_data.loaded = true;

}

str* arg_arg(text_t n) {

    cnx(arg_data.loaded);

    return var_get_str(arg_data.arg, n);

}

str* arg_set(text_t n) {

    cnx(arg_data.loaded);

    return var_get_str(arg_data.set, n);

}

bool arg_opt(text_t n) {

    cnf(arg_data.loaded);

    val* v = var_get_bool(arg_data.opt, n);

    return v ? v->v_bool : false;

}

str* arg_ext(size_t n) {

    cnx(arg_data.loaded);

    return txt_get(arg_data.ext, n);

}

size_t arg_ext_count(void) {

    cnz(arg_data.loaded);

    return txt_count(arg_data.ext);

}

void arg_fini(void) {

    cne(arg_data.loaded);

    del_txt(arg_data.ext, NULL);

    arg_data.loaded   = false;
    arg_data.ext      = NULL;
    arg_data.arg_name = NULL;
    arg_data.set_name = NULL;
    arg_data.arg      = NULL;
    arg_data.set      = NULL;
    arg_data.opt      = NULL;

}
