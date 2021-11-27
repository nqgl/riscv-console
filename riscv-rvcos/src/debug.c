#include "debug.h"
#include "string.h"
#include <unistd.h>
#include <fcntl.h>


void debug_log_info(char* cstr){
    write(2, cstr, strlen(cstr));
}


void debug_log_info1(char* cstr, int len){
    int file = open("/home/g/sc/ecs150/examplesavefilehelp", O_APPEND);
    write(file, cstr, len);
    close(file);
}