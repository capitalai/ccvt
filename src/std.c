#include "std.h"
#include "lib.h"
#include "sac.h"
#include "str.h"

#include <stdlib.h>
#include <time.h>

std_a std = { false, sac_init, 0 };

void std_init(void) {

    static bool done = false; cce(done);

    if(std.rand)          srand((unsigned)time(NULL));
    if(std.new_bag)       set_bag_init(std.new_bag);
    if(std.sac_pack_size) sac_pack_def_size(std.sac_pack_size);
    if(std.str_size)      str_def_size(std.str_size);
    if(std.str_add_rate)  str_add_def_rate(std.str_add_rate);

    done = true;

}
