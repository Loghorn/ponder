#pragma once
#ifndef PONDER_STRING_VIEW_HPP
#define PONDER_STRING_VIEW_HPP

// If we have <string_view> then wrap it, otherwise supply implementation.
//  - Clang 7 libc++ and GCC 5.4 do not have <string_view>.
#if defined(__cpp_lib_string_view)

#include <string_view>

namespace ponder {
namespace detail {
    using string_view = std::string_view;
}}

#else

static_assert(false, "Please use a compiler that supports string_view")

#endif // use string_view

#endif // PONDER_STRING_VIEW_HPP
