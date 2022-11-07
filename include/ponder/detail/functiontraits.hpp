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
#ifndef PONDER_DETAIL_FUNCTIONTRAITS_HPP
#define PONDER_DETAIL_FUNCTIONTRAITS_HPP

#include <ponder/type.hpp>
#include "objecttraits.hpp"
#include <type_traits>
#include <array>
#include <functional>

namespace ponder {
namespace detail {
namespace function {

template <typename T>
struct FunctionDetails {};

template <typename R, typename... A>
struct FunctionDetails<R(*)(A...)>
{
    using ParamTypes = std::tuple<A...>;
    using ReturnType = R;
    using FuncType = ReturnType(*)(A...);
    using DispatchType = ReturnType(A...);
    using FunctionCallTypes = std::tuple<A...>;
	static constexpr size_t c_nParams = sizeof...(A);
};

template <typename R, typename... A>
struct FunctionDetails<R(A...)>
{
    using ParamTypes = std::tuple<A...>;
    using ReturnType = R;
    using FuncType = ReturnType(*)(A...);
    using DispatchType = ReturnType(A...);
    using FunctionCallTypes = std::tuple<A...>;
	static constexpr size_t c_nParams = sizeof...(A);
};

template <typename R, typename... A>
struct FunctionDetails<R(*)(A...) noexcept>
{
    using ParamTypes = std::tuple<A...>;
    using ReturnType = R;
    using FuncType = ReturnType(*)(A...);
    using DispatchType = ReturnType(A...);
    using FunctionCallTypes = std::tuple<A...>;
	static constexpr size_t c_nParams = sizeof...(A);
};

template <typename R, typename... A>
struct FunctionDetails<R(A...) noexcept>
{
    using ParamTypes = std::tuple<A...>;
    using ReturnType = R;
    using FuncType = ReturnType(*)(A...);
    using DispatchType = ReturnType(A...);
    using FunctionCallTypes = std::tuple<A...>;
	static constexpr size_t c_nParams = sizeof...(A);
};

template <typename R>
struct FunctionDetails<R(*)(Args)>
{
    using ParamTypes = Args;
    using ReturnType = R;
    using FuncType = ReturnType(*)(Args);
    using DispatchType = ReturnType(Args);
    using FunctionCallTypes = Args;
	static constexpr size_t c_nParams = 1;
};

template <typename R>
struct FunctionDetails<R(Args)>
{
    using ParamTypes = Args;
    using ReturnType = R;
    using FuncType = ReturnType(*)(Args);
    using DispatchType = ReturnType(Args);
    using FunctionCallTypes = Args;
	static constexpr size_t c_nParams = 1;
};

template <typename R>
struct FunctionDetails<R(*)(Args) noexcept>
{
    using ParamTypes = Args;
    using ReturnType = R;
    using FuncType = ReturnType(*)(Args);
    using DispatchType = ReturnType(Args);
    using FunctionCallTypes = Args;
	static constexpr size_t c_nParams = 1;
};

template <typename R>
struct FunctionDetails<R(Args) noexcept>
{
    using ParamTypes = Args;
    using ReturnType = R;
    using FuncType = ReturnType(*)(Args);
    using DispatchType = ReturnType(Args);
    using FunctionCallTypes = Args;
	static constexpr size_t c_nParams = 1;
};

template <typename R>
struct FunctionDetails<R(*)(const Args&)>
{
    using ParamTypes = Args;
    using ReturnType = R;
    using FuncType = ReturnType(*)(Args);
    using DispatchType = ReturnType(Args);
    using FunctionCallTypes = Args;
	static constexpr size_t c_nParams = 1;
};

template <typename R>
struct FunctionDetails<R(const Args&)>
{
    using ParamTypes = Args;
    using ReturnType = R;
    using FuncType = ReturnType(*)(Args);
    using DispatchType = ReturnType(Args);
    using FunctionCallTypes = Args;
	static constexpr size_t c_nParams = 1;
};

template <typename R>
struct FunctionDetails<R(*)(const Args&) noexcept>
{
    using ParamTypes = Args;
    using ReturnType = R;
    using FuncType = ReturnType(*)(Args);
    using DispatchType = ReturnType(Args);
    using FunctionCallTypes = Args;
	static constexpr size_t c_nParams = 1;
};

template <typename R>
struct FunctionDetails<R(const Args&) noexcept>
{
    using ParamTypes = Args;
    using ReturnType = R;
    using FuncType = ReturnType(*)(Args);
    using DispatchType = ReturnType(Args);
    using FunctionCallTypes = Args;
	static constexpr size_t c_nParams = 1;
};


// Class method
template <typename T>
struct MethodDetails {};

// Non-const method.
template <typename C, typename R, typename... A>
struct MethodDetails<R(C::*)(A...)>
{
    using ClassType = C;
    using ParamTypes = std::tuple<A...>;
    using ReturnType = R;
    using FuncType = ReturnType(ClassType::*)(A...);
    using DispatchType = ReturnType(ClassType&, A...);
    using FunctionCallTypes = std::tuple<ClassType&, A...>;
    static constexpr bool isConst = false;
	static constexpr size_t c_nParams = sizeof...(A);
};

// Const method.
template <typename C, typename R, typename... A>
struct MethodDetails<R(C::*)(A...) const>
{
    using ClassType = const C;
    using ParamTypes = std::tuple<A...>;
    using ReturnType = R;
    using DispatchType = ReturnType(const ClassType&, A...);
    using FuncType = ReturnType(ClassType::*)(A...) const;
    using FunctionCallTypes = std::tuple<const ClassType&, A...>;
    static constexpr bool isConst = true;
	static constexpr size_t c_nParams = sizeof...(A);
};

// Non-const noexcept method.
template <typename C, typename R, typename... A>
struct MethodDetails<R(C::*)(A...) noexcept>
{
    using ClassType = C;
    using ParamTypes = std::tuple<A...>;
    using ReturnType = R;
    using FuncType = ReturnType(ClassType::*)(A...);
    using DispatchType = ReturnType(ClassType&, A...);
    using FunctionCallTypes = std::tuple<ClassType&, A...>;
    static constexpr bool isConst = false;
	static constexpr size_t c_nParams = sizeof...(A);
};

// Const noexcept method.
template <typename C, typename R, typename... A>
struct MethodDetails<R(C::*)(A...) const noexcept>
{
    using ClassType = const C;
    using ParamTypes = std::tuple<A...>;
    using ReturnType = R;
    using DispatchType = ReturnType(const ClassType&, A...);
    using FuncType = ReturnType(ClassType::*)(A...) const;
    using FunctionCallTypes = std::tuple<const ClassType&, A...>;
    static constexpr bool isConst = true;
	static constexpr size_t c_nParams = sizeof...(A);
};


template<typename T, typename = void>
struct IsCallable : std::false_type {};

template<typename T>
struct IsCallable<T,
                  std::enable_if_t< std::is_same_v<decltype(void(&T::operator())), void>
                             && !std::is_function_v<T> >> : std::true_type
{};

template <typename T, typename U = void>
struct IsFunctionWrapper : std::false_type {};

template <typename T>
struct IsFunctionWrapper<std::function<T>> : std::true_type {};

// T::operator() callable
template <typename T>
struct CallableDetails : CallableDetails<decltype(&T::operator())>
{
    using FuncType = T;
};

template <typename L, typename R, typename... A>
struct CallableDetails<R(L::*)(A...) const>
{
    using LambdaClassType = L;    // N.B. Lambda class
    using ParamTypes = std::tuple<A...>;
    using ReturnType = R;
    using DispatchType = ReturnType(A...);
    using FunctionCallTypes = std::tuple<A...>;
	static constexpr size_t c_nParams = sizeof...(A);
};

template <typename L, typename R>
struct CallableDetails<R(L::*)(Args) const>
{
    using LambdaClassType = L;    // N.B. Lambda class
    using ParamTypes = Args;
    using ReturnType = R;
    using DispatchType = ReturnType(Args);
    using FunctionCallTypes = Args;
	static constexpr size_t c_nParams = 1;
};

template <typename L, typename R>
struct CallableDetails<R(L::*)(const Args&) const>
{
    using LambdaClassType = L;    // N.B. Lambda class
    using ParamTypes = Args;
    using ReturnType = R;
    using DispatchType = ReturnType(Args);
    using FunctionCallTypes = Args;
	static constexpr size_t c_nParams = 1;
};

/*
 * For functions, if they return an rvalue, we cannot return by reference.
 */
template <typename T, bool isWritable = true>
struct ReturnType
{
    using Type = T&; // Not a rvalue so we can reference
};

template <>
struct ReturnType<void>
{
    using Type = void;
};

template <typename T>
struct ReturnType<T, false>
{
    using Type = T; // T is an rvalue so return by value
};

} // namespace function

/*
 * Uniform type declaration to all function types.
 *  - Used by property and function declaration, so not class specific.
 *  - BoundType - the exposer, e.g. member function or pointer.
 *  - DataType - scalar non-const/non-ref return type. E.g. int.
 *  - ExposedType - real type exposed, const/ref, e.g. int[], const int.
 *  - getter/setter are both const functions but may reference non-const objects.
 *  - getter returns ExposedType and is set via DataType, which may be component, e.g. int[]
 */
template <typename T, typename E = void>
struct FunctionTraits
{
    static constexpr FunctionKind kind = FunctionKind::None;
};

/*
 * Specialization for native callable types (function and function pointer types)
 *  - We cannot derive a ClassType from these as they may not have one. e.g. int get()
 */
template <typename T>
struct FunctionTraits<T,
                      std::enable_if_t<std::is_function_v<std::remove_pointer_t<T>>>>
{
    static constexpr FunctionKind kind = FunctionKind::Function;
    using Details = function::FunctionDetails<std::remove_pointer_t<T>>;
    using BoundType = typename Details::FuncType;
    using ExposedType = typename Details::ReturnType;
    using ExposedTraits = TypeTraits<ExposedType>;
    static constexpr bool isWritable = std::is_lvalue_reference_v<ExposedType>
                                       && !std::is_const_v<typename ExposedTraits::DereferencedType>;
    using AccessType = typename function::ReturnType<typename ExposedTraits::DereferencedType, isWritable>::Type;
    using DataType = typename ExposedTraits::DataType;
    using DispatchType = typename Details::DispatchType;

    template <typename C, typename A>
    class Binding
    {
    public:
        using ClassType = C;
        using AccessType = A;

        Binding(BoundType d) : data(d) {}

        AccessType access(ClassType& c) const {return (*data)(c);}
    private:
        BoundType data;
    };
};

/*
 * Specialization for native callable types (member function types)
 */
template <typename T>
struct FunctionTraits<T, std::enable_if_t<std::is_member_function_pointer_v<T>>>
{
    static constexpr FunctionKind kind = FunctionKind::MemberFunction;
    using Details = function::MethodDetails<T>;
    using BoundType = typename Details::FuncType;
    using ExposedType = typename Details::ReturnType;
    using ExposedTraits = TypeTraits<ExposedType>;
    static constexpr bool isWritable = std::is_lvalue_reference_v<ExposedType> && !Details::isConst;
    using AccessType = typename function::ReturnType<typename ExposedTraits::DereferencedType, isWritable>::Type;
    using DataType = typename ExposedTraits::DataType;
    using DispatchType = typename Details::DispatchType;

