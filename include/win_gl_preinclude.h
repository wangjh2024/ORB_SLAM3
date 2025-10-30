// win_gl_preinclude.h - Fix Windows OpenGL header conflicts
#ifndef WIN_GL_PREINCLUDE_H
#define WIN_GL_PREINCLUDE_H

// Prevent Windows API conflicts with OpenGL
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

// Boost compatibility definitions
#define BOOST_ALL_NO_LIB
#define BOOST_SERIALIZATION_DYN_LINK

#endif // WIN_GL_PREINCLUDE_H