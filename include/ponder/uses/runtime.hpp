/****************************************************************************
**
** This file is part of the Ponder library.
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

/**
 * \file
 * \brief Runtime uses for Ponder registered data.
 */

#pragma once
#ifndef PONDER_USES_RUNTIME_HPP
#define PONDER_USES_RUNTIME_HPP

#include <ponder/class.hpp>
#include <ponder/constructor.hpp>

/**
 * \namespace ponder::runtime
 * \brief Contains Ponder runtime support public API.
 */

namespace ponder {
namespace runtime {

inline void destroy(const UserObject &obj);

namespace detail {

template <typename... A>
struct ArgsBuilder
{
    static Args makeArgs(A&&... args)
    {
        return Args(std::forward<A>(args)...);
    }
};

template <>
struct ArgsBuilder<Args> {
    static Args makeArgs(const Args& args) { return args; }
    static Args makeArgs(Args&& args) { return std::move(args); }
};

template <>
struct ArgsBuilder<void> {
    static Args makeArgs(const Args&) { return Args::empty; }
};


struct UserObjectDeleter {
    void operator () (const UserObject *uo) const { destroy(*uo); }
};

} // namespace detail

/**
 * \brief This object is used to create instances of metaclasses
 *
 * There are helpers for this class, see ponder::runtime::construct() and
 * ponder::runtime::create().
 *
 * Example of use:
 * \code
 * runtime::ObjectFactory fact(classByType<MyClass>.function("fpp"));
 * fact.create("bar");
 * \endcode
 *
 */
class ObjectFactory
{
public:

    /**
     * \brief Constructor
     *
     * \param cls The Class to be called
     * \return a Class reference
     */
    ObjectFactory(const Class &cls) : m_class(cls) {}

    /**
     * \brief Get the class begin used
     *
     * \return a Class reference
     */
    [[nodiscard]] const Class& getClass() const { return m_class; }

    /**
     * \brief Construct a new instance of the C++ class bound to the metaclass
     *
     * If no constructor can match the provided arguments, UserObject::nothing
     * is returned. If a pointer is provided then placement new is used instead of
     * the new instance being dynamically allocated using new.
     * The new instance is wrapped into a UserObject.
     *
     * \note It must be destroyed with the appropriate destruction function:
     * Class::destroy for new and Class::destruct for placement new.
     *
     * \param args Arguments to pass to the constructor (empty by default)
     * \param ptr Optional pointer to the location to construct the object (placement new)
     * \return New instance wrapped into a UserObject, or UserObject::nothing if it failed
     * \sa create()
     */
    inline UserObject construct(const Args& args = Args::empty, void* ptr = nullptr) const;

    /**
     * \brief Create a new instance of the class bound to the metaclass
     *
     * Create an object without having to create an Args list. See notes for Class::construct().
     * If you need to create an argument list at runtime and use it to create an object then
     * see Class::construct().
     *
     * \param args An argument list.
     * \return New instance wrapped into a UserObject, or UserObject::nothing if it failed
     * \sa construct()
     */
    template <typename ...A>
    UserObject create(A... args) const;

    /**
     * \brief Destroy an instance of the C++ class bound to the metaclass
     *
     * This function must be called to destroy every instance created with
     * Class::construct.
     *
     * \param object Object to be destroyed
     *
     * \see construct
     */
    inline void destroy(const UserObject& object) const;

    /**
     * \brief Destruct an object created using placement new
     *
     * This function must be called to destroy every instance created with
     * Class::construct.
     *
     * \param object Object to be destroyed
     *
     * \see construct
     */
    inline void destruct(const UserObject& object) const;

private:

    const Class &m_class;
};


/**
 * \brief This object is used to invoke a object member function, or method
 *
 * There are helpers for this class, see ponder::runtime::call() and
 * ponder::runtime::callStatic().
 *
 */
class ObjectCaller
{
public:

    /**
     * \brief Constructor
     *
     * \param fn The Function to be called
     * \return a Function reference
     */
    inline ObjectCaller(const Function &fn);

