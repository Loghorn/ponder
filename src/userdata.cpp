/****************************************************************************
 **
 ** This file is part of the Ponder library, formerly CAMP.
 **
 ** The MIT License (MIT)
 **
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

#include <ponder/userdata.hpp>
#include <ponder/detail/dictionary.hpp>

namespace ponder {

class TypeUserDataStore final : public IUserDataStore
{
    using key_t = const Type*;
    using store_t = detail::Dictionary<Id, IdRef, Value>;
    using class_store_t = std::map<key_t, store_t>;
    class_store_t m_store;

public:

    void setValue(const Type& t, IdRef name, const Value& v);
    [[nodiscard]] const Value* getValue(const Type& t, IdRef name) const;
    void removeValue(const Type& t, IdRef name);
};

void TypeUserDataStore::setValue(const Type& t, IdRef name, const Value& v)
{
    auto it = m_store.find(&t);
    if (it == m_store.end())
    {
        const auto [fst, snd] = m_store.insert(class_store_t::value_type(&t, store_t()));
        it = fst;
    }
    it->second.insert(name, v);
}

const Value* TypeUserDataStore::getValue(const Type& t, IdRef name) const
{
    if (const auto it = m_store.find(&t); it != m_store.end())
    {
        const auto vit = it->second.findKey(name);
        if (vit != it->second.end())
            return &vit->second;
    }
    return nullptr;
}

void TypeUserDataStore::removeValue(const Type& t, IdRef name)
{
    if (const auto it = m_store.find(&t); it != m_store.end())
    {
        it->second.erase(name);
    }
}

std::unique_ptr<TypeUserDataStore> g_memberDataStore;

IUserDataStore* userDataStore()
{
    auto p = g_memberDataStore.get();

    if (p == nullptr)
    {
        g_memberDataStore = std::make_unique<TypeUserDataStore>();
        p = g_memberDataStore.get();
    }

    return p;
}

} // namespace ponder



