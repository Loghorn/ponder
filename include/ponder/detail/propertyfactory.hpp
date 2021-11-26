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
#ifndef PONDER_DETAIL_PROPERTYFACTORY_HPP
#define PONDER_DETAIL_PROPERTYFACTORY_HPP

#include <ponder/detail/simplepropertyimpl.hpp>
#include <ponder/detail/arraypropertyimpl.hpp>
#include <ponder/detail/enumpropertyimpl.hpp>
#include <ponder/detail/userpropertyimpl.hpp>
#include <ponder/detail/functiontraits.hpp>
#include <ponder/detail/typeid.hpp>

namespace ponder {
namespace detail {

// Bind to value.
template <class C, typename PropTraits>
class ValueBinder
{
public:
    using ClassType = C;
    using AccessType = std::conditional_t<PropTraits::isWritable,
                                        typename PropTraits::AccessType&, typename PropTraits::AccessType>;
    using SetType = std::remove_reference_t<AccessType>;

    using Binding = typename PropTraits::template Binding<ClassType, AccessType>;

    static_assert(!std::is_pointer_v<AccessType>, "Error: Pointers not handled here");

    ValueBinder(const Binding& b) : m_bound(b) {}

    AccessType getter(ClassType& c) const { return m_bound.access(c); }

    Value getValue(ClassType& c) const {
        if constexpr (PropTraits::isWritable)
            return UserObject::makeRef(getter(c));
        else
            return UserObject::makeCopy(getter(c));
    }

    bool setter(ClassType& c, SetType v) const {
        if constexpr (PropTraits::isWritable)
            return this->m_bound.access(c) = v, true;
        else
            return false;
    }

    bool setter(ClassType& c, Value const& value) const {
        return setter(c, value.to<SetType>());
    }

protected:
    Binding m_bound;
};

template <class C, typename PropTraits>
class ValueBinder2 : public ValueBinder<C, PropTraits>
{
    using Base = ValueBinder<C, PropTraits>;
public:
    template <typename S>
    ValueBinder2(const typename Base::Binding& g, S s) : Base(g), m_set(s) {}

    bool setter(typename Base::ClassType& c, typename Base::SetType v) const {
        return m_set(c, v), true;
    }

    bool setter(typename Base::ClassType& c, Value const& value) const {
        return setter(c, value.to<typename Base::SetType>());
    }

protected:
    std::function<void(typename Base::ClassType&, typename Base::AccessType)> m_set;
};


// Bind to internal reference getter.
template <class C, typename PropTraits>
class InternalRefBinder
{
public:
    using ClassType = C;
    using AccessType = typename PropTraits::ExposedType;

    using Binding = typename PropTraits::template Binding<ClassType, AccessType>;

    static_assert(std::is_pointer_v<AccessType>, "Error: Only pointers handled here");

    InternalRefBinder(const Binding& b) : m_bound(b) {}

    AccessType getter(ClassType& c) const {
        if constexpr (std::is_const_v<AccessType>)
            return m_bound.access(c);
        else
            return m_bound.access(const_cast<std::remove_const_t<ClassType>&>(c));
    }

    Value getValue(ClassType& c) const {return UserObject::makeRef(getter(c));}

    bool setter(ClassType&, AccessType) const {return false;}
    bool setter(ClassType&, Value const&) const {return false;}

protected:
    Binding m_bound;
};

// Internal reference getter & setter.
template <class C, typename PropTraits>
class InternalRefBinder2 : public InternalRefBinder<C, PropTraits>
{
    using Base = InternalRefBinder<C, PropTraits>;
public:
    template <typename S>
    InternalRefBinder2(const typename Base::Binding& g, S s) : Base(g), m_set(s) {}

    bool setter(typename Base::ClassType& c, typename Base::AccessType v) const {
        return m_set(c, v), true;
    }
    bool setter(typename Base::ClassType& c, Value const& value) const{
        return setter(c, value.to<typename Base::AccessType>());
    }

protected:
    std::function<void(typename Base::ClassType&, typename Base::AccessType)> m_set;
};

/*
 *  Access traits for an exposed type T.
 *    - I.e. how we use an instance to access the bound property data using the correct interface.
 *  Traits:
 *    - ValueBinder & RefBinder : RO (const) or RW data.
 *    - Impl : which specialise property impl to use.
 */
template <typename PT, typename E = void>
struct AccessTraits
{
    static constexpr PropertyAccessKind kind = PropertyAccessKind::Simple;

    template <class C>
    using ValueBinder = ValueBinder<C, PT>;

    template <class C>
    using ValueBinder2 = ValueBinder2<C, PT>;

