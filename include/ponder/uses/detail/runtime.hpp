/****************************************************************************
**
** This file is part of the Ponder library.
**
** The MIT License (MIT)
**
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
#ifndef PONDER_USES_RUNTIME_IMPL_HPP
#define PONDER_USES_RUNTIME_IMPL_HPP

#include <functional>
#include <ponder/detail/rawtype.hpp>
#include <ponder/detail/util.hpp>

namespace ponder {
namespace runtime {
namespace detail {

//-----------------------------------------------------------------------------
// Handle returning copies

template <typename R, typename U = void> struct CallReturnCopy;

template <typename R>
struct CallReturnCopy<R, std::enable_if_t<!ponder::detail::IsUserType<R>::value>>
{
    static Value value(R&& o) {return Value(o);}
};

template <typename R>
struct CallReturnCopy<R, std::enable_if_t<ponder::detail::IsUserType<R>::value>>
{
    static_assert(!std::is_pointer_v<R>, "Cannot return unowned pointer. Use ponder::policy::ReturnInternalRef?");
    static Value value(R&& o) {return Value(UserObject::makeCopy(std::forward<R>(o)));}
};

//-----------------------------------------------------------------------------
// Handle returning internal references

template <typename R, typename U = void> struct CallReturnInternalRef;

template <typename R>
struct CallReturnInternalRef<R,
                             std::enable_if_t<
        !ponder::detail::IsUserType<R>::value
        && !std::is_same_v<typename ponder::detail::DataType<R>::Type, UserObject>
    >>
{
    static Value value(R&& o) {return Value(o);}
};

template <typename R>
struct CallReturnInternalRef<R,
                             std::enable_if_t<
        ponder::detail::IsUserType<R>::value
        || std::is_same_v<typename ponder::detail::DataType<R>::Type, UserObject>
    >>
{
    static Value value(R&& o) {return Value(UserObject::makeRef(std::forward<R>(o)));}
};

//-----------------------------------------------------------------------------
// Choose which returner to use, based on policy
//  - map policy kind to actionable policy type

template <typename Policies_t, typename R> struct ChooseCallReturner;

template <typename... Ps, typename R>
struct ChooseCallReturner<std::tuple<policy::ReturnCopy, Ps...>, R>
{
    using type = CallReturnCopy<R>;
};

template <typename... Ps, typename R>
struct ChooseCallReturner<std::tuple<policy::ReturnInternalRef, Ps...>, R>
{
    using type = CallReturnInternalRef<R>;
};

template <typename R>
struct ChooseCallReturner<std::tuple<>, R> // default
{
    using type = CallReturnCopy<R>;
};

template <typename P, typename... Ps, typename R>
struct ChooseCallReturner<std::tuple<P, Ps...>, R> // recurse
{
    using type = typename ChooseCallReturner<std::tuple<Ps...>, R>::type;
};

//-----------------------------------------------------------------------------

/*
 * Helper function which converts an argument to a C++ type
 *
 * The main purpose of this function is to convert any BadType error to
 * a BadArgument one.
 */
template <int TFrom, typename TTo>
struct ConvertArg
{
    using ReturnType = std::remove_reference_t<TTo>;
    static ReturnType
    convert(const Args& args, size_t index)
    {
        try {
            return args[index].to<std::remove_reference_t<TTo>>();
        }
        catch (const BadType&) {
            PONDER_ERROR(BadArgument(args[index].kind(), mapType<TTo>(), index, "?"));
        }
    }
};

// Specialisation for returning references.
template <typename TTo>
struct ConvertArg<static_cast<int>(ValueKind::User), TTo&>
{
    using ReturnType = TTo&;
    static ReturnType
    convert(const Args& args, size_t index)
    {
        auto&& uobj = const_cast<Value&>(args[index]).ref<UserObject>();
        if (uobj.pointer() == nullptr)
            PONDER_ERROR(NullObject(&uobj.getClass()));
        return uobj.ref<TTo>();
    }
};

