//=== boost_compat.h : 强力清理 Windows/POSIX 宏污染（特别是 access） ===
#pragma once

// 禁止 MSVC 头把 POSIX 别名宏（如 access）导出
#ifndef _CRT_DECLARE_NONSTDC_NAMES
#define _CRT_DECLARE_NONSTDC_NAMES 0
#endif

// 先把**所有可能定义 access 宏**的头抢先包含一次
#if defined(_MSC_VER)
  #if __has_include(<corecrt_io.h>)
  #  include <corecrt_io.h>
  #endif
  #if __has_include(<io.h>)
  #  include <io.h>
  #endif
  #if __has_include(<direct.h>)
  #  include <direct.h>
  #endif
#endif

#if __has_include(<unistd.h>)
  // 一些第三方会自带兼容版 unistd.h，里头也会 #define access _access
  #include <unistd.h>
#endif

// 然后立刻把可能的宏统统关掉（只留我们需要的符号）
#ifdef access
#  undef access
#endif
#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif

// 别在这里包含任何 Boost 头！
