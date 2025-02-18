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
#ifndef PONDER_DETAIL_ENUMPROPERTYIMPL_HPP
#define PONDER_DETAIL_ENUMPROPERTYIMPL_HPP

#include <ponder/enumproperty.hpp>

namespace ponder {
namespace detail {

/**
 * \brief Typed implementation of EnumProperty
 *
 * EnumPropertyImpl is a template implementation of EnumProperty, which is strongly typed
 * in order to keep track of the true underlying C++ types involved in the property.
 *
 * The template parameter A is an abstract helper to access the actual C++ property.
 *
 * \sa EnumProperty
 */
template <typename A>
class EnumPropertyImpl final : public EnumProperty
{
public:

    /**
     * \brief Construct the property from its accessors
     *
     * \param name Name of the property
     * \param accessor Object used to access the actual C++ property
     */
    EnumPropertyImpl(IdRef name, A&& accessor);

protected:

    /**
     * \see Property::isReadable
     */
    [[nodiscard]] bool isReadable() const;

    /**
     * \see Property::isWritable
     */
    [[nodiscard]] bool isWritable() const;

    /**
     * \see Property::getValue
     */
    [[nodiscard]] Value getValue(const UserObject& object) const;

    /**
     * \see Property::setValue
     */
    void setValue(const UserObject& object, const Value& value) const;

private:

    A m_accessor;
};

} // namespace detail
} // namespace ponder

#include <ponder/detail/enumpropertyimpl.inl>

#endif // PONDER_DETAIL_ENUMPROPERTYIMPL_HPP
