#ifndef UNISTD_H
#define UNISTD_H

#if defined(_WIN32)
#include <io.h>
#include <process.h>
#include <direct.h>
#include <windows.h>

#define access _access
#define getcwd _getcwd
#define chdir _chdir
#define rmdir _rmdir
#define unlink _unlink

inline unsigned int sleep(unsigned int seconds) {
    Sleep(seconds * 1000);
    return 0;
}

// 修复 useconds_t 问题
#ifdef _WIN32
typedef unsigned long useconds_t;
#endif

inline int usleep(unsigned long usec) {  // 修改参数类型
    Sleep((usec + 999) / 1000);
    return 0;
}

#else
#include <unistd.h>
#endif

#endif // UNISTD_H