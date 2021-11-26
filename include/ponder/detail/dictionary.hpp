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
#ifndef PONDER_DICTIONARY_HPP
#define PONDER_DICTIONARY_HPP

#include <ponder/config.hpp>
#include <utility>
#include <vector>
#include <algorithm> // std::lower_bound

namespace ponder
{
namespace detail
{

template <typename T>
struct DictKeyCmp {
    bool operator () (T a, T b) const {return a < b;}
};

//
// Key-value pair dictionary.
//  - Stored as vector of pairs, more cache friendly.
//  - Sorted on keys. Once only insertion cost gives better access times.
//
template <typename KEY, typename KEY_REF, typename VALUE, class CMP = DictKeyCmp<KEY_REF>>
class Dictionary
{
public:

    struct pair_t : std::pair<KEY,VALUE>
    {
        pair_t() : std::pair<KEY,VALUE>() {}
        pair_t(KEY_REF k, const VALUE& v) : std::pair<KEY,VALUE>(KEY(k), v) {}
        [[nodiscard]] KEY_REF name() const { return std::pair<KEY,VALUE>::first; }
        [[nodiscard]] const VALUE& value() const { return std::pair<KEY,VALUE>::second; }
    };

private:

    struct KeyCmp {
        bool operator () (const pair_t& a, KEY_REF b) const noexcept {
            return CMP() (a.first, b);
        }
    };

    using container_t = std::vector<pair_t>;
    container_t m_contents;

public:

    using value_type = pair_t;
    using const_iterator = typename container_t::const_iterator;

    [[nodiscard]] const_iterator begin() const noexcept { return m_contents.cbegin(); }
    [[nodiscard]] const_iterator end() const noexcept { return m_contents.cend(); }

    [[nodiscard]] const_iterator findKey(KEY_REF key) const noexcept
    {
        // binary search for key
        auto it(std::lower_bound(begin(), end(), key, KeyCmp()));
        if (it != end() && CMP()(key, it->first)) // it > it-1, check ==
            it = end();
        return it;
    }

    [[nodiscard]] const_iterator findValue(const VALUE& value) const noexcept
    {
        for (auto&& it = begin(); it != end(); ++it)
        {
            if (it->second == value)
                return it;
        }
        return end();
    }

    [[nodiscard]] bool tryFind(KEY_REF key, const_iterator& returnValue) const noexcept
    {
        if (auto it = findKey(key); it != end())
        {
            returnValue = it;
            return true;
        }
        return false; // not found
    }

    [[nodiscard]] bool containsKey(KEY_REF key) const noexcept
    {
        return findKey(key) != end();
    }

    [[nodiscard]] bool containsValue(const VALUE& value) const noexcept
    {
        return findValue(value) != end();
    }

    [[nodiscard]] size_t size() const noexcept { return m_contents.size(); }

    void insert(KEY_REF key, const VALUE &value)
    {
        erase(key);
        auto it = std::lower_bound(begin(), end(), key, KeyCmp());
        m_contents.insert(it, pair_t(key, value));
    }

    void insert(const_iterator it)
    {
        insert(it->first, it->second);
    }

    void erase(KEY_REF key) noexcept
    {
        auto it = findKey(key);
        if (it != end())
        {
            m_contents.erase(it);
        }
    }

    [[nodiscard]] const_iterator at(size_t index) const
    {
        auto it = begin();
        std::advance(it, index);
        return it;
    }
};

} // detail
} // ponder

#endif // PONDER_DICTIONARY_HPP
