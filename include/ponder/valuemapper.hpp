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
#ifndef PONDER_VALUEMAPPER_HPP
#define PONDER_VALUEMAPPER_HPP

#include <ponder/config.hpp>
#include <ponder/enum.hpp>
#include <ponder/enumobject.hpp>
#include <ponder/userobject.hpp>
#include <ponder/arraymapper.hpp>
#include <ponder/errors.hpp>
#include <ponder/detail/util.hpp>
#include <ponder/detail/valueref.hpp>

/**
 * \namespace ponder_ext
 * \brief Ponder user extendable namespace.
 *
 * C++ only allows specialisation within the same namespace as the type you
 * are extending. To avoid clashes with the ponder namespace we use `ponder_ext`
 * to provide a safe place to place custom specialisations.
 * \see ValueMapper.
 */

namespace ponder_ext
{
    template <typename T, typename C = void> struct ValueMapper;
}

namespace ponder {

/**
 * \brief Map a C++ type to a Ponder type
 *
 * This function simply returns the mapping defined by ValueMapper (i.e. \c ValueMapper<T>::type).
 *
 * \return Ponder type which T maps to
 */
template <typename T>
ValueKind mapType()
{
    return ponder_ext::ValueMapper<typename detail::DataType<T>::Type>::kind;
}

} // namespace ponder


namespace ponder_ext {

/**
 * \class ValueMapper
 *
 * \brief Template providing a mapping between C++ types/values and Ponder types/values
 *
 * ValueMapper<T> defines a mapping to and from type T to a Value. It defines three things in
 * order to make T fully compliant with the system:
 *
 * \li The abstract Ponder type that T is mapped to
 * \li A function to convert from T *to* the mapped Ponder type
 * \li A function to convert *from* all supported Ponder types to T
 *
 * Pseudo-code:
 * \code
 * template <> struct ValueMapper<TypeSpecialised>
 * {
 *     static PonderValueKind to(ValueKind value)      { return convertToPonderType(value); }
 *     static TypeSpecialised from(PonderValueKind pv) { return convertPonderToType(pc); }
 * };
 * \endcode
 *
 * ValueMapper is specialized for every supported type, and can be specialized
 * for any of your own types in order to extend the system.
 *
 * Here is an example of mapping for a custom string class:
 *
 * \code
 * namespace ponder_ext
 * {
 *     template <>
 *     struct ValueMapper<MyStringClass>
 *     {
 *         // The corresponding Ponder type is "string"
 *         static constexpr ponder::ValueKind kind = ponder::ValueKind::String;
 *
 *         // Convert from MyStringClass to ponder::String
 *         static ponder::String to(const MyStringClass& source)
 *         {
 *             return source.to_std_string();
 *         }
 *
 *         // Convert from any type to MyStringClass
 *         // Be smart, just reuse ValueMapper<ponder::String> :)
 *         template <typename T>
 *         static MyStringClass from(const T& source)
 *         {
 *             return MyStringClass(ValueMapper<ponder::String>::from(source));
 *         }
 *     };
 * }
 * \endcode
 *
 * Generic version of ValueMapper -- T doesn't match with any specialization
 * and is thus treated as a user object
 */

/** \cond NoDocumentation */

template <typename T, typename C>
struct ValueMapper
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::User;

    static ponder::UserObject to(const T& source) {return ponder::UserObject(source);}

    static T from(bool)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Boolean,    ponder::mapType<T>()));}
    static bool can_from(bool) {return false;}
    static T from(long)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Integer,    ponder::mapType<T>()));}
    static bool can_from(long) {return false;}
    static T from(long long)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::LongInteger,ponder::mapType<T>()));}
    static bool can_from(long long) {return false;}
    static T from(double)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Real,       ponder::mapType<T>()));}
    static bool can_from(double) {return false;}
    static T from(const ponder::String&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::String,     ponder::mapType<T>()));}
    static bool can_from(const ponder::String&) {return false;}
    static T from(const ponder::EnumObject&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Enum,       ponder::mapType<T>()));}
    static bool can_from(const ponder::EnumObject&) {return false;}
    static T from(const ponder::detail::ValueRef&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Reference,  ponder::mapType<T>()));}
    static bool can_from(const ponder::detail::ValueRef&) {return false;}
    static T from(const ponder::UserObject& source)
        {return source.get<T>();}
    static bool can_from(const ponder::UserObject& source) try {source.get<T>(); return true;} catch (...) {return false;}
};

