#ifndef lot_h
#define lot_h

#include "bag.h"

// lot: a bag can provide and release caps

extern bag*   lot_init(void);
extern size_t lot_type(void);
inline bool   is_lot(bag* b) { return bag_type(b) == lot_type(); }

#endif /* lot_h */
