//=== boost_compat.h : 屏蔽 Windows 对 POSIX 名称的宏污染（尤其是 access） ===
#pragma once

//--- 1) 在任何系统头之前声明：不要导出 POSIX 非标准别名 ----------------------
#ifndef _CRT_DECLARE_NONSTDC_NAMES
#define _CRT_DECLARE_NONSTDC_NAMES 0
#endif

//--- 2) 主动预包含 <io.h>，让它在“禁用别名”的前提下完成一次性声明 ----------
#include <io.h>

//--- 3) 清理可能已存在的宏污染 ------------------------------------------------
#ifdef access
#  undef access
#endif
#ifdef min
#  undef min
#endif
#ifdef max
#  undef max
#endif

// 注意：不要在这里包含任何 Boost 头！
//      DBoW2 / ORB-SLAM3 自己会按需包含 boost/serialization/*。
//      我们只负责把 "access" 宏移走，避免被替换成 "_access"。