// Specialisation for returning const references.
template <typename TTo>
struct ConvertArg<static_cast<int>(ValueKind::User), const TTo&>
{
    using ReturnType = const TTo&;
    static ReturnType
    convert(const Args& args, size_t index)
    {
        auto&& uobj = args[index].cref<UserObject>();
        if (uobj.pointer() == nullptr)
            PONDER_ERROR(NullObject(&uobj.getClass()));
        return uobj.cref<TTo>();
    }
};

//-----------------------------------------------------------------------------
// Object function call helper to allow specialisation by return type. Applies policies.

template <typename A>
struct ConvertArgs
{
    using Raw = typename ponder::detail::DataType<A>::Type;
    static constexpr ValueKind kind = ponder_ext::ValueMapper<Raw>::kind;
    using Convertor = ConvertArg<static_cast<int>(kind), A>;

    static typename Convertor::ReturnType convert(const Args& args, size_t index)
    {
        return Convertor::convert(args, index);
    }
};

template <typename R, typename FTraits, typename FPolicies>
class CallHelper
{
public:

    template<typename F, typename... A, size_t... Is>
    static Value call(F func, const Args& args, std::index_sequence<Is...>)
    {
        using CallReturner = typename ChooseCallReturner<FPolicies, R>::type;
        return CallReturner::value(func(ConvertArgs<A>::convert(args, Is)...));
    }

    template<typename F>
    static Value call(F func, const Args& args)
    {
        using CallReturner = typename ChooseCallReturner<FPolicies, R>::type;
        return CallReturner::value(func(args));
    }
};

// Specialization of CallHelper for functions returning void
template <typename FTraits, typename FPolicies>
class CallHelper<void, FTraits, FPolicies>
{
public:

    template<typename F, typename... A, size_t... Is>
    static Value call(F func, const Args& args, std::index_sequence<Is...>)
    {
        func(ConvertArgs<A>::convert(args,Is)...);
        return Value::nothing;
    }

    template<typename F>
    static Value call(F func, const Args& args)
    {
        func(args);
        return Value::nothing;
    }
};

//-----------------------------------------------------------------------------
// Convert traits to callable function wrapper. Generic for all function types.

template <typename R, typename A> struct FunctionWrapper;

template <typename R, typename... A> struct FunctionWrapper<R, std::tuple<A...>>
{
    using Type = std::function<R(A...)>;

    template <typename F, typename FTraits, typename FPolicies>
    static Value call(F func, const Args& args)
    {
        using ArgEnumerator = std::make_index_sequence<sizeof...(A)>;
        return CallHelper<R, FTraits, FPolicies>::template
            call<F, A...>(func, args, ArgEnumerator());
    }
};

template <typename R> struct FunctionWrapper<R, Args>
{
    using Type = std::function<R(Args)>;

    template <typename F, typename FTraits, typename FPolicies>
    static Value call(F func, const Args& args)
    {
        return CallHelper<R, FTraits, FPolicies>::template
            call<F>(func, args);
    }
};

//-----------------------------------------------------------------------------
// Base for runtime function caller

class FunctionCaller
{
public:
    FunctionCaller(const IdRef name) : m_name(name) {}
    virtual ~FunctionCaller() = default;

    FunctionCaller(const FunctionCaller&) = delete; // no copying

    [[nodiscard]] IdRef name() const { return m_name; }

    [[nodiscard]] virtual Value execute(const Args& args) const = 0;

private:
    const IdRef m_name;
};

// The FunctionImpl class is a template which is specialized according to the
// underlying function prototype.
template <typename F, typename FTraits, typename FPolicies>
class FunctionCallerImpl final : public FunctionCaller
{
public:

    FunctionCallerImpl(IdRef name, F function)
    :   FunctionCaller(name)
    ,   m_function(function)
    {}

private:

    using CallTypes = typename FTraits::Details::FunctionCallTypes;
    using DispatchType = FunctionWrapper<typename FTraits::ExposedType, CallTypes>;

    typename DispatchType::Type m_function; // Object containing the actual function to call

    [[nodiscard]] Value execute(const Args& args) const override
    {
        return DispatchType::template
            call<decltype(m_function), FTraits, FPolicies>(m_function, args);
    }
};

} // namespace detail
} // namespace runtime
} // namespace ponder

#endif // PONDER_USES_RUNTIME_IMPL_HPP
