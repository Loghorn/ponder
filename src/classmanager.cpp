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

#include <ponder/detail/classmanager.hpp>
#include <ponder/class.hpp>

namespace ponder {
namespace detail {

ClassManager& ClassManager::instance()
{
    static ClassManager cm;
    return cm;
}

Class& ClassManager::addClass(TypeId const& id, IdRef name)
{
    // First make sure that the class doesn't already exist
    // Note, we check by id and name. Neither should be registered.
    if (classExists(id) || (!name.empty() && getByNameSafe(name) != nullptr))
    {
        PONDER_ERROR(ClassAlreadyCreated(name));
    }

    // Create the new class
    auto* newClass = new Class(id, name);

    // Insert it into the table
    m_classes.insert(std::make_pair(id, newClass));
    m_names.insert(std::make_pair(Id(name), newClass));

    // Notify observers
    notifyClassAdded(*newClass);

    // Done
    return *newClass;
}

void ClassManager::removeClass(TypeId const& id)
{
    if (!classExists(id))
    {
        PONDER_ERROR(ClassNotFound("?"));
    }

    const auto it = m_classes.find(id);
    const auto* classPtr = it->second;
    const auto itName = m_names.find(classPtr->m_name);

    // Notify observers
    notifyClassRemoved(*classPtr);

    m_names.erase(itName);
    delete classPtr;
    m_classes.erase(it);
}

size_t ClassManager::count() const
{
    return m_classes.size();
}

ClassManager::ClassView ClassManager::getClasses() const
{
    return {m_classes.begin(), m_classes.end()};
}

const Class* ClassManager::getByIdSafe(TypeId const& id) const
{
    const auto it = m_classes.find(id);
    return (it == m_classes.end()) ? nullptr : it->second;
}

const Class& ClassManager::getById(TypeId const& id) const
{
    const Class* cls = getByIdSafe(id);
    if (!cls)
        PONDER_ERROR(ClassNotFound("?"));

    return *cls;
}

const Class* ClassManager::getByNameSafe(IdRef name) const
{
    const auto it = std::find_if(m_names.begin(), m_names.end(),
                                 [=](const std::pair<Id, Class*>& a) { return a.first == name; } );
    return (it == m_names.end()) ? nullptr : it->second;
}

const Class& ClassManager::getByName(IdRef name) const
{
    const Class* cls{ getByNameSafe(name) };
    if (!cls)
        PONDER_ERROR(ClassNotFound(name));

    return *cls;
}

bool ClassManager::classExists(TypeId const& id) const
{
    return m_classes.find(id) != m_classes.end();
}

ClassManager::ClassManager() = default;

ClassManager::~ClassManager()
{
    // Notify observers
    for (const auto& [fst, classPtr] : m_classes)
    {
        notifyClassRemoved(*classPtr);
        delete classPtr;
    }
}

} // namespace ponder
} // namespace detail
