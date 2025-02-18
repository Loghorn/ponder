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

#include <ponder/classbuilder.hpp>
#include <ponder/enumproperty.hpp>
#include "test.hpp"

namespace EnumPropertyTest
{
    enum MyEnum
    {
        Zero = 0,
        One  = 1
    };

    struct MyClass
    {
        MyClass(MyEnum x_) : x(x_)
        {
        }

        MyEnum x;
    };

    void declare()
    {
        ponder::Enum::declare<MyEnum>("EnumPropertyTest::MyEnum");

        ponder::Class::declare<MyClass>("EnumPropertyTest::MyClass")
            .property("x", &MyClass::x);
    }
}

PONDER_AUTO_TYPE(EnumPropertyTest::MyEnum, &EnumPropertyTest::declare)
PONDER_AUTO_TYPE(EnumPropertyTest::MyClass, &EnumPropertyTest::declare)

using namespace EnumPropertyTest;


//-----------------------------------------------------------------------------
//                         Tests for ponder::EnumProperty
//-----------------------------------------------------------------------------

TEST_CASE("Enum properties")
{
    const ponder::Class& metaclass = ponder::classByType<MyClass>();

    const ponder::EnumProperty* property =
        &static_cast<const ponder::EnumProperty&>(metaclass.property("x"));

    SECTION("have enum type")
    {
        IS_TRUE(property->kind() == ponder::ValueKind::Enum);
    }

    SECTION("wrap an Enum")
    {
        IS_TRUE( property->getEnum() == ponder::enumByType<MyEnum>() );
    }

    SECTION("have values")
    {
        IS_TRUE( property->get(MyClass(Zero)) == ponder::Value(Zero) );
        IS_TRUE( property->get(MyClass(One)) == ponder::Value(One) );
    }

    SECTION("allow setting of values")
    {
        MyClass object(Zero);
        property->set(&object, One);

        IS_TRUE( property->get(object) == ponder::Value(One) );
    }
}