    template <typename A>
    using Impl = SimplePropertyImpl<A>;
};

/*
 * Enums.
 */
template <typename PT>
struct AccessTraits<PT,
                    std::enable_if_t<std::is_enum_v<typename PT::ExposedTraits::DereferencedType>>>
{
    static constexpr PropertyAccessKind kind = PropertyAccessKind::Enum;

    template <class C>
    using ValueBinder = ValueBinder<C, PT>;

    template <class C>
    using ValueBinder2 = ValueBinder2<C, PT>;

    template <typename A>
    using Impl = EnumPropertyImpl<A>;
};

/*
 * Array types.
 */
template <typename PT>
struct AccessTraits<PT,
                    std::enable_if_t<ponder_ext::ArrayMapper<typename PT::ExposedTraits::DereferencedType>::isArray>>
{
    static constexpr PropertyAccessKind kind = PropertyAccessKind::Container;

    using ArrayTraits = ponder_ext::ArrayMapper<typename PT::ExposedTraits::DereferencedType>;

    template <class C>
    class ValueBinder : public ArrayTraits
    {
    public:
        using ArrayType = typename PT::ExposedTraits::DereferencedType;
        using ClassType = C;
        using AccessType = typename PT::AccessType&;

        using Binding = typename PT::template Binding<ClassType, AccessType>;

        ValueBinder(const Binding& a) : m_bound(a) {}
        
        AccessType getter(ClassType& c) const {return m_bound.access(c);}

        bool setter(ClassType& c, AccessType v) const {
            return this->m_bound.access(c) = v, true;
        }
    protected:
        Binding m_bound;
    };

    template <typename A>
    using Impl = ArrayPropertyImpl<A>;
};

/*
 * User objects.
 *  - I.e. Registered classes.
 *  - Enums also use registration so must differentiate.
 *  - Pointer to basic types, such as std::string, integers and floats
 *    also use registration. They must differentiate also.
 */
template <typename PT>
struct AccessTraits<PT,
                    std::enable_if_t<hasStaticTypeDecl<typename PT::ExposedTraits::DereferencedType>()
                        && !std::is_same_v<typename PT::ExposedTraits::DereferencedType, std::string>
                        && !std::is_integral_v<typename PT::ExposedTraits::DereferencedType>
                        && !std::is_floating_point_v<typename PT::ExposedTraits::DereferencedType>
                        && !std::is_enum_v<typename PT::ExposedTraits::DereferencedType>>>
{
    static constexpr PropertyAccessKind kind = PropertyAccessKind::User;

    template <class C>
    using ValueBinder = std::conditional_t<
        std::is_pointer_v<typename PT::ExposedType>, InternalRefBinder<C, PT>, ValueBinder<C, PT>
    >;

    template <class C>
    using ValueBinder2 = std::conditional_t<
        std::is_pointer_v<typename PT::ExposedType>, InternalRefBinder2<C, PT>, ValueBinder2<C, PT>
    >;

    template <typename A>
    using Impl = UserPropertyImpl<A>;
};


// Read-only accessor wrapper. Not RW, not a pointer.
template <class C, typename TRAITS>
class GetSet1
{
public:
    using PropTraits = TRAITS;
    using ClassType = C;
    using ExposedType = typename PropTraits::ExposedType;
    using TypeTraits = typename PropTraits::ExposedTraits;
    using DataType = typename PropTraits::DataType; // raw type or container
    static constexpr bool canRead = true;
    static constexpr bool canWrite = PropTraits::isWritable;

    using Access = AccessTraits<PropTraits>;

    using InterfaceType = typename Access::template ValueBinder<ClassType>;

    InterfaceType m_interface;

    GetSet1(typename PropTraits::BoundType getter)
        : m_interface(typename InterfaceType::Binding(getter))
    {}
};

/*
 * Property accessor composed of 1 getter and 1 setter
 */
template <typename C, typename FUNCTRAITS>
class GetSet2
{
public:

    using PropTraits = FUNCTRAITS;
    using ClassType = C;
    using ExposedType = typename PropTraits::ExposedType;
    using TypeTraits = typename PropTraits::ExposedTraits;
    using DataType = typename PropTraits::DataType; // raw type
    static constexpr bool canRead = true;
    static constexpr bool canWrite = true;

    using Access = AccessTraits<PropTraits>;

    using InterfaceType = typename Access::template ValueBinder2<ClassType>;

    InterfaceType m_interface;

    template <typename F1, typename F2>
    GetSet2(F1 getter, F2 setter)
        : m_interface(typename InterfaceType::Binding(getter), setter)
    {}
};

/*
 * Property factory which instantiates the proper type of property from 1 accessor.
 */
template <typename C, typename T, typename E = void>
struct PropertyFactory1
{
    static constexpr PropertyKind kind = PropertyKind::Function;

    static Property* create(IdRef name, T accessor)
    {
        using Accessor = GetSet1<C, FunctionTraits<T>>; // read-only?

        using PropertyImpl = typename Accessor::Access::template Impl<Accessor>;
        
        return new PropertyImpl(name, Accessor(accessor));
    }
};

template <typename C, typename T>
struct PropertyFactory1<C, T, std::enable_if_t<std::is_member_object_pointer_v<T>>>
{
    static constexpr PropertyKind kind = PropertyKind::MemberObject;

    static Property* create(IdRef name, T accessor)
    {
        using Accessor = GetSet1<C, MemberTraits<T>>; // read-only?

        using PropertyImpl = typename Accessor::Access::template Impl<Accessor>;

        return new PropertyImpl(name, Accessor(accessor));
    }
};

/*
 * Expose property with a getter and setter function.
 * Type of property is the return type of the getter.
 */
template <typename C, typename F1, typename F2, typename E = void>
struct PropertyFactory2
{
    static Property* create(IdRef name, F1 accessor1, F2 accessor2)
    {
        using Accessor = GetSet2<C, FunctionTraits<F1>>; // read-write wrapper

        using PropertyImpl = typename Accessor::Access::template Impl<Accessor>;

        return new PropertyImpl(name, Accessor(accessor1, accessor2));
    }
};

} // namespace detail
} // namespace ponder

#endif // PONDER_DETAIL_PROPERTYFACTORY_HPP