/**
 * Specialization of ValueMapper for abstract types
 */
template <typename T>
struct ValueMapper<T, std::enable_if_t<std::is_abstract_v<T>>>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::User;

    static ponder::UserObject to(const T& source) {return ponder::UserObject(source);}
};

/**
 * Specialization of ValueMapper for pointers to basic types
 *  - Used for pass by-reference parameters that are non-registered types.
 */
template <typename T>
struct ValueMapper<T*, std::enable_if_t<!ponder::detail::hasStaticTypeDecl<T>()>>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::Reference;

    static ponder::detail::ValueRef to(T* source) {return ponder::detail::ValueRef::make(source);}

    static T* from(const ponder::detail::ValueRef& source) {return source.getRef<T>();}
    static bool can_from(const ponder::detail::ValueRef&) {return true;}

    static T from(bool)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Boolean,    ponder::mapType<T>()));}
    static bool can_from(bool) {return false;}
    static T from(long)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Integer,    ponder::mapType<T>()));}
    static bool can_from(long) {return false;}
    static T from(long long)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::LongInteger,ponder::mapType<T>()));}
    static bool can_from(long long) {return false;}
    static T from(double)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Real,       ponder::mapType<T>()));}
    static bool can_from(double) {return false;}
    static T from(const ponder::String&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::String,     ponder::mapType<T>()));}
    static bool can_from(const ponder::String&) {return false;}
    static T from(const ponder::EnumObject&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Enum,       ponder::mapType<T>()));}
    static bool can_from(const ponder::EnumObject&) {return false;}
    static T from(const ponder::UserObject&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::User,       ponder::mapType<T>()));}
    static bool can_from(const ponder::UserObject&) {return false;}
};

///**
// * Specialization of ValueMapper for pointers to basic types
// *  - Used for pass by-reference parameters that are non-registered types.
// */
//template <typename T>
//struct ValueMapper<T&, typename std::enable_if<!ponder::detail::hasStaticTypeDecl<T>()>::type>
//{
//    static constexpr ponder::ValueKind kind = ponder::ValueKind::User;
//
//    static ponder::UserObject to(T& source) {return ponder::UserObject::makeRef(source);}
//
//    static T from(const ponder::UserObject& source) {return *static_cast<T*>(source.pointer());}
//
//    static T from(bool)
//    {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Boolean,    ponder::mapType<T>()));}
//    static T from(long)
//    {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Integer,    ponder::mapType<T>()));}
//    static T from(long long)
//    {PONDER_ERROR(ponder::BadType(ponder::ValueKind::LongInteger,ponder::mapType<T>()));}
//    static T from(double)
//    {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Real,       ponder::mapType<T>()));}
//    static T from(const ponder::String&)
//    {PONDER_ERROR(ponder::BadType(ponder::ValueKind::String,     ponder::mapType<T>()));}
//    static T from(const ponder::EnumObject&)
//    {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Enum,       ponder::mapType<T>()));}
//};

/**
 * Specialization of ValueMapper for booleans
 */
template <>
struct ValueMapper<bool>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::Boolean;

    static bool to(bool source) {return source;}

    static bool from(bool source)                  {return source;}
    static bool can_from(bool)                     {return true;}
    static bool from(long source)                  {return source != 0;}
    static bool can_from(long)                     {return true;}
    static bool from(long long source)             {return source != 0;}
    static bool can_from(long long)                {return true;}
    static bool from(double source)                {return source != 0.;}
    static bool can_from(double)                   {return true;}
    static bool from(const ponder::String& source) {return ponder::detail::convert<bool>(source);}
    static bool can_from(const ponder::String& source) {bool dummy; return ponder::detail::conv(source, dummy);}
    static bool from(const ponder::EnumObject& source) {return source.value() != 0;}
    static bool can_from(const ponder::EnumObject&)    {return true;}
    static bool from(const ponder::UserObject& source) {return source.pointer() != nullptr;}
    static bool can_from(const ponder::UserObject&)    {return true;}
    static bool from(const ponder::detail::ValueRef& source) {return source.getRef<bool>();}
    static bool can_from(const ponder::detail::ValueRef&)    {return true;}
};

