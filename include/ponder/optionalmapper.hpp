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
#ifndef PONDER_OPTIONALMAPPER_HPP
#define PONDER_OPTIONALMAPPER_HPP

#include <ponder/config.hpp>
#include <ponder/enum.hpp>
#include <ponder/enumobject.hpp>
#include <ponder/userobject.hpp>
#include <ponder/errors.hpp>
#include <ponder/detail/util.hpp>
#include <ponder/detail/valueref.hpp>
#include <ponder/uses/runtime.hpp>
#include <ponder/pondertype.hpp>
#include <ponder/valuemapper.hpp>

#include <optional>

namespace ponder_ext {
namespace detail {

    template <typename T>
    struct OptionalProperty : ponder::SimpleProperty
    {
        OptionalProperty()
            : SimpleProperty("v", ponder::ValueKind::None)
            //: SimpleProperty("v", ponder::mapType<T>())
        {
        }

        ponder::Value getValue(const ponder::UserObject &object) const override
        {
            auto *v = object.get<std::optional<T>*>();
            if (*v == std::nullopt)
            {
                return ponder::Value();
            }
            return ponder::Value(**v);
        }

        ponder::Value getValueForSerialization(const ponder::UserObject &object) const override
        {
            auto *v = object.get<std::optional<T>*>();
            if (*v == std::nullopt)
            {
                return ponder::Value(T());
            }
            return ponder::Value(**v);
        }

        void setValue(const ponder::UserObject &object, const ponder::Value &value) const override
        {
            auto *v = object.get<std::optional<T>*>();
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
                *v = value.to<T>();
                break;
            case ponder::ValueKind::User:
                *v = value.to<ponder::UserObject>().get<T>();
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
struct OptionalMapper {};

template <typename T>
struct OptionalMapper<std::optional<T>>
{
    static size_t propertyCount() { return 1; }

    static std::shared_ptr<ponder::Property> property(size_t index)
    {
        return std::make_shared<detail::OptionalProperty<T>>();
    }

    static size_t functionCount() { return 0; }

    static std::shared_ptr<ponder::Function> function(size_t) { return nullptr; }
};
}

namespace ponder { namespace detail {
    template<typename T> struct StaticTypeDecl<std::optional<T>> {
        static void registerFn() {
            ponder::Class::declare<std::optional<T>>().template external<ponder_ext::OptionalMapper>();
        }
        static TypeId id(bool checkRegister = true) {
            if (checkRegister) detail::ensureTypeRegistered(calcTypeId<std::optional<T>>(), registerFn);
            return calcTypeId<std::optional<T>>();
        }
        static const char* name(bool checkRegister = true) {
            if (checkRegister) detail::ensureTypeRegistered(calcTypeId<std::optional<T>>(), registerFn);
            return typeid(std::optional<T>).name();
        }
        static constexpr bool defined = true, copyable = true;
    };
}}

#endif
