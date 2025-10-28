// include/win_compat_undef_access.h
#pragma once
#ifdef _WIN32
  // 某些头会把 access 定义成 _access，干扰 Boost Serialization 的 access 类
  #ifdef access
  #undef access
  #endif
#endif