    /**
     * \brief Get the function begin used
     *
     * \return a Function reference
     */
    [[nodiscard]] const Function& function() const { return m_func; }

    /**
     * \brief Call the function
     *
     * \param obj Object
     * \param args Arguments to pass to the function, for example "ponder::Args::empty"
     *
     * \return Value returned by the function call
     *
     * \code
     * runtime::ObjectCaller caller(classByType<MyClass>.function("foo"));
     * caller.call(instancem, "bar");
     * \endcode
     *
     * \throw ForbiddenCall the function is not callable
     * \throw NullObject object is invalid
     * \throw NotEnoughArguments too few arguments are provided
     * \throw BadArgument one of the arguments can't be converted to the requested type
     */
    template <typename... A>
    Value call(const UserObject &obj, A&&... vargs);

    inline Value call(const UserObject &obj, Args args);

private:

    const Function &m_func;
    const std::unique_ptr<detail::FunctionCaller>& m_caller;
};

/**
 * \brief This object is used to invoke a function
 *
 * There are helpers for this class, see ponder::runtime::call() and
 * ponder::runtime::callStatic().
 *
 */
class FunctionCaller
{
public:

    /**
     * \brief Constructor
     *
     * \param f The function to call
     */
    inline FunctionCaller(const Function &f);

    /**
     * \brief Get the function begin used
     *
     * \return a Function reference
     */
    [[nodiscard]] const Function& function() const { return m_func; }

    /**
     * \brief Call the static function
     *
     * \param args Arguments to pass to the function, for example "ponder::Args::empty"
     *
     * \return Value returned by the function call
     *
     * \code
     * runtime::FunctionCaller caller(classByType<MyClass>.function("fpp"));
     * caller.call(Args("bar"));
     * \endcode
     *
     * \throw NotEnoughArguments too few arguments are provided
     * \throw BadArgument one of the arguments can't be converted to the requested type
     */
    template <typename... A>
    Value call(A... vargs);

    inline Value call(const Args &args);

private:

    const Function &m_func;
    const std::unique_ptr<detail::FunctionCaller>& m_caller;
};

//--------------------------------------------------------------------------------------
// Helpers

/**
 * \brief Create instance of metaclass as a UserObject
 *
 * This is a helper function which uses ObjectFactory to create the instance.
 *
 * \param cls The metaclass to make an instance of
 * \param args The constructor arguments for the class instance
 * \return A UserObject which owns an instance of the metaclass.
 *
 * \snippet simple.cpp eg_simple_create
 *
 * \sa destroy()
 */
template <typename... A>
inline UserObject create(const Class &cls, A... args)
{
    return ObjectFactory(cls).create(args...);
}

using UniquePtr = std::unique_ptr<UserObject>;

template <typename... A>
inline UniquePtr createUnique(const Class &cls, A... args)
{
    return std::make_unique<UserObject>(create(cls, args...));
}

/**
 * \brief Destroy a UserObject instance
 *
 * This is a helper function which uses ObjectFactory to destroy the instance.
 *
 * \param obj Reference to UserObject instance to destroy
 *
 * \sa create()
 */
inline void destroy(const UserObject &obj)
{
    ObjectFactory(obj.getClass()).destroy(obj);
}

/**
 * \brief Call a member function
 *
 * This is a helper function which uses ObjectCaller to call the member function.
 *
 * \param fn The Function to call
 * \param obj Reference to UserObject instance to destroy
 * \param args Arguments for the function
 * \return The return value. This is NoType if function return type return is `void`.
 *
 * \sa callStatic(), Class::function()
 */
template <typename... A>
inline Value call(const Function &fn, const UserObject &obj, A&&... args)
{
    return ObjectCaller(fn).call(obj,
                                 detail::ArgsBuilder<A...>::makeArgs(std::forward<A>(args)...));
}

inline Value call(const Function &fn, const UserObject &obj, const Args &args)
{
    return ObjectCaller(fn).call(obj, args);
}

/**
 * \brief Call a non-member function
 *
 * This is a helper function which uses FunctionCaller to call the function.
 *
 * \param fn The Function to call
 * \param args Arguments for the function
 * \return The return value. This is NoType if function return type return is `void`.
 *
 * \sa call(), Class::function()
 */
template <typename... A>
inline Value callStatic(const Function &fn, A&&... args)
{
    return FunctionCaller(fn).call(detail::ArgsBuilder<A...>::makeArgs(std::forward<A>(args)...));
}

inline Value callStatic(const Function &fn, Args &args)
{
    return FunctionCaller(fn).call(args);
}

} // namespace runtime
} // namespace ponder

