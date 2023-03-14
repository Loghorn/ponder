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

    struct VariantHolder
    {
        size_t index;
        ponder::UserObject object;
    };

    template <typename... Ts>
    class VariantIndexProperty : public ponder::SimpleProperty
    {
    public:
        VariantIndexProperty()
            : SimpleProperty("i", ponder::ValueKind::LongInteger)
        {
        }

        ponder::Value getValue(const ponder::UserObject &object) const override
        {
            auto holder = getData<detail::VariantHolder>(object);
            if (!holder)
            {
                setData(object, std::make_shared<detail::VariantHolder>());
                holder = getData<detail::VariantHolder>(object);
            }
            auto *v = object.get<std::variant<Ts...> *>();
            holder->index = v->index();
            return holder->index;
        }

        void setValue(const ponder::UserObject &object, const ponder::Value &value) const override
        {
            auto holder = getData<detail::VariantHolder>(object);
            if (!holder)
            {
                setData(object, std::make_shared<detail::VariantHolder>());
                holder = getData<detail::VariantHolder>(object);
            }
            holder->index = static_cast<size_t>(value.to<long long>());
        }

        bool isReadable() const override
        {
            return true;
        }

        bool isWritable() const override
        {
            return true;
        }
    };

    template <typename... Ts>
    class VariantTypeProperty : public ponder::SimpleProperty
    {
    public:
        VariantTypeProperty()
            : SimpleProperty("t", ponder::ValueKind::String)
        {
        }

        ponder::Value getValue(const ponder::UserObject &object) const override
        {
            auto *v = object.get<std::variant<Ts...> *>();
            ponder::Value value = std::visit([](auto &&val) { return ponder::Value(val); }, *v);

            switch (value.kind())
            {
            default:
            case ponder::ValueKind::None: return "error";
            case ponder::ValueKind::Boolean: return "bool";
            case ponder::ValueKind::Integer: return "long";
            case ponder::ValueKind::LongInteger: return "long long";
            case ponder::ValueKind::Real: return "double";
            case ponder::ValueKind::String: return "string";
            case ponder::ValueKind::Enum: return "enum";
            case ponder::ValueKind::User: return value.cref<ponder::UserObject>().getClass().name();
            }
        }

        void setValue(const ponder::UserObject &object, const ponder::Value &value) const override
        {
            auto holder = getData<detail::VariantHolder>(object);

            auto type = value.to<std::string>();

            auto mc = ponder::classByNameSafe(type);
            if (mc)
            {
                if (!holder)
                {
                    setData(object, std::make_shared<detail::VariantHolder>());
                    holder = getData<detail::VariantHolder>(object);
                }

                holder->object = ponder::runtime::create(*mc);
            }
            else if (holder)
            {
                holder->object = ponder::UserObject::nothing;
            }
        }

        bool isReadable() const override
        {
            return true;
        }

        bool isWritable() const override
        {
            return true;
        }
    };

    template <typename... Ts>
    class VariantProperty : public ponder::SimpleProperty
    {
        struct expander
        {
            template<std::size_t I, std::size_t ...Is>
            static inline std::variant<Ts...> expand_type(std::size_t index, std::index_sequence<I, Is...>)
            {
                if constexpr (sizeof...(Is) > 0)
                    return index == I ? std::variant<Ts...>(std::in_place_index<I>) : expand_type(index, std::index_sequence<Is...>{});
                else
                    return std::variant<Ts...>(std::in_place_index<I>);
            }
        };

        [[nodiscard]] static constexpr auto expand_type(std::size_t index)
        {
            return expander::expand_type(index, std::index_sequence_for<Ts...>{});
        }

        struct assigner
        {
            assigner(std::variant<Ts...>& v, std::size_t index) : v(v), index(index) {}

            template<std::size_t I, std::size_t ...Is>
            inline auto assign(std::index_sequence<I, Is...>, const ponder::Value &value)
            {
                if constexpr(sizeof...(Is) > 0)
                {
                    if(index == I) 
                        v.template emplace<I>(value.to<std::variant_alternative_t<I, std::variant<Ts...>>>());
                    else 
                        assign(std::index_sequence<Is...>{}, value);
                }
                else
                    v.template emplace<I>(value.to<std::variant_alternative_t<I, std::variant<Ts...>>>());
            }

            template<std::size_t I, std::size_t ...Is>
            inline auto assign(std::index_sequence<I, Is...>, const ponder::UserObject &value)
            {
                if constexpr(sizeof...(Is) > 0)
                {
                    if(index == I) 
                        v.template emplace<I>(value.get<std::variant_alternative_t<I, std::variant<Ts...>>>());
                    else 
                        assign(std::index_sequence<Is...>{}, value);
                }
                else
                    v.template emplace<I>(value.get<std::variant_alternative_t<I, std::variant<Ts...>>>());
            }
        private:
            std::variant<Ts...>& v;
            std::size_t index;
        };


        static constexpr void assign_variant(std::variant<Ts...>& v, std::size_t index, const ponder::Value &value)
        {
            assigner{v, index}.assign(std::index_sequence_for<Ts...>{}, value);
        }

        static constexpr void assign_variant(std::variant<Ts...>& v, std::size_t index, const ponder::UserObject &value)
        {
            assigner{v, index}.assign(std::index_sequence_for<Ts...>{}, value);
        }

    public:
        VariantProperty()
            : SimpleProperty("v", ponder::ValueKind::None)
        {
        }

        ponder::Value getValue(const ponder::UserObject &object) const override
        {
            auto holder = getData<detail::VariantHolder>(object);
            if (!holder)
                PONDER_ERROR(ponder::ForbiddenCall("setData(object, std::make_shared<detail::VariantHolder>())"));

            if (holder->object != ponder::UserObject::nothing)
            {
                return holder->object;
            }

            auto *v = object.get<std::variant<Ts...> *>();
            if (v->index() == holder->index)
                return std::visit([](auto &&val) { return ponder::Value(val); }, *v);
            else
                return std::visit([](auto &&val) { return ponder::Value(val); }, expand_type(holder->index));
        }

        void setValue(const ponder::UserObject &object, const ponder::Value &value) const override
        {
            auto holder = getData<detail::VariantHolder>(object);
            if (!holder)
                PONDER_ERROR(ponder::ForbiddenCall("setData(object, std::make_shared<detail::VariantHolder>())"));

            auto *v = object.get<std::variant<Ts...> *>();
            switch (value.kind())
            {
            default:
            case ponder::ValueKind::None:
                break;
            case ponder::ValueKind::Boolean:
            case ponder::ValueKind::Integer:
            case ponder::ValueKind::LongInteger:
            case ponder::ValueKind::Real:
            case ponder::ValueKind::String:
            case ponder::ValueKind::Enum:
                assign_variant(*v, holder->index, value);
                break;
            case ponder::ValueKind::User:
                assign_variant(*v, holder->index, value.to<ponder::UserObject>());
                break;
            }
        }

        bool isReadable() const override
        {
            return true;
        }

        bool isWritable() const override
        {
            return true;
        }
    };
}

