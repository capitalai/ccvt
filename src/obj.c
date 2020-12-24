#include "lib.h"
#include "obj.h"

bool obj_set_bind(pobj_t o, parg_t p) {

    ccf(pobj_bind(o));

    pobj_bind(o) = p;

    return true;

}

bool obj_set_stat(pobj_t o, size_t s) {

    pobj_stat(o) = s;

    return true;

}

void obj_push(pobj_t o, pobj_t p) {

    cxe(p);

    if(pobj_hold(p) != o || pobj_prev(p) != NULL || pobj_next(p) != NULL) return;

    pobj_t a = pobj_head(o);

    pobj_head(o) = p;
    pobj_next(p) = a;

    if(a) pobj_prev(a) = p;

}

bool obj_pull(pobj_t o, pobj_t p) {

    cxf(p);
    ccf(pobj_hold(p) != o);

    if(pobj_head(o) == p) pobj_head(o) = pobj_next(p);

    ccf(pobj_prev(p) == NULL && pobj_next(p) == NULL);

    if(pobj_prev(p)) pobj_next(pobj_prev(p)) = pobj_next(p);
    if(pobj_next(p)) pobj_prev(pobj_next(p)) = pobj_prev(p);

    pobj_prev(p) = NULL;
    pobj_next(p) = NULL;

    return true;

}

void obj_fini(pobj_t o) {

    if(pobj_fini(o) && pobj_stat(o) != OBJ_STAT_NONE) pobj_fini(o)(o); 

    obj_set_stat(o, OBJ_STAT_NONE);

    // finishing child objects

    pobj_t h = pobj_head(o);

    while(h) {

        obj_pull(o, h);
        obj_fini(h);

        h = pobj_head(o);

    }

    // finishing local memory allocator

    ifn(pobj_pick(o) == NULL || (pobj_hold(o) && pobj_pick(pobj_hold(o)) == pobj_pick(o))) {

        bag_fini(pobj_pick(o));

    }

}

extern inline size_t obj_size(pobj_t o);
extern inline size_t obj_type(pobj_t o);
