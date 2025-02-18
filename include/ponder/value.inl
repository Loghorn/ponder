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

#include <ponder/detail/valueimpl.hpp>

namespace ponder {
namespace detail {


// Is T a user type.
template <typename T> struct IsUserType {
    using DataType = typename DataType<T>::Type;
    static constexpr bool value = std::is_class_v<DataType>
        && !std::is_same_v<DataType, Value>
        && !std::is_same_v<DataType, UserObject>
        && !std::is_same_v<DataType, ValueRef>
        && !std::is_same_v<DataType, String>;
};

// Decide whether the UserObject holder should be ref (true) or copy (false).
template <typename T> struct IsUserObjRef {
    static constexpr bool value = std::is_pointer_v<T> || std::is_reference_v<T>;
};


// Convert ponder::Value to type
template <typename T, typename E = void>
struct ValueTo
{
    static T convert(const Value& value) {return value.visit(ConvertVisitor<T>());}
    static bool can_convert(const Value& value) {return value.visit(CanConvertVisitor<T>());}
};

// Don't need to convert, we're returning a Value
template <>
struct ValueTo<Value>
{
    static Value convert(const Value& value) {return value;}
    static Value convert(Value&& value) {return std::move(value);}
    static bool can_convert(const Value&) {return true;}
};

// Convert Values to pointers for basic types
template <typename T>
struct ValueTo<T*, std::enable_if_t<!hasStaticTypeDecl<T>()>>
{
    static T* convert(const Value& value)
    {
        return value.to<ValueRef>().getRef<T>();
    }
    static bool can_convert(const Value&) {return true;}
};

// Convert Values to references for basic types
//template <typename T>
//struct ValueTo<T&, typename std::enable_if<!hasStaticTypeDecl<T>()>::type>
//{
//    static T convert(const Value& value)
//    {
//        return *static_cast<T*>(value.to<UserObject>().pointer());
//    }
//};

} // namespace detail

template <typename T>
Value::Value(const T& val)
    : m_value(ponder_ext::ValueMapper<T>::to(val))
    , m_type(ponder_ext::ValueMapper<T>::kind) // mapType<T> NOT used so get same kind as to()
{
}

template <typename T>
T Value::to() const
{
    try
    {
        return detail::ValueTo<T>::convert(*this);
    }
    catch (detail::bad_conversion&)
    {
        PONDER_ERROR(BadType(kind(), mapType<T>()));
    }
}

template <typename T>
T& Value::ref()
{
    try
    {
        return std::get<T>(m_value);
    }
    catch (detail::bad_conversion&)
    {
        PONDER_ERROR(BadType(kind(), mapType<T>()));
    }
}

template <typename T>
const T& Value::cref() const
{
    try
    {
        return std::get<T>(m_value);
    }
    catch (detail::bad_conversion&)
    {
        PONDER_ERROR(BadType(kind(), mapType<T>()));
    }
}

template <typename T>
bool Value::isCompatible() const noexcept
{
    try
    {
        return detail::ValueTo<T>::can_convert(*this);
    }
    catch (std::exception&)
    {
        return false;
    }
}

template <typename T>
typename T::result_type Value::visit(T visitor) const
{
    return std::visit(visitor, m_value);
}

template <typename T>
typename T::result_type Value::visit(T visitor, const Value& other) const
{
    return std::visit(visitor, m_value, other.m_value);
}

} // namespace ponder
