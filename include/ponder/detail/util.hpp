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
#ifndef PONDER_UTIL_HPP
#define PONDER_UTIL_HPP

#include <ponder/config.hpp>
#include <ponder/type.hpp>
#include <type_traits>
#include <memory>

namespace ponder {
namespace detail {

//------------------------------------------------------------------------------

template<bool C, typename T, typename F>
struct if_c
{
    using type = T;
};

template<typename T, typename F>
struct if_c<false,T,F>
{
    using type = F;
};

//------------------------------------------------------------------------------

class bad_conversion final : public std::exception {};

template <typename T, typename F, typename O = void>
struct convert_impl
{
    T operator () (const F& from)
    {
        const T result = static_cast<T>(from);
        return result;
    }
};

template <typename F>
Id to_str(F from)
{
    return std::to_string(from);
}

template <typename F>
struct convert_impl <Id, F>
{
    Id operator () (const F& from)
    {
        return detail::to_str(from);
    }
};

template <>
struct convert_impl <Id, bool>
{
    Id operator () (const bool& from) const
    {
        static const Id t("1"), f("0");
        return from ? t : f;
    }
};

PONDER_API bool conv(const String& from, bool& to);
PONDER_API bool conv(const String& from, char& to);
PONDER_API bool conv(const String& from, unsigned char& to);
PONDER_API bool conv(const String& from, short& to);
PONDER_API bool conv(const String& from, unsigned short& to);
PONDER_API bool conv(const String& from, int& to);
PONDER_API bool conv(const String& from, unsigned int& to);
PONDER_API bool conv(const String& from, long& to);
PONDER_API bool conv(const String& from, unsigned long& to);
PONDER_API bool conv(const String& from, long long& to);
PONDER_API bool conv(const String& from, unsigned long long& to);
PONDER_API bool conv(const String& from, float& to);
PONDER_API bool conv(const String& from, double& to);

template <typename T>
struct convert_impl <T, Id,
                     std::enable_if_t< (std::is_integral_v<T> || std::is_floating_point_v<T>)
                             && !std::is_const_v<T>
                             && !std::is_reference_v<T> > >
{
    T operator () (const String& from)
    {
        T result;
        if (!conv(from, result))
            throw bad_conversion();
        return result;
    }
};

template <typename T, typename F>
T convert(const F& from)
{
    return convert_impl<T,F>()(from);
}

//------------------------------------------------------------------------------
// Return true if all args true.
template <typename... As>
static bool allTrue(As... a)
    {return (true && ... && a); }

// Get value type enum as string description.
PONDER_API const char* valueKindAsString(ValueKind t);

} // detail
} // Ponder

#endif // PONDER_UTIL_HPP
