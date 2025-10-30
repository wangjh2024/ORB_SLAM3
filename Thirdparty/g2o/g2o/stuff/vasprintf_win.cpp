#ifdef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern "C" int vasprintf(char** strp, const char* fmt, va_list ap) {
    if (!strp || !fmt) return -1;
    
    int size = _vscprintf(fmt, ap);
    if (size < 0) return -1;
    
    *strp = (char*)malloc(size + 1);
    if (*strp == NULL) return -1;
    
    int result = vsprintf_s(*strp, size + 1, fmt, ap);
    if (result < 0) {
        free(*strp);
        *strp = NULL;
        return -1;
    }
    
    return result;
}
#endif