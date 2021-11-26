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

#include <ponder/class.hpp>

namespace ponder {

Class::Class(TypeId const& id, IdRef name)
    : m_sizeof(0)
    , m_id(id)
    , m_name(name)
    , m_destructor(nullptr)
    , m_userObjectCreator(nullptr)
{
}

IdReturn Class::name() const
{
    return m_name;
}

size_t Class::sizeOf() const
{
    return m_sizeof;
}

size_t Class::constructorCount() const
{
    return m_constructors.size();
}

const Constructor* Class::constructor(size_t index) const
{
    return m_constructors[index].get();
}

void Class::destruct(const UserObject &uobj, bool destruct) const
{
    m_destructor(uobj, destruct);
}

size_t Class::baseCount() const
{
    return m_bases.size();
}

const Class& Class::base(size_t index) const
{
    // Make sure that the index is not out of range
    if (index >= m_bases.size())
        PONDER_ERROR(OutOfRange(index, m_bases.size()));

    return *m_bases[index].base;
}

size_t Class::functionCount() const
{
    return m_functions.size();
}

bool Class::hasFunction(IdRef name) const
{
    return m_functions.containsKey(name);
}

const Function& Class::function(size_t index) const
{
    // Make sure that the index is not out of range
    if (index >= m_functions.size())
        PONDER_ERROR(OutOfRange(index, m_functions.size()));

    auto it = m_functions.begin();
    std::advance(it, index);

    return *it->second;
}

const Function& Class::function(IdRef name) const
{
    FunctionTable::const_iterator it;
    if (!m_functions.tryFind(name, it))
    {
        PONDER_ERROR(FunctionNotFound(name, this->name()));
    }

    return *it->second;
}

size_t Class::propertyCount() const
{
    return m_properties.size();
}

bool Class::hasProperty(IdRef name) const
{
    return m_properties.containsKey(name);
}

const Property& Class::property(size_t index) const
{
    // Make sure that the index is not out of range
    if (index >= m_properties.size())
        PONDER_ERROR(OutOfRange(index, m_properties.size()));

    auto it = m_properties.begin();
    std::advance(it, index);

    return *it->second;
}

const Property& Class::property(IdRef name) const
{
    PropertyTable::const_iterator it;
    if (!m_properties.tryFind(name, it))
    {
        PONDER_ERROR(PropertyNotFound(name, this->name()));
    }

    return *it->second;
}

void Class::visit(ClassVisitor& visitor) const
{
    // First visit properties
    for (PropertyTable::pair_t const& prop : m_properties)
    {
        prop.value()->accept(visitor);
    }

    // Then visit functions
    for (FunctionTable::pair_t const& func : m_functions)
    {
        func.value()->accept(visitor);
    }
}

void* Class::applyOffset(void* pointer, const Class& target) const
{
    // Special case for null pointers: don't apply offset to leave them null
    if (!pointer)
        return pointer;

    // Check target as a base class of this
    int offset = baseOffset(target);
    if (offset != -1)
        return static_cast<char*>(pointer) + offset;

    // Check target as a derived class of this
    offset = target.baseOffset(*this);
    if (offset != -1)
        return static_cast<char*>(pointer) - offset;

    // No match found, target is not a base class nor a derived class of this
    PONDER_ERROR(ClassUnrelated(name(), target.name()));
}

bool Class::operator == (const Class& other) const
{
    return m_id == other.m_id;
}

bool Class::operator != (const Class& other) const
{
    return m_id != other.m_id;
}

int Class::baseOffset(const Class& base) const
{
    // Check self
    if (&base == this)
        return 0;

    // Search base in the base classes
    for (const auto& [b, offset] : m_bases)
    {
        if (const int baseOffset = b->baseOffset(base); baseOffset != -1)
            return baseOffset + offset;
    }

    return -1;
}

} // namespace ponder
