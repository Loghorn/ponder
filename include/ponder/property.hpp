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
#ifndef PONDER_PROPERTY_HPP
#define PONDER_PROPERTY_HPP


#include <ponder/value.hpp>

namespace ponder
{

class ClassVisitor;

/**
 * \brief Abstract representation of a property
 *
 * Properties are members of metaclasses. Their main purpose is to be get and/or set;
 * They also provide detailed informations about their type.
 *
 * \sa SimpleProperty, ArrayProperty, EnumProperty, ObjectProperty
 */
class PONDER_API Property : public Type
{
public:
    Property(Property const&) = delete;
    Property& operator=(Property const&) = delete;

    /**
     * \brief Get the name of the property
     *
     * \return Name of the property
     */
    [[nodiscard]] IdReturn name() const;

    /**
     * \brief Get the type of the property
     *
     * \return Type of the property
     */
    [[nodiscard]] ValueKind kind() const;

    /**
     * \brief Check if the property can be read
     *
     * \return True if the property can be read, false otherwise
     */
    [[nodiscard]] virtual bool isReadable() const;

    /**
     * \brief Check if the property can be written
     *
     * \return True if the property can be written, false otherwise
     */
    [[nodiscard]] virtual bool isWritable() const;

    /**
     * \brief Get the current value of the property for a given object
     *
     * \param object Object
     *
     * \return Value of the property
     *
     * \throw NullObject object is invalid
     * \throw ForbiddenRead property is not readable
     */
    [[nodiscard]] Value get(const UserObject& object) const;

    /**
     * \brief Get the current value of the property for a given object. Used only for serialization
     *
     * \param object Object
     *
     * \return Value of the property
     *
     * \throw NullObject object is invalid
     * \throw ForbiddenRead property is not readable
     */
    [[nodiscard]] Value getForSerialization(const UserObject& object) const;

    /**
     * \brief Set the current value of the property for a given object
     *
     * \param object Object
     * \param value New value to assign to the property
     *
     * \throw NullObject \a object is invalid
     * \throw ForbiddenWrite property is not writable
     * \throw BadType \a value can't be converted to the property's type
     */
    void set(const UserObject& object, const Value& value) const;

    /**
     * \brief Accept the visitation of a ClassVisitor
     *
     * \param visitor Visitor to accept
     */
    virtual void accept(ClassVisitor& visitor) const;

protected:

    template <typename T> friend class ClassBuilder;
    friend class UserObject;

    /**
     * \brief Construct the property from its description
     *
     * \param name Name of the property
     * \param type Type of the property
     */
    Property(IdRef name, ValueKind type);

    /**
     * \brief Do the actual reading of the value
     *
     * This function is a pure virtual which has to be implemented in derived classes.
     *
     * \param object Object
     *
     * \return Value of the property
     */
    [[nodiscard]] virtual Value getValue(const UserObject& object) const = 0;

    /**
     * \brief Do the actual reading of the value
     *
     * This function is only used for serialization and by default calls \cref getValue.
     *
     * \param object Object
     *
     * \return Value of the property
     */
    [[nodiscard]] virtual Value getValueForSerialization(const UserObject& object) const { return getValue(object); }

    /**
     * \brief Do the actual writing of the value
     *
     * This function is a pure virtual which has to be implemented in derived classes.
     *
     * \param object Object
     * \param value New value to assign to the property
     */
    virtual void setValue(const UserObject& object, const Value& value) const = 0;

    [[nodiscard]] void* getRawData(const UserObject& object) const;
    template <typename T>
    [[nodiscard]] T* getData(const UserObject& object) const { return static_cast<T*>(getRawData(object)); }
    void setData(const UserObject& object, std::shared_ptr<void> data) const;

private:

    Id m_name; // Name of the property
    ValueKind m_type; // Type of the property
};

} // namespace ponder


#endif // PONDER_PROPERTY_HPP