//--------------------------------------------------------------------------------------
// .inl

namespace ponder {
namespace runtime {

template <typename... A>
UserObject ObjectFactory::create(A... args) const
{
    const Args a(args...);
    return construct(a);
}

template <typename... A>
Value ObjectCaller::call(const UserObject &obj, A&&... vargs)
{
    if (obj.pointer() == nullptr)
        PONDER_ERROR(NullObject(&obj.getClass()));

    Args args(detail::ArgsBuilder<A...>::makeArgs(std::forward<A>(vargs)...));

    // Check the number of arguments
    if (args.count() < m_func.paramCount())
        PONDER_ERROR(NotEnoughArguments(m_func.name(), args.count(), m_func.paramCount()));

    args.insert(0, obj);

    return m_caller->execute(args);
}

Value ObjectCaller::call(const UserObject &obj, Args args)
{
    if (obj.pointer() == nullptr)
        PONDER_ERROR(NullObject(&obj.getClass()));

    // Check the number of arguments
    if (args.count() < m_func.paramCount())
        PONDER_ERROR(NotEnoughArguments(m_func.name(), args.count(), m_func.paramCount()));

    args.insert(0, obj);

    return m_caller->execute(args);
}

template <typename... A>
Value FunctionCaller::call(A... vargs)
{
    const Args args(detail::ArgsBuilder<A...>::makeArgs(vargs...));

    // Check the number of arguments
    if (args.count() < m_func.paramCount())
        PONDER_ERROR(NotEnoughArguments(m_func.name(), args.count(), m_func.paramCount()));

    return m_caller->execute(args);
}

Value FunctionCaller::call(const Args &args)
{
    // Check the number of arguments
    if (args.count() < m_func.paramCount())
        PONDER_ERROR(NotEnoughArguments(m_func.name(), args.count(), m_func.paramCount()));

    return m_caller->execute(args);
}

} // namespace runtime
} // namespace ponder

//--------------------------------------------------------------------------------------

namespace ponder {
namespace runtime {

UserObject ObjectFactory::construct(const Args& args, void* ptr) const
{
    // Search an arguments match among the list of available constructors
    for (size_t nb = m_class.constructorCount(), i = 0; i < nb; ++i)
    {
        if (const Constructor& constructor = *m_class.constructor(i); constructor.matches(args))
        {
            // Match found: use the constructor to create the new instance
            return constructor.create(ptr, args);
        }
    }

    return UserObject::nothing;  // no match found
}

void ObjectFactory::destroy(const UserObject& object) const
{
    m_class.destruct(object, false);

    const_cast<UserObject&>(object) = UserObject::nothing;
}

void ObjectFactory::destruct(const UserObject& object) const
{
    m_class.destruct(object, true);

    const_cast<UserObject&>(object) = UserObject::nothing;
}

ObjectCaller::ObjectCaller(const Function &f)
    :   m_func(f)
    ,   m_caller(std::get<uses::Uses::eRuntimeModule>(
                 *static_cast<const uses::Uses::PerFunctionUserData*>(m_func.getUsesData())))
{
}

FunctionCaller::FunctionCaller(const Function &f)
    :   m_func(f)
    ,   m_caller(std::get<uses::Uses::eRuntimeModule>(
                 *static_cast<const uses::Uses::PerFunctionUserData*>(m_func.getUsesData())))
{
}

} // runtime
} // ponder

#endif // PONDER_USES_RUNTIME_HPP
