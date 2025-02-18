/****************************************************************************
**
** This file is part of the Ponder library.
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

#pragma once
#ifndef PONDER_ARCHIVE_RAPIDJSON_HPP
#define PONDER_ARCHIVE_RAPIDJSON_HPP

#include <ponder/class.hpp>
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <stack>

namespace ponder {
namespace archive {

/**
 * \brief Write to an archive that uses JSON format as storage.
 *
 * The [RapidJSON](http://rapidjson.org/) library is used for JSON parsing and formatting.
 */
template <typename ARCHIVE>
class RapidJsonArchiveWriter
{
    ARCHIVE& m_archive;

    enum class Type { object, array };
	std::stack<Type> m_stack;

public:

    struct JsonNode {};
    using Node = JsonNode*;

    RapidJsonArchiveWriter(ARCHIVE& archive) : m_archive(archive)
    {
        m_stack.push(Type::object);
    }

    Node beginChild(Node, const std::string& name)
    {
        if (m_stack.top() == Type::object)
            m_archive.Key(name);
        m_stack.push(Type::object);
        m_archive.StartObject();
        return Node();
    }

    void endChild(Node, Node)
    {
        m_stack.pop();
        m_archive.EndObject();
    }

    void setProperty(Node, const std::string& name, const Value& value)
    {
        if (m_stack.top() == Type::object)
            m_archive.Key(name);
        switch (value.kind())
        {
            case ValueKind::None:
                m_archive.Null();
                break;
            case ValueKind::Boolean:
                m_archive.Bool(value.to<bool>());
                break;
            case ValueKind::Integer:
                m_archive.Int(value.to<long>());
                break;
            case ValueKind::LongInteger:
                m_archive.Int64(value.to<long long>());
                break;
            case ValueKind::Real:
                m_archive.Double(value.to<double>());
                break;
            case ValueKind::String:
            case ValueKind::Enum:
            case ValueKind::Reference:
                m_archive.String(value.to<std::string>());
                break;
            default:
                break;
        }
    }

    Node beginArray(Node parent, const std::string& name)
    {
        if (m_stack.top() == Type::object)
            m_archive.Key(name);
        m_stack.push(Type::array);
        m_archive.StartArray();
        return parent;
    }

    void endArray(Node, Node)
    {
        m_archive.EndArray();
        m_stack.pop();
    }

    string_view getValue(Node node)
    {
        return string_view();
    }

    bool isValid(Node node)
    {
        return node != nullptr;
    }
};

/**
 * \brief Read from an archive that uses JSON format as storage.
 *
 * The [RapidJSON](http://rapidjson.org/) library is used for JSON parsing and formatting.
 */
class RapidJsonArchiveReader
{
    [[maybe_unused]] rapidjson::Document& m_archive;

public:
    //! An abstract node within the JSON archive.
    struct Node
    {
        const rapidjson::Value& m_value;
        Node(const rapidjson::Value& value) : m_value(value) {}
    };

    //! Facilitate iteration over JSON arrays.
    struct ArrayIterator
    {
        const rapidjson::Value& m_value;
        rapidjson::Value::ConstValueIterator m_iter;

        [[nodiscard]] bool isEnd() const { return m_iter == m_value.End(); }
        void next() { ++m_iter; }
        Node getItem() const { return *m_iter; }
    };

    RapidJsonArchiveReader(rapidjson::Document& archive) : m_archive(archive) {}

    Node findProperty(Node node, const std::string& name)
    {
        const rapidjson::Value& val{ node.m_value[name] };
        return val;
    }

    ArrayIterator createArrayIterator(Node node, const std::string&)
    {
        return ArrayIterator({ node.m_value, node.m_value.Begin() });
    }

    Value getValue(Node node)
    {
        switch (node.m_value.GetType())
        {
            case rapidjson::kFalseType:
                return false;
            case rapidjson::kTrueType:
                return true;
            case rapidjson::kStringType:
                return std::string_view{ node.m_value.GetString(), node.m_value.GetStringLength() };
            case rapidjson::kNumberType:
                if (node.m_value.IsInt64())
                    return node.m_value.GetInt64();
                else if (node.m_value.IsUint64())
                    return node.m_value.GetUint64();
                else if (node.m_value.IsUint())
                    return node.m_value.GetUint();
                else if (node.m_value.IsInt())
                    return node.m_value.GetInt();
                else
                    return node.m_value.GetDouble();
            case rapidjson::kNullType:
            case rapidjson::kObjectType:
            case rapidjson::kArrayType:
                break;
        }
        return {};
    }

    bool isValid(Node node)
    {
        return !node.m_value.IsNull();
    }
};

} // namespace archive
} // namespace ponder

#endif // PONDER_ARCHIVE_RAPIDJSON_HPP
