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
#ifndef PONDER_DETAIL_VALUEIMPL_HPP
#define PONDER_DETAIL_VALUEIMPL_HPP

#include <ponder/type.hpp>
#include <ponder/valuemapper.hpp>

namespace ponder {
namespace detail {

/**
 * \brief Value visitor which converts the stored value to a type T
 */
template <typename T>
struct ConvertVisitor
{
    using result_type = T;

    template <typename U>
    T operator()(const U& value) const
    {
        // Dispatch to the proper ValueConverter
        return ponder_ext::ValueMapper<T>::from(value);
    }

    // Optimization when source type is the same as requested type
    T operator()(const T& value) const
    {
        return value;
    }
    T operator()(T&& value) const
    {
        return std::move(value);
    }

    T operator()(NoType) const
    {
        // Error: trying to convert an empty value
        PONDER_ERROR(BadType(ValueKind::None, mapType<T>()));
    }
};

/**
 * \brief Value visitor which verifies if the stored value can convert to a type T
 */
template <typename T>
struct CanConvertVisitor
{
    using result_type = bool;

    template <typename U>
    bool operator()(const U& value) const
    {
        // Dispatch to the proper ValueConverter
        return ponder_ext::ValueMapper<T>::can_from(value);
    }

    // Optimization when source type is the same as requested type
    bool operator()(const T& value) const
    {
        return true;
    }

    bool operator()(NoType) const
    {
        return false;
    }
};

/**
 * \brief Binary value visitor which compares two values using operator <
 */
struct LessThanVisitor
{
    using result_type = bool;

    template <typename T, typename U>
    bool operator()(const T&, const U&) const
    {
        // Different types : compare types identifiers
        return mapType<T>() < mapType<U>();
    }

    template <typename T>
    bool operator()(const T& v1, const T& v2) const
    {
        // Same types : compare values
        return v1 < v2;
    }

    bool operator()(NoType, NoType) const
    {
        // No type (empty values) : they're considered equal
        return false;
    }
};

/**
 * \brief Binary value visitor which compares two values using operator ==
 */
struct EqualVisitor
{
    using result_type = bool;

    template <typename T, typename U>
    bool operator()(const T&, const U&) const
    {
        // Different types : not equal
        return false;
    }

    template <typename T>
    bool operator()(const T& v1, const T& v2) const
    {
        // Same types : compare values
        return v1 == v2;
    }

    bool operator()(NoType, NoType) const
    {
        // No type (empty values) : they're considered equal
        return true;
    }
};

} // namespace detail
} // namespace ponder

#endif // PONDER_DETAIL_VALUEIMPL_HPP