    template <typename C, typename A>
    class Binding
    {
    public:
        using ClassType = C;
        using AccessType = A;

        Binding(BoundType d) : data(d) {}

        AccessType access(ClassType& c) const {return (c.*data)();}
    private:
        BoundType data;
    };
};

/**
 * Specialization for functors (classes exporting a result_type type, T operator() ).
 */
template <typename T>
struct FunctionTraits<T, std::enable_if_t<std::is_bind_expression_v<T>>>
{
    static constexpr FunctionKind kind = FunctionKind::BindExpression;
    using Details = function::CallableDetails<T>;
    using BoundType = typename Details::FuncType;
    using ExposedType = typename Details::ReturnType;
    using ExposedTraits = TypeTraits<ExposedType>;
    static constexpr bool isWritable = std::is_lvalue_reference_v<ExposedType>
                                       && !std::is_const_v<typename ExposedTraits::DereferencedType>;
    using AccessType = typename function::ReturnType<typename ExposedTraits::DereferencedType, isWritable>::Type;
    using DataType = typename ExposedTraits::DataType;
    using DispatchType = typename Details::DispatchType;

    template <typename C, typename A>
    class Binding
    {
    public:
        using ClassType = C;
        using AccessType = A;

        Binding(BoundType d) : data(d) {}

