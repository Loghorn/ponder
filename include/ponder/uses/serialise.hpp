/****************************************************************************
**
** This file is part of the Ponder library.
**
** The MIT License (MIT)
**
** Copyright (C) 2015-2020 Nick Trout.
** Copyright (C) 2009-2014 TEGESO/TEGESOFT and/or its subsidiary(-ies) and mother company.
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
#ifndef PONDER_USES_SERIALISE_HPP
#define PONDER_USES_SERIALISE_HPP

namespace ponder {
namespace archive {
    
/**
 For writing archive requires the following concepts:
 
    class Archive
    {
    public:
        NodeType beginChild(NodeType parent, const std::string& name);
        NodeType endChild(NodeType parent, NodeType child);
        NodeType beginArray(NodeType parent, const std::string& name);
        NodeType endArray(NodeType parent, NodeType arrayNode);
        void setProperty(NodeType node, const std::string& name, const std::string& value);
    };
 
 */
template <class ARCHIVE>
class ArchiveWriter
{
public:
    
    using ArchiveType = ARCHIVE;
    using NodeType = typename ArchiveType::Node;
    
    ArchiveWriter(ArchiveType& archive)
    :   m_archive(archive)
    {}
    
    void write(NodeType parent, const UserObject& object);
    
private:
    
    ArchiveType& m_archive;
};

/**
 For reading an archive requires the following concepts:
 
 class Archive
 {
 public:
     NodeType findProperty(NodeType node, const std::string& name);
     ArrayIterator createArrayIterator(NodeType node, const std::string& name);
     std::string getValue(NodeType node);
     bool isValid(NodeType node);
 };
 
 */
template <class ARCHIVE>
class ArchiveReader
{
public:
    
    using ArchiveType = ARCHIVE;
    using NodeType = typename ArchiveType::Node;
    using ArrayIterator = typename ArchiveType::ArrayIterator;

    ArchiveReader(ArchiveType& archive)
    :   m_archive(archive)
    {}
    
    void read(NodeType node, const UserObject& object);
    
private:
    
    ArchiveType& m_archive;
};

} // namespace archive
} // namespace ponder

#include "serialise.inl"

#endif // PONDER_USES_SERIALISE_HPP
