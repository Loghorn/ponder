/****************************************************************************
**
** This file is part of the Ponder library, formerly CAMP.
**
** The MIT License (MIT)
**
** Copyright (C) 2009-2014 TEGESO/TEGESOFT and/or its subsidiary(-ies) and mother company.
** Copyright (C) 2015-2020 Nick Trout.
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
**
****************************************************************************/

#pragma once
#ifndef PONDER_CONFIG_HPP
#define PONDER_CONFIG_HPP

// Get <version> file, if implemented (P0754R2).
#if (defined(__APPLE__) && __clang_major__>=10) ||\
    (!defined(__APPLE__) && (\
        (defined(__clang__) && __clang_major__>=8) ||\
        (defined(__GNUC__) && __GNUC___>=9) )) ||\
    (defined(_MSC_VER) && _MSC_VER >= 1922)
#   include <version> // C++ version & features
#endif

// Check we have C++17 support. Report issues to Github project.
#if defined(_MSC_VER)
    // Earlier MSVC compilers lack features or have C++17 bugs.
    static_assert(_MSC_VER >= 1911, "MSVC 2017 required");
    // We disable some annoying warnings of VC++
#   pragma warning(disable: 4275) // non dll-interface class 'X' used as base for dll-interface class 'Y'
#   pragma warning(disable: 4251) // class 'X' needs to have dll-interface to be used by clients of class 'Y'
#   include <ostream> // In future MSVC, <string> doesn't transitively <ostream>, ponder will
                      // compile failed with error C2027 and C2065, so add <ostream>.
#endif

// We define the PONDER_API macro according to the current operating system and build mode
#if defined(_WIN32) || defined(__WIN32__)
#   ifndef PONDER_STATIC
#       ifdef PONDER_EXPORTS
#           define PONDER_API __declspec(dllexport)
#       else
#           define PONDER_API __declspec(dllimport)
#       endif
#   else
#       define PONDER_API
#   endif
#else
#   define PONDER_API
#endif

// Debug build config?
#define PONDER_DEBUG (!defined(NDEBUG))

#ifndef PONDER_USING_LUA
#   define PONDER_USING_LUA 0
#endif

// If user doesn't define traits use the default:
#ifndef PONDER_ID_TRAITS_USER
//# define PONDER_ID_TRAITS_STD_STRING      // Use std::string and const std::string&
#   define PONDER_ID_TRAITS_STRING_VIEW     // Use std::string and std::string_view
#endif // PONDER_ID_TRAITS_USER

#include "detail/idtraits.hpp"
#include <cassert>

#define PONDER_NON_COPYABLE(CLS) \
    CLS(CLS const&) = delete; \
    CLS& operator=(CLS const&) = delete

#define PONDER_UNUSED(VAR) ((void)&(VAR))

#endif // PONDER_CONFIG_HPP

