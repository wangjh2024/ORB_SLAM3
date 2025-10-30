// gl_wrapper.h - 解决Windows下OpenGL头文件冲突问题
#ifndef GL_WRAPPER_H
#define GL_WRAPPER_H

// 首先包含Windows头文件并禁用冲突宏
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// 取消Windows.h可能定义的冲突宏
#undef near
#undef far
#undef DELETE
#undef ERROR
#undef IGNORE

// 然后包含OpenGL头文件
#include <GL/gl.h>
#include <GL/glu.h>

#endif // GL_WRAPPER_H