//=== boost_compat.h : 屏蔽 Windows 对 POSIX 名称的宏污染（尤其是 access） ===
#pragma once

#ifndef _CRT_DECLARE_NONSTDC_NAMES
#define _CRT_DECLARE_NONSTDC_NAMES 0
#endif

// 主动在“禁用别名”的前提下包含一次 <io.h>，并清理残留宏
#include <io.h>
#ifdef access
#  undef access
#endif
#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif

// 不要在这里包含任何 Boost 头！
