// include/win_gl_preinclude.h
#pragma once

// 1) 先做 Windows 轻量化，避免 min/max 等污染
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

// 2) 某些库会把这些宏“定义坏了”（比如定义成 int），先清理
#ifdef WINGDIAPI
#  undef WINGDIAPI
#endif
#ifdef APIENTRY
#  undef APIENTRY
#endif

// 3) 正确包含 Windows 头（会为 WINGDIAPI / APIENTRY 设定合适取值）
#ifdef _WIN32
#  include <windows.h>
#  undef near   // 清理常见干扰宏
#  undef far
#endif

// 4) 如果你自己的代码里直接包含 GL/GLEW，确保顺序：glew 在 gl 之前
#if __has_include(<GL/glew.h>)
#  include <GL/glew.h>
#endif
#if __has_include(<GL/gl.h>)
#  include <GL/gl.h>
#endif
