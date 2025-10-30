// boost_compat.h
#ifndef BOOST_COMPAT_H
#define BOOST_COMPAT_H

// Boost序列化兼容性修复 - 针对Boost 1.83.0
#define BOOST_ALLOW_DEPRECATED_HEADERS

// 显式声明access类
namespace boost {
namespace serialization {
    class access;
}
}

// 包含必要的Boost序列化头文件
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/singleton.hpp>
#include <boost/serialization/access.hpp>

#endif // BOOST_COMPAT_H