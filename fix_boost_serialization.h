// fix_boost_serialization.h - 正确的版本
#pragma once

#include <boost/serialization/access.hpp>

// 这个宏应该在类定义内部使用
#ifndef BOOST_SERIALIZATION_ACCESS
#define BOOST_SERIALIZATION_ACCESS() \
    friend class boost::serialization::access;
#endif