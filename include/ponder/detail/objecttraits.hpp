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
#ifndef PONDER_DETAIL_OBJECTTRAITS_HPP
#define PONDER_DETAIL_OBJECTTRAITS_HPP

#include "rawtype.hpp"
#include <ponder/type.hpp>

namespace ponder {
namespace detail {

/*
 * - ReferenceType: the reference type closest to T which allows to have direct access
 *   to the object (T& for raw types and references, T* for pointer types)
 *   Note: not T& reference type!
 * - PointerType: the pointer type closest to T which allows to have direct access to
 *   the object (T*)
 * - DataType: the actual raw type of the object (removes all indirections, as well const
 *   and reference modifiers)
 *
 * - isWritable: true if the type allows to modify the object (non-const references and pointers)
 * - isRef: true if the type is a reference type (references and pointers)
 *
 * - get(void*): get a direct access to an object given by a typeless pointer (in other
 *   words, converts from void* to ReferenceType)
 * - getPointer(T): get a direct pointer to an object, regardless its original
 *   storage / modifiers (in other words, convert from T to PointerType)
 */

template <typename T, typename E = void>
struct TypeTraits
{
    static constexpr ReferenceKind kind = ReferenceKind::Instance;
    using Type = T;
    using ReferenceType = T&;
    using PointerType = T*;
    using DereferencedType = T;
    static_assert(!std::is_void_v<T>, "Incorrect type details");
    using DataType = typename DataType<T>::Type;
    static constexpr bool isWritable = !std::is_const_v<DereferencedType>;
    static constexpr bool isRef = false;

    static ReferenceType get(void* pointer) noexcept { return *static_cast<T*>(pointer); }
    static PointerType getPointer(T& value) noexcept { return &value; }
    static PointerType getPointer(T* value) noexcept { return value; }
};

// void
template <>
struct TypeTraits<void>
{
    static constexpr ReferenceKind kind = ReferenceKind::None;
    using T = void;
    using ReferenceType = T*;
    using PointerType = T*;
    using DereferencedType = T;
    using DataType = DataType<T>::Type;
    static constexpr bool isWritable = false;
    static constexpr bool isRef = false;

    static ReferenceType get(void* /*pointer*/) noexcept { return nullptr; }
    static PointerType getPointer(T* value) noexcept { return value; }
};

// Raw pointers
template <typename T>
struct TypeTraits<T*>
{
    static constexpr ReferenceKind kind = ReferenceKind::Pointer;
    using Type = T*;
    using ReferenceType = T*;
    using PointerType = T*;
    using DereferencedType = T;
    using DataType = typename DataType<T>::Type;
    static constexpr bool isWritable = !std::is_const_v<DereferencedType>;
    static constexpr bool isRef = true;

    static ReferenceType get(void* pointer) noexcept {return static_cast<T*>(pointer);}
    static PointerType getPointer(T& value) noexcept {return &value;}
    static PointerType getPointer(T* value) noexcept {return value;}
};

// References
template <typename T>
struct TypeTraits<T&>
{
    static constexpr ReferenceKind kind = ReferenceKind::Reference;
    using Type = T&;
    using ReferenceType = T&;
    using PointerType = T*;
    using DereferencedType = T;
    using DataType = typename DataType<T>::Type;
    static constexpr bool isWritable = !std::is_const_v<DereferencedType>;
    static constexpr bool isRef = true;

    static ReferenceType get(void* pointer) noexcept {return *static_cast<T*>(pointer);}
    static PointerType getPointer(T& value) noexcept {return &value;}
    static PointerType getPointer(T* value) noexcept {return value;}
};

// Base class for smart pointers
template <class P, typename T>
struct SmartPointerReferenceTraits
{
    static constexpr ReferenceKind kind = ReferenceKind::SmartPointer;
    using Type = P;
    using ReferenceType = T&;
    using PointerType = P;
    using DereferencedType = T;
    using DataType = typename DataType<T>::Type;
    static constexpr bool isWritable = !std::is_const_v<DereferencedType>;
    static constexpr bool isRef = true;

    static ReferenceType get(void* pointer) noexcept {return *static_cast<P*>(pointer);}
    static PointerType getPointer(P& value) {return get_pointer(value);}
};

// std::shared_ptr<>
template <typename T>
struct TypeTraits<std::shared_ptr<T>>
    : SmartPointerReferenceTraits<std::shared_ptr<T>,T> {};


// Built-in arrays []
template <typename T, size_t N>
struct TypeTraits<T[N], std::enable_if_t<std::is_array_v<T>>>
{
    static constexpr ReferenceKind kind = ReferenceKind::BuiltinArray;
    using Type = T[N];
    using DataType = typename DataType<T>::Type;
    using ReferenceType = T(&)[N];
    using PointerType = T*;
    using DereferencedType = T[N];
    static constexpr size_t Size = N;
    static constexpr bool isWritable = !std::is_const_v<T>;
    static constexpr bool isRef = false;
};


//template <typename C, typename T, size_t S>
//struct MemberTraits<std::array<T,S>(C::*)>
//{
//    using Type = std::array<T,S>(C::*);
//    using ClassType = C;
//    using ExposedType = std::array<T,S>;
//    using DataType = typename DataType<T>::Type;
//    //static constexpr bool isWritable = !std::is_const<DataType>::value;
//
//    class Access
//    {
//    public:
//        Access(Type d) : data(d) {}
//        ExposedType& getter(ClassType& c) const { return c.*data; }
//    private:
//        Type data;
//    };
//};
//
//template <typename C, typename T>
//struct MemberTraits<std::vector<T>(C::*)>
//{
//    using Type = std::vector<T>(C::*);
//    using ClassType = C;
//    using ExposedType = std::vector<T>;
//    using DataType = typename DataType<T>::Type;
//
//    class Access
//    {
//    public:
//        Access(Type d) : data(d) {}
//        ExposedType& getter(ClassType& c) const { return c.*data; }
//    private:
//        Type data;
//    };
//};
//
//template <typename C, typename T>
//struct MemberTraits<std::list<T>(C::*)>
//{
//    using Type = std::list<T>(C::*);
//    using ClassType = C;
//    using ExposedType = std::list<T>;
//    using DataType = typename DataType<T>::Type;
//
//    class Access
//    {
//    public:
//        Access(Type d) : data(d) {}
//        ExposedType& getter(ClassType& c) const { return c.*data; }
//    private:
//        Type data;
//    };
//};

} // namespace detail
} // namespace ponder

#endif // PONDER_DETAIL_OBJECTTRAITS_HPP
