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
#ifndef PONDER_ID_TRAITS_HPP
#define PONDER_ID_TRAITS_HPP

#ifdef PONDER_ID_TRAITS_STD_STRING

#include <string>

namespace ponder {

    using std::size_t;

namespace detail {

struct IdTraits
{
    using string_t = std::string;
    using id_value_t = string_t;
    using id_ref_t = const id_value_t&;
    using id_return_t = const id_value_t&;

    static const char* cstr(id_ref_t r) {return r.c_str();}
};

} // namespace detail
} // namespace ponder

#elif defined(PONDER_ID_TRAITS_STRING_VIEW)

#include <string>
#include <string_view>

namespace ponder {

    using std::size_t;
    using std::string_view;

namespace detail {

struct IdTraits
{
    using string_t = std::string;
    using id_value_t = string_t;
    using id_ref_t = string_view;
    using id_return_t = const id_value_t&;

    static const char* cstr(id_ref_t r) noexcept {return r.data();}
};

} // namespace detail
} // namespace ponder

#endif // PONDER_ID_TRAITS_STRING_VIEW

namespace ponder {

/// Ponder string class. Needs to have traits of std::string.
using String = detail::IdTraits::string_t;

/// Type used to hold identifier values.
using Id = detail::IdTraits::id_value_t;

/// Type used to pass around references to the an identifier type.
using IdRef = detail::IdTraits::id_ref_t;

/// Type used to return a identifier value.
using IdReturn = detail::IdTraits::id_return_t;

namespace id {

[[nodiscard]] inline const char* c_str(IdRef r) noexcept {return detail::IdTraits::cstr(r);}

}

} // namespace ponder

#endif // PONDER_ID_TRAITS_HPP