template <typename T>
struct VariantMapper {};

template <typename... Ts>
struct VariantMapper<std::variant<Ts...>>
{
    static size_t propertyCount() { return 3; }

    static std::shared_ptr<ponder::Property> property(size_t index)
    {
        if (index == 0)
            return std::make_shared<detail::VariantIndexProperty<Ts...>>();
        if (index == 1)
            return std::make_shared<detail::VariantTypeProperty<Ts...>>();
        return std::make_shared<detail::VariantProperty<Ts...>>();
    }

    static size_t functionCount() { return 0; }

    static std::shared_ptr<ponder::Function> function(size_t index) { return nullptr; }
};
}

namespace ponder { namespace detail {
    template<typename... Ts> struct StaticTypeDecl<std::variant<Ts...>> {
        static void registerFn() {
            ponder::Class::declare<std::variant<Ts...>>().external<ponder_ext::VariantMapper>();
        }
        static TypeId id(bool checkRegister = true) {
            if (checkRegister) detail::ensureTypeRegistered(calcTypeId<std::variant<Ts...>>(), registerFn);
            return calcTypeId<std::variant<Ts...>>();
        }
        static const char* name(bool checkRegister = true) {
            if (checkRegister) detail::ensureTypeRegistered(calcTypeId<std::variant<Ts...>>(), registerFn);
            return typeid(std::variant<Ts...>).name();
        }
        static constexpr bool defined = true, copyable = true;
    };
}}

#endif
