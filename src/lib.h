#ifndef lib_h
#define lib_h

#include <stdlib.h>
#include <sys/types.h>

// lib: macros for ccl source code

#define CCL_CODE

// macro definitions

#define until(a)  while(!(a))
#define run       while(true)

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define DIV(a, b)  ((b) ? (a) / (b) : (a))
#define CEIL(a, b) ((b) ? ((a) + (b) - 1) / (b) : (a))

// special macros

// - object

#define OBJ       (o)->d_base
#define ARG       (o)->d_args
#define COR       (o)->d_core
#define CNF       (o)->d_base.d_conf

// - others

#define P(v, t)   ((t*)v)

// extended controls

#define ifd(s, p) if((s)  > 0 && (p) != NULL)
#define ifn(a)    if(!(a))
#define ifq(s, p) if((s) == 0 || (p) == NULL)
#define ifx(a)    if((a) == NULL)
#define ifz(a)    if((a) == 0)

#define ifx2(a, b) if((a) == NULL || (b) == NULL)

#define ccb(a) if(a) break
#define cce(a) if(a) return
#define ccf(a) if(a) return false
#define ccx(a) if(a) return NULL
#define ccz(a) if(a) return 0

#define cxb(a) ifx(a) break
#define cxc(a) ifx(a) continue
#define cxe(a) ifx(a) return
#define cxf(a) ifx(a) return false
#define cxr(a) ifx(a) return r
#define cxx(a) ifx(a) return NULL
#define cxz(a) ifx(a) return 0

#define cxz2(a, b) ifx2(a, b) return 0

#define czb(a) ifz(a) break
#define cze(a) ifz(a) return
#define czf(a) ifz(a) return false
#define czr(a) ifz(a) return r
#define czx(a) ifz(a) return NULL
#define czz(a) ifz(a) return 0

#define cnb(a) ifn(a) break
#define cne(a) ifn(a) return
#define cnf(a) ifn(a) return false
#define cnx(a) ifn(a) return NULL
#define cnz(a) ifn(a) return 0

#define cqx(s, p) ifq(s, p)  return NULL

// type id macros

#define TID_A(n)       PAD(n, _a)
#define TID_C(n)       PAD(n, _c)()
#define TID_S(n)       sizeof(TID_A(n))
#define TID_H(n)       text_hash(STR(TID_A(n)))
#define TID_P(p, n)    P(p, TID_A(n))

#define tid(n)         PAD(n, _id)
#define tid_make(n)    static size_t tid(n) = 0; if(tid(n) == 0) tid(n) = TID_H(n)
#define tid_test(p, n) if(TID_P(p, n)->s != TID_S(n) || TID_P(p, n)->t != TID_C(n)) return NULL

// constants

#define data_s sizeof(data_t)

#if     SIZE_MAX == UINT16_MAX
#define SIZE_LEVEL  1
#define SIZE_BITS  16
#define d_size(n)  ((n) + ((n) & 1))
#else
#if     SIZE_MAX == UINT32_MAX
#define SIZE_LEVEL  2
#define SIZE_BITS  32
#define d_size(n)  ((n) + (size_t)((-(ssize_t)(n)) & 3))
#else
#if     SIZE_MAX == UINT64_MAX
#define SIZE_LEVEL  3
#define SIZE_BITS  64
#define d_size(n)  ((n) + (size_t)((-(ssize_t)(n)) & 7))
#else
#error (lib_ccl) not vaild size of size_t
#endif
#endif
#endif

#define SIZE_LIMIT   ((SIZE_MAX) >> 2)

#define t_size(t)    d_size(sizeof(t))
#define e_size(t, n) (t_size(t) + d_size(n))

// naming rules:
//
// - varible
//
//  c_xxx: count
//  d_xxx: structure data
//  f_xxx: function
//  i_xxx: index, id
//  o_xxx: object
//  p_xxx: pointer
//  s_xxx: size_t type
//
//  p f d > c i o > s
//
// - argument
//
//  xxx_a: argument type
//  xxx_b: macro    for building argument data
//  xxx_c: function for taking argument type id
//
// - function
//
//  xxx_c: complex
//  xxx_r: recursive function
//
// - type
//
//  xxx_n: node
//  xxx_l: loop
//  xxx_f: function
//  xxx_p: path
//  xxx_v: visit node
//  xxx_d: data
//
// - constant
//
//  xxx_s: size of type xxx
//

#endif /* lib_h */
