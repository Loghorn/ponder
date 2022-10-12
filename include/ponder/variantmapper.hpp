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
#ifndef PONDER_VARIANTMAPPER_HPP
#define PONDER_VARIANTMAPPER_HPP

#include <ponder/config.hpp>
#include <ponder/enum.hpp>
#include <ponder/enumobject.hpp>
#include <ponder/userobject.hpp>
#include <ponder/errors.hpp>
#include <ponder/detail/util.hpp>
#include <ponder/detail/valueref.hpp>
#include <ponder/uses/runtime.hpp>
#include <ponder/pondertype.hpp>

#include <variant>

namespace ponder_ext {
namespace detail {

    template <typename... Ts>
    class VariantProperty : public ponder::SimpleProperty
    {
        template <typename T, typename... Rest>
        static void variant_assign(std::variant<Ts...> &var, const ponder::UserObject &val)
        {
            const ponder::Class *targetClass = ponder::classByTypeSafe<T>();
            if (!targetClass || val.getClass() != *targetClass)
            {
                if constexpr (sizeof...(Rest) > 0)
                    variant_assign<Rest...>(var, val);
            }
            else
            {
                var = val.get<T>();
            }
        }

        template <typename Tval, typename T, typename... Rest>
        static void variant_assign(std::variant<Ts...> &var, const ponder::Value &val)
        {
            if constexpr (!std::is_convertible_v<Tval, T>)
            {
                if constexpr (sizeof...(Rest) > 0)
                    variant_assign<Tval, Rest...>(var, val);
            }
            else
            {
                var = val.to<T>();
            }
        }

    public:
        VariantProperty(bool typeInfo)
            : SimpleProperty(typeInfo ? "t" : "v", typeInfo ? ponder::ValueKind::String : ponder::ValueKind::None), m_typeInfo(typeInfo)
        {
        }

        ponder::Value getValue(const ponder::UserObject &object) const override
        {
            if (m_holder != ponder::UserObject::nothing)
            {
                return m_holder;
            }

            auto *v = object.get<std::variant<Ts...> *>();
            ponder::Value value = std::visit(
                [](auto &&val)
                {
                    return ponder::Value(val);
                },
                *v);

            if (m_typeInfo)
            {
                switch (value.kind())
                {
                default:
                case ponder::ValueKind::None:
                    return "error";
                case ponder::ValueKind::Boolean:
                    return "bool";
                case ponder::ValueKind::Integer:
                    return "long";
                case ponder::ValueKind::LongInteger:
                    return "long long";
                case ponder::ValueKind::Real:
                    return "double";
                case ponder::ValueKind::String:
                    return "string";
                case ponder::ValueKind::Enum:
                    return "enum";
                case ponder::ValueKind::User:
                    return value.cref<ponder::UserObject>().getClass().name();
                    break;
                }
            }
            return value;
        }

        void setValue(const ponder::UserObject &object, const ponder::Value &value) const override
        {
            if (m_typeInfo)
            {
                auto type = value.to<std::string>();

                auto mc = ponder::classByNameSafe(type);
                if (mc)
                {
                    m_holder = ponder::runtime::create(*mc);
                }
                return;
            }

            auto *v = object.get<std::variant<Ts...> *>();
            switch (value.kind())
            {
            default:
            case ponder::ValueKind::None:
                break;
            case ponder::ValueKind::Boolean:
                if constexpr (std::disjunction_v<std::is_same<bool, Ts>...>)
                    *v = value.to<bool>();
                else
                    variant_assign<bool, Ts...>(*v, value);
                break;
            case ponder::ValueKind::Integer:
                if constexpr (std::disjunction_v<std::is_same<long, Ts>...>)
                    *v = value.to<long>();
                else
                    variant_assign<long, Ts...>(*v, value);
                break;
            case ponder::ValueKind::LongInteger:
                if constexpr (std::disjunction_v<std::is_same<long long, Ts>...>)
                    *v = value.to<long long>();
                else
                    variant_assign<long long, Ts...>(*v, value);
                break;
            case ponder::ValueKind::Real:
                if constexpr (std::disjunction_v<std::is_same<double, Ts>...> || std::disjunction_v<std::is_same<long double, Ts>...>)
                    *v = value.to<double>();
                else if constexpr (std::disjunction_v<std::is_same<float, Ts>...>)
                    *v = value.to<float>();
                else
                    variant_assign<double, Ts...>(*v, value);
                break;
            case ponder::ValueKind::String:
                if constexpr (std::disjunction_v<std::is_same<std::string, Ts>...>)
                    *v = value.to<std::string>();
                else
                    variant_assign<std::string, Ts...>(*v, value);
                break;
            case ponder::ValueKind::Enum:
                if constexpr (std::disjunction_v<std::is_same<int, Ts>...>)
                    *v = value.to<int>();
                else
                    variant_assign<int, Ts...>(*v, value);
                break;
            case ponder::ValueKind::User:
                variant_assign<Ts...>(*v, value.to<ponder::UserObject>());
                break;
            }
            m_holder = ponder::UserObject::nothing;
        }

        bool isReadable() const override
        {
            return true;
        }

        bool isWritable() const override
        {
            return true;
        }

    private:
        bool m_typeInfo;
        static inline ponder::UserObject m_holder;
    };
}

template <typename T>
struct VariantMapper {};

template <typename... Ts>
struct VariantMapper<std::variant<Ts...>>
{
    static size_t propertyCount() { return 2; }

    static std::shared_ptr<ponder::Property> property(size_t index)
    {
        return std::make_shared<detail::VariantProperty<Ts...>>(index == 0);
    }

    static size_t functionCount() { return 0; }

    static std::shared_ptr<ponder::Function> function(size_t index) { return nullptr; }
};
}

#define PONDER_VARIANT_TYPE(...) \
    namespace ponder { namespace detail { \
        template<> struct StaticTypeDecl<__VA_ARGS__> { \
            static void registerFn() { \
                ponder::Class::declare<__VA_ARGS__>().external<ponder_ext::VariantMapper>(); \
            } \
            static TypeId id(bool checkRegister = true) { \
                if (checkRegister) detail::ensureTypeRegistered(calcTypeId<__VA_ARGS__>(), registerFn); \
                return calcTypeId<__VA_ARGS__>(); \
            } \
            static const char* name(bool checkRegister = true) { \
                if (checkRegister) detail::ensureTypeRegistered(calcTypeId<__VA_ARGS__>(), registerFn); \
                return #__VA_ARGS__; \
            } \
            static constexpr bool defined = true, copyable = true; \
        }; \
    }}

#endif
