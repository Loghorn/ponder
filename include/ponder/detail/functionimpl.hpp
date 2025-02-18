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
#ifndef PONDER_DETAIL_FUNCTIONIMPL_HPP
#define PONDER_DETAIL_FUNCTIONIMPL_HPP

#include <ponder/function.hpp>
#include <ponder/detail/functiontraits.hpp>
#include <ponder/valuemapper.hpp>
#include <ponder/uses/uses.hpp>

namespace ponder {
namespace detail {

//--------------------------------------------------------------------------------------

struct FunctionParamInfo
{
    const std::type_info *m_typeinfo;
    ValueKind m_valueType;
};

template <int SZ>
struct FunctionMapParamsToValueKind
{
    using ReturnType = std::array<FunctionParamInfo, SZ>;

    template <typename T>
    static FunctionParamInfo apply() { return { &typeid(T), mapType<T>() }; }
};


// Apply B to canonical function F = R(A...)
template <typename FuncArgTuple, typename B> struct FunctionApplyToParams;

template <typename B, typename... A>
struct FunctionApplyToParams<std::tuple<A...>, B>
{
    static typename B::ReturnType foreach()
    {
        return typename B::ReturnType { {B::template apply<A>()...} };
    }
};

template <typename B>
struct FunctionApplyToParams<std::tuple<void>, B>
{
    static typename B::ReturnType foreach()
    {
        return typename B::ReturnType {};
    }
};

template <typename B>
struct FunctionApplyToParams<Args, B>
{
    static typename B::ReturnType foreach()
    {
        return typename B::ReturnType {B::template apply<Args>()};
    }
};

//--------------------------------------------------------------------------------------

template <typename R, typename... P>
struct ReturnPolicy
{
    static constexpr policy::ReturnKind kind = policy::ReturnKind::Copy;   // default
};

template <>
struct ReturnPolicy<void>   // nothing returned
{
    static constexpr policy::ReturnKind kind = policy::ReturnKind::NoReturn;
};

template <typename R>
struct ReturnPolicy<R, policy::ReturnInternalRef>
{
    static constexpr policy::ReturnKind kind = policy::ReturnKind::InternalRef;
};

//--------------------------------------------------------------------------------------
// FunctionImpl
//--------------------------------------------------------------------------------------

/*
 *  Bake the function type information into non-templated Function.
 */
template <typename T, typename F, typename... P>
class FunctionImpl : public Function
{
    using FuncTraits = T;
    using FuncPolicies = std::tuple<P...>;

    static constexpr size_t c_nParams = FuncTraits::Details::c_nParams;

    std::array<FunctionParamInfo, c_nParams> m_paramInfo;

public:

    FunctionImpl(IdRef name, F function, P...) : Function(name)
    {
        m_funcType = FuncTraits::kind;
        m_returnType = mapType<typename FuncTraits::ExposedType>();
        m_returnPolicy = ReturnPolicy<typename FuncTraits::ExposedType, P...> ::kind;
        m_paramInfo = FunctionApplyToParams<typename FuncTraits::Details::ParamTypes,
                                            FunctionMapParamsToValueKind<c_nParams>>::foreach();
        m_usesData = &m_userData;

        processUses<uses::Uses::eRuntimeModule>(m_name, function);
        PONDER_IF_LUA(processUses<uses::Uses::eLuaModule>(m_name, function);)
    }

    FunctionImpl(const FunctionImpl&) = delete;
    FunctionImpl& operator=(const FunctionImpl&) = delete;
    FunctionImpl(FunctionImpl&&) = delete;
    FunctionImpl& operator=(FunctionImpl&&) = delete;

private:

    uses::Uses::PerFunctionUserData m_userData;

    template <int M>
    void processUses(IdRef name, F function)
    {
        using Processor = std::tuple_element_t<M, uses::Uses::Users>;

        std::get<M>(m_userData) =
            Processor::template perFunction<F, T, FuncPolicies>(name, function);
    }

    [[nodiscard]] size_t paramCount() const override { return c_nParams; }

    [[nodiscard]] ValueKind paramType(size_t index) const override
    {
        // Make sure that the index is not out of range
        if (index >= c_nParams)
            PONDER_ERROR(OutOfRange(index, c_nParams));

        return m_paramInfo[index].m_valueType;
    }
};

// Used by ClassBuilder to create new function instance.
template <typename F, typename... P>
static std::shared_ptr<Function> newFunction(IdRef name, F function, P... policies)
{
    using FuncTraits = FunctionTraits<F>;

    static_assert(FuncTraits::kind != FunctionKind::None, "Type is not a function");

    return std::make_shared<FunctionImpl<FuncTraits, F, P...>>(name, function, policies...);
}

} // namespace detail
} // namespace ponder

#endif // PONDER_DETAIL_FUNCTIONIMPL_HPP
