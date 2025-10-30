// g2o - General Graph Optimization
// Copyright (C) 2011 R. Kuemmerle, G. Grisetti, W. Burgard
// All rights reserved.

#ifndef G2O_TIMEUTIL_H
#define G2O_TIMEUTIL_H

#ifdef _WIN32
    #include <windows.h>
    #include <time.h>
    #include <chrono>
    #include <cstdint>
    
    // 防止 winsock 冲突
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
#else
    #include <sys/time.h>
    #include <unistd.h>
#endif

#include <string>
#include <iostream>

namespace g2o {

#ifdef _WIN32
// Windows 平台下的结构定义
struct timeval {
    long tv_sec;
    long tv_usec;
};

struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

// Windows 平台的 gettimeofday 实现
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

inline int gettimeofday(struct timeval* tv, struct timezone* tz) {
    FILETIME ft;
    unsigned __int64 tmpres = 0;
    static int tzflag = 0;

    if (tv != NULL) {
        GetSystemTimeAsFileTime(&ft);

        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;

        tmpres /= 10;  // 转换为微秒
        tmpres -= DELTA_EPOCH_IN_MICROSECS; 
        tv->tv_sec = static_cast<long>(tmpres / 1000000UL);
        tv->tv_usec = static_cast<long>(tmpres % 1000000UL);
    }

    if (tz != NULL) {
        if (!tzflag) {
            _tzset();
            tzflag++;
        }
        long timezone_val;
        int daylight_val;
        _get_timezone(&timezone_val);
        _get_daylight(&daylight_val);
        tz->tz_minuteswest = static_cast<int>(timezone_val / 60);
        tz->tz_dsttime = daylight_val;
    }

    return 0;
}
#endif

/**
 * return the current time in seconds since 1. Jan 1970
 */
inline double get_time() {
#ifdef _WIN32
    // 使用 chrono 替代方案，避免 winsock 冲突
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration - seconds);
    return static_cast<double>(seconds.count()) + static_cast<double>(micros.count()) * 1e-6;
#else
    struct timeval tv;
    ::gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
#endif
}

/**
 * return a monotonic increasing time
 */
inline double get_monotonic_time() {
#if (defined(_POSIX_TIMERS) && (_POSIX_TIMERS+0 >= 0) && defined(_POSIX_MONOTONIC_CLOCK))
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
#elif defined(_WIN32)
    static LARGE_INTEGER frequency;
    static BOOL frequency_initialized = QueryPerformanceFrequency(&frequency);
    
    if (frequency_initialized) {
        LARGE_INTEGER counter;
        QueryPerformanceCounter(&counter);
        return static_cast<double>(counter.QuadPart) / static_cast<double>(frequency.QuadPart);
    } else {
        return get_time();
    }
#else
    return get_time();
#endif
}

/**
 * \brief Class to measure the time spent in a scope
 */
class ScopeTime {
public: 
    ScopeTime(const char* title) : _title(title), _startTime(get_monotonic_time()) {}
    
    ~ScopeTime() {
        std::cerr << _title << " took " << 1000 * (get_monotonic_time() - _startTime) << "ms.\n";
    }
    
private:
    std::string _title;
    double _startTime;
};

} // end namespace

// 宏定义
#ifndef DO_EVERY_TS
#define DO_EVERY_TS(secs, currentTime, code) \
if (1) {\
  static double s_lastDone_ = (currentTime); \
  double s_now_ = (currentTime); \
  if (s_lastDone_ > s_now_) \
    s_lastDone_ = s_now_; \
  if (s_now_ - s_lastDone_ > (secs)) { \
    code; \
    s_lastDone_ = s_now_; \
  }\
} else \
  (void)0
#endif

#ifndef DO_EVERY
#define DO_EVERY(secs, code) DO_EVERY_TS(secs, g2o::get_time(), code)
#endif

#ifndef MEASURE_TIME
#define MEASURE_TIME(text, code) \
  if(1) { \
    double _start_time_ = g2o::get_time(); \
    code; \
    fprintf(stderr, "%s took %f sec\n", text, g2o::get_time() - _start_time_); \
  } else \
    (void) 0
#endif

#ifndef MEASURE_FUNCTION_TIME
#ifdef _WIN32
#define MEASURE_FUNCTION_TIME g2o::ScopeTime scopeTime(__FUNCTION__)
#else
#define MEASURE_FUNCTION_TIME g2o::ScopeTime scopeTime(__PRETTY_FUNCTION__)
#endif
#endif

#endif