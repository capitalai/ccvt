#include <sys/stat.h>
#include <dirent.h>

#include "dir.h"

bool dir_exist(text_t dir_name) {

    struct stat st;

    if(stat(dir_name, &st) < 0 || (st.st_mode & S_IFDIR) == 0) return false;

    return true;

}

size_t dir_loop(text_t dir_name, dir_loop_f f, parg_t extra) {

    DIR*           dir;
    struct dirent* dirent;
    dir_l          d = { extra };
    size_t         r = 0;
    
    dir = opendir(dir_name);
    
    if(dir) {
    
        while(true) {
            
            dirent = readdir(dir); if(dirent == NULL) break;
            
            d.name     = dirent->d_name;
            d.is_dir   = dirent->d_type & DT_DIR;
            d.is_file  = dirent->d_type & DT_REG;

            r += f(&d);
            
        }
        
        closedir(dir);

    }
    
    return r;

}

