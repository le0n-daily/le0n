#ifndef LE0N_UTIL_H
#define LE0N_UTIL_H

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <cstdint>

namespace le0n {
    
pid_t GetThreadId();
uint32_t GetFiberId();
}



#endif