/**
 * Specialization of ValueMapper for integers
 */
template <typename T>
struct ValueMapper<T,
                   std::enable_if_t<
                 std::is_integral_v<T>
                 && !std::is_const_v<T>
    > >
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::Integer;
    static long to(T source) {return static_cast<long>(source);}

    static T from(bool source)                    {return static_cast<T>(source);}
    static bool can_from(bool)                    {return true;}
    static T from(long source)                    {return static_cast<T>(source);}
    static bool can_from(long)                    {return true;}
    static T from(long long source)               {return static_cast<T>(source);}
    static bool can_from(long long)               {return true;}
    static T from(double source)                  {return static_cast<T>(source);}
    static bool can_from(double)                  {return true;}
    static T from(const ponder::String& source)   {return ponder::detail::convert<T>(source);}
    static bool can_from(const ponder::String& source)   {long dummy; return ponder::detail::conv(source, dummy);}
    static T from(const ponder::EnumObject& source) {return static_cast<T>(source.value());}
    static bool can_from(const ponder::EnumObject&) {return true;}
    static T from(const ponder::UserObject&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::User, ponder::ValueKind::Integer));}
    static bool can_from(const ponder::UserObject&)    {return false;}
    static T from(const ponder::detail::ValueRef& source) {return *source.getRef<T>();}
    static bool can_from(const ponder::detail::ValueRef&) {return true;}
};

/**
 * Specializations of ValueMapper for long integers
 */
template <>
struct ValueMapper<long long>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::LongInteger;
    static long long to(long long source) {return source;}

    static long long from(bool source)                    {return static_cast<long long>(source);}
    static bool can_from(bool)                            {return true;}
    static long long from(long source)                    {return static_cast<long long>(source);}
    static bool can_from(long)                            {return true;}
    static long long from(long long source)               {return static_cast<long long>(source);}
    static bool can_from(long long)                       {return true;}
    static long long from(double source)                  {return static_cast<long long>(source);}
    static bool can_from(double)                          {return true;}
    static long long from(const ponder::String& source)   {return ponder::detail::convert<long long>(source);}
    static bool can_from(const ponder::String& source)    {long long dummy; return ponder::detail::conv(source, dummy);}
    static long long from(const ponder::EnumObject& source) {return static_cast<long long>(source.value());}
    static bool can_from(const ponder::EnumObject&)         {return true;}
    static long long from(const ponder::UserObject&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::User, ponder::ValueKind::LongInteger));}
    static bool can_from(const ponder::UserObject&)    {return false;}
    static long long from(const ponder::detail::ValueRef& source) {return *source.getRef<long long>();}
    static bool can_from(const ponder::detail::ValueRef&)         {return true;}
};

template <>
struct ValueMapper<unsigned long long>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::LongInteger;
    static long long to(unsigned long long source) {return source;}

    static unsigned long long from(bool source)                    {return static_cast<unsigned long long>(source);}
    static bool can_from(bool)                                     {return true;}
    static unsigned long long from(long source)                    {return static_cast<unsigned long long>(source);}
    static bool can_from(long)                                     {return true;}
    static unsigned long long from(long long source)               {return static_cast<unsigned long long>(source);}
    static bool can_from(long long)                                {return true;}
    static unsigned long long from(double source)                  {return static_cast<unsigned long long>(source);}
    static bool can_from(double)                                   {return true;}
    static unsigned long long from(const ponder::String& source)   {return ponder::detail::convert<unsigned long long>(source);}
    static bool can_from(const ponder::String& source)             {unsigned long long dummy; return ponder::detail::conv(source, dummy);}
    static unsigned long long from(const ponder::EnumObject& source) {return static_cast<unsigned long long>(source.value());}
    static bool can_from(const ponder::EnumObject&)                  {return true;}
    static unsigned long long from(const ponder::UserObject&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::User, ponder::ValueKind::LongInteger));}
    static bool can_from(const ponder::UserObject&)    {return false;}
    static unsigned long long from(const ponder::detail::ValueRef& source) {return *source.getRef<unsigned long long>();}
    static bool can_from(const ponder::detail::ValueRef&)                  {return true;}
};

/*
 * Specialization of ValueMapper for reals
 */
template <typename T>
struct ValueMapper<T,
                   std::enable_if_t<
                 std::is_floating_point_v<T>
                 && !std::is_const_v<T>
    > >
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::Real;
    static double to(T source) {return static_cast<double>(source);}

    static T from(bool source)                    {return static_cast<T>(source);}
    static bool can_from(bool)                    {return true;}
    static T from(long source)                    {return static_cast<T>(source);}
    static bool can_from(long)                    {return true;}
    static T from(long long source)               {return static_cast<T>(source);}
    static bool can_from(long long)               {return true;}
    static T from(double source)                  {return static_cast<T>(source);}
    static bool can_from(double)                  {return true;}
    static T from(const ponder::String& source)   {return ponder::detail::convert<T>(source);}
    static bool can_from(const ponder::String& source)   {T dummy; return ponder::detail::conv(source, dummy);}
    static T from(const ponder::EnumObject& source) {return static_cast<T>(source.value());}
    static bool can_from(const ponder::EnumObject&) {return true;}
    static T from(const ponder::UserObject&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::User, ponder::ValueKind::Real));}
    static bool can_from(const ponder::UserObject&)    {return false;}
    static T from(const ponder::detail::ValueRef& source) {return *source.getRef<T>();}
    static bool can_from(const ponder::detail::ValueRef&) {return true;}
};

/**
 * Specialization of ValueMapper for ponder::String
 */
template <>
struct ValueMapper<ponder::String>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::String;
    static const ponder::String& to(const ponder::String& source) {return source;}

    static ponder::String from(bool source)
        {return ponder::detail::convert<ponder::String>(source);}
    static bool can_from(bool)                     {return true;}
    static ponder::String from(long source)
        {return ponder::detail::convert<ponder::String>(source);}
    static bool can_from(long)                     {return true;}
    static ponder::String from(long long source)
        {return ponder::detail::convert<ponder::String>(source);}
    static bool can_from(long long)                {return true;}
    static ponder::String from(double source)
        {return ponder::detail::convert<ponder::String>(source);}
    static bool can_from(double)                   {return true;}
    static ponder::String from(const ponder::String& source)
        {return source;}
    static bool can_from(const ponder::String&)    {return true;}
    static ponder::String from(const ponder::EnumObject& source)
        {return source.name();}
    static bool can_from(const ponder::EnumObject&)    {return true;}
    static ponder::String from(const ponder::UserObject&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::User, ponder::ValueKind::String));}
    static bool can_from(const ponder::UserObject&)    {return false;}
    static ponder::String from(const ponder::detail::ValueRef&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Reference, ponder::ValueKind::String));}
    static bool can_from(const ponder::detail::ValueRef&)    {return false;}
};

// TODO - Add ponder::is_string() ?
template <>
struct ValueMapper<const ponder::String> : ValueMapper<ponder::String> {};

template <>
struct ValueMapper<ponder::string_view>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::String;

    static ponder::String to(const ponder::string_view& sv)
        {return {sv.data(), sv.length()};}
    template <typename T>
    static ponder::string_view from(const T& source)
        {return ValueMapper<ponder::String>::from(source);}
    template <typename T>
    static bool can_from(const T& source)
        {return ValueMapper<ponder::String>::can_from(source);}
};

/**
 * Specialization of ValueMapper for const char*.
 * Conversions to const char* are disabled (can't return a pointer to a temporary)
 */
template <>
struct ValueMapper<const char*>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::String;
    static ponder::String to(const char* source) {return source;}

    template <typename T>
    static const char* from(const T&)
    {
        // If you get this error, it means you're trying to cast
        // a ponder::Value to a const char*, which is not allowed
        static_assert(T::CONVERSION_TO_CONST_CHAR_PTR_IS_NOT_ALLOWED(), "Conversion to cont char* is not allowed");
    }

    template <typename T>
    static bool can_from(const T&)
    {
        // If you get this error, it means you're trying to cast
        // a ponder::Value to a const char*, which is not allowed
        static_assert(T::CONVERSION_TO_CONST_CHAR_PTR_IS_NOT_ALLOWED(), "Conversion to cont char* is not allowed");
    }
};

/**
 * Specialization of ValueMapper for arrays.
 * No conversion allowed, only type mapping is provided.
 *
 * Warning: special case for char[] and const char[], they are strings not arrays
 */
template <typename T>
struct ValueMapper<T,
                   std::enable_if_t<
                       ArrayMapper<T>::isArray
            && !std::is_same_v<typename ArrayMapper<T>::ElementType, char>
            && !std::is_same_v<typename ArrayMapper<T>::ElementType, const char>
        > >
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::Array;
};

/**
 * Specializations of ValueMapper for char arrays.
 * Conversion to char[N] is disabled (can't return an array).
 */
template <size_t N>
struct ValueMapper<char[N]>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::String;
    static ponder::String to(const char (&source)[N]) {return ponder::String(source);}
};
template <size_t N>
struct ValueMapper<const char[N]>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::String;
    static ponder::String to(const char (&source)[N]) {return ponder::String(source);}
};

/**
 * Specialization of ValueMapper for enum types
 */
template <typename T>
struct ValueMapper<T, std::enable_if_t<std::is_enum_v<T>>>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::Enum;
    static ponder::EnumObject to(T source) {return ponder::EnumObject(source);}

    static T from(bool source)      {return static_cast<T>(static_cast<long>(source));}
    static bool can_from(bool)      {return true;}
    static T from(long source)      {return static_cast<T>(source);}
    static bool can_from(long)      {return true;}
    static T from(long long source) {return static_cast<T>(source);}
    static bool can_from(long long) {return true;}
    static T from(double source)    {return static_cast<T>(static_cast<long>(source));}
    static bool can_from(double)    {return true;}
    static T from(const ponder::EnumObject& source)
        {return static_cast<T>(source.value());}
    static bool can_from(const ponder::EnumObject&)    {return true;}
    static T from(const ponder::UserObject&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::User, ponder::ValueKind::Enum));}
    static bool can_from(const ponder::UserObject&)    {return false;}
    static T from(const ponder::detail::ValueRef&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Reference, ponder::ValueKind::Enum));}
    static bool can_from(const ponder::detail::ValueRef&)    {return false;}

    // The string -> enum conversion involves a little more work:
    // we try two different conversions (as a name and as a value)
    static T from(const ponder::String& source)
    {
        // Get the metaenum of T, if any
        const ponder::Enum* metaenum = ponder::enumByTypeSafe<T>();

        // First try as a name
        if (metaenum && metaenum->hasName(source))
            return static_cast<T>(metaenum->value(source));

        // Then try as a number
        long value = ponder::detail::convert<long>(source);
        if (!metaenum || metaenum->hasValue(value))
            return static_cast<T>(value);

        // Not a valid enum name or number: throw an error
        PONDER_ERROR(ponder::BadType(ponder::ValueKind::String, ponder::ValueKind::Enum));
    }
    static bool can_from(const ponder::String& source)
    {
        const ponder::Enum* metaenum = ponder::enumByTypeSafe<T>();

        if (metaenum && metaenum->hasName(source))
            return true;

        long value;
        if (!ponder::detail::conv(source, value))
            return false;
        if (!metaenum || metaenum->hasValue(value))
            return true;

        return false;
    }
};

/**
 * Specialization of ValueMapper for EnumObject
 */
template <>
struct ValueMapper<ponder::EnumObject>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::Enum;
    static const ponder::EnumObject& to(const ponder::EnumObject& source) {return source;}
    static const ponder::EnumObject& from(const ponder::EnumObject& source) {return source;}
    static bool can_from(const ponder::EnumObject&)    {return true;}

    static ponder::EnumObject from(bool)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Boolean,    ponder::ValueKind::Enum));}
    static bool can_from(bool)                     {return false;}
    static ponder::EnumObject from(long)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Integer,    ponder::ValueKind::Enum));}
    static bool can_from(long)                     {return false;}
    static ponder::EnumObject from(long long)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::LongInteger,ponder::ValueKind::Enum));}
    static bool can_from(long long)                {return false;}
    static ponder::EnumObject from(double)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Real,       ponder::ValueKind::Enum));}
    static bool can_from(double)                   {return false;}
    static ponder::EnumObject from(const ponder::String&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::String,     ponder::ValueKind::Enum));}
    static bool can_from(const ponder::String&)    {return false;}
    static ponder::EnumObject from(const ponder::UserObject&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Enum,       ponder::ValueKind::Enum));}
    static bool can_from(const ponder::UserObject&)    {return false;}
    static ponder::EnumObject from(const ponder::detail::ValueRef&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Reference,  ponder::ValueKind::Enum));}
    static bool can_from(const ponder::detail::ValueRef&)    {return false;}
};

/**
 * Specialization of ValueMapper for ponder::ValueKind.
 */
template <>
struct ValueMapper<ponder::ValueKind>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::String;
    static ponder::String to(ponder::ValueKind source)
        {return ponder::detail::valueKindAsString(source);}
};

/**
 * Specialization of ValueMapper for UserObject
 */
template <>
struct ValueMapper<ponder::UserObject>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::User;
    static const ponder::UserObject& to(const ponder::UserObject& source) {return source;}
    static const ponder::UserObject& from(const ponder::UserObject& source) {return source;}
    static bool can_from(const ponder::UserObject&)    {return true;}

    static ponder::UserObject from(bool)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Boolean,    ponder::ValueKind::User));}
    static bool can_from(bool)                     {return false;}
    static ponder::UserObject from(long)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Integer,    ponder::ValueKind::User));}
    static bool can_from(long)                     {return false;}
    static ponder::UserObject from(long long)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::LongInteger,ponder::ValueKind::User));}
    static bool can_from(long long)                {return false;}
    static ponder::UserObject from(double)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Real,       ponder::ValueKind::User));}
    static bool can_from(double)                   {return false;}
    static ponder::UserObject from(const ponder::String&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::String,     ponder::ValueKind::User));}
    static bool can_from(const ponder::String&)    {return false;}
    static ponder::UserObject from(const ponder::EnumObject&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Enum,       ponder::ValueKind::User));}
    static bool can_from(const ponder::EnumObject&)    {return false;}
    static ponder::UserObject from(const ponder::detail::ValueRef&)
        {PONDER_ERROR(ponder::BadType(ponder::ValueKind::Reference,  ponder::ValueKind::User));}
    static bool can_from(const ponder::detail::ValueRef&)    {return false;}
};

/**
 * Specialization of ValueMapper for void.
 * Conversion to void should never happen, the only aim of this
 * specialization is to define the proper type mapping.
 */
template <>
struct ValueMapper<void>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::None;
};

/**
 * Specialization of ValueMapper for NoType.
 * Conversion to NoType should never happen, the only aim of this
 * specialization is to define the proper mapped type.
 */
template <>
struct ValueMapper<ponder::NoType>
{
    static constexpr ponder::ValueKind kind = ponder::ValueKind::None;
};

/*----------------------------------------------------------------------
 * Modifiers.
 *  - Modify type to avoid supporting every variation above, e.g. const.
 */

/**
 * Show error for references. Not allowed.
 */
template <typename T>
struct ValueMapper<const T&>
{
    using ReferencesNotAllowed = int[-static_cast<int>(sizeof(T))];
};

/**
 * Show error for references using smart pointers.
 */
template <template <typename> class T, typename U>
struct ValueMapper<T<U>,
                   std::enable_if_t<ponder::detail::IsSmartPointer<T<U>,U>::value>>
{
    using ReferencesNotAllowed = int[-static_cast<int>(sizeof(U))];
};

/** \endcond NoDocumentation */

} // namespace ponder_ext

#endif // PONDER_VALUEMAPPER_HPP