        AccessType access(ClassType& c) const {return data(c);}
    private:
        BoundType data;
    };
};

/**
 * Specialization for function wrappers (std::function<>).
 */
template <typename T>
struct FunctionTraits<T,
                      std::enable_if_t<function::IsCallable<T>::value
                            && function::IsFunctionWrapper<T>::value>>
{
    static constexpr FunctionKind kind = FunctionKind::FunctionWrapper;
    using Details = function::CallableDetails<T>;
    using BoundType = typename Details::FuncType;
    using ExposedType = typename Details::ReturnType;
    using ExposedTraits = TypeTraits<ExposedType>;
    static constexpr bool isWritable = std::is_lvalue_reference_v<ExposedType>
                                       && !std::is_const_v<typename ExposedTraits::DereferencedType>;
    using AccessType = typename function::ReturnType<typename ExposedTraits::DereferencedType, isWritable>::Type;
    using DataType = typename ExposedTraits::DataType;
    using DispatchType = typename Details::DispatchType;

    template <typename C, typename A>
    class Binding
    {
    public:
        using ClassType = C;
        using AccessType = A;

        Binding(BoundType d) : data(d) {}

        AccessType access(ClassType& c) const {return data(c);}
    private:
        BoundType data;
    };
};

/**
 * Specialization for lambda functions ([](){}).
 */
template <typename T>
struct FunctionTraits<T,
                      std::enable_if_t<function::IsCallable<T>::value
                            && !function::IsFunctionWrapper<T>::value>>
{
    static constexpr FunctionKind kind = FunctionKind::Lambda;
    using Details = function::CallableDetails<T>;
    using BoundType = T;
    using ExposedType = typename Details::ReturnType;
    using ExposedTraits = TypeTraits<ExposedType>;
    static constexpr bool isWritable = std::is_lvalue_reference_v<ExposedType>
                                       && !std::is_const_v<typename ExposedTraits::DereferencedType>;
    using AccessType = typename function::ReturnType<typename ExposedTraits::DereferencedType, isWritable>::Type;
    using DataType = typename ExposedTraits::DataType;
    using DispatchType = typename Details::DispatchType;

    template <typename C, typename A>
    class Binding
    {
    public:
        using ClassType = C;
        using AccessType = A;

        Binding(BoundType d) : data(d) {}

        AccessType access(ClassType& c) const {return data(c);}
    private:
        BoundType data;
    };
};

/*
 * Uniform access to the member type T.
 */
template <typename T>
struct MemberTraits;

template <typename C, typename T>
struct MemberTraits<T C::*>
{
    using BoundType = T C::*;                               // full type inc ref
    using ExposedType = T;                                   // the type exposed inc refs
    using ExposedTraits = TypeTraits<ExposedType>;
    using DataType = typename ExposedTraits::DataType;       // raw type or container
    static constexpr bool isWritable = !std::is_const_v<typename ExposedTraits::DereferencedType>;
    using AccessType = typename ExposedTraits::DereferencedType;

    template <typename BC, typename A>
    class Binding
    {
    public:
        using ClassType = BC;
        using AccessType = A;

        Binding(const BoundType& d) : data(d) {}

        AccessType access(ClassType& c) const { return c.*data; }
    private:
        BoundType data;
    };
};

} // namespace detail
} // namespace ponder

#endif // PONDER_DETAIL_FUNCTIONTRAITS_HPP
