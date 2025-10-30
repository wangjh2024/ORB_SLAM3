#ifndef G2O_OS_SPECIFIC_HH_
#define G2O_OS_SPECIFIC_HH_

#ifdef _WIN32
    #include <windows.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <stdarg.h>
    #include <time.h>
    
    #define drand48() ((double) rand()/(double)RAND_MAX)

    // 只声明不定义，避免重复定义
    #ifdef __cplusplus
    extern "C" {
    #endif
    int vasprintf(char** strp, const char* fmt, va_list ap);
    #ifdef __cplusplus
    }
    #endif

#else
    #include <sys/time.h>
    #include <unistd.h>
    #define UNIX 1
#endif

#endif