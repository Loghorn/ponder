
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

// Test object serialisation.

#include "test.hpp"
#include <ponder/uses/archive/rapidxml.hpp>
#include <ponder/uses/archive/rapidjson.hpp>
#include <ponder/uses/serialise.hpp>
#include <ponder/classbuilder.hpp>

#include <rapidxml/rapidxml_print.hpp>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <iostream>
#include <sstream>

namespace SerialiseTest
{
    class Simple
    {
    public:
        Simple() : m_i(0), m_f(0.f), m_b(true), m_ll(9999999999LL) {}

        Simple(int i, std::string s, float f, bool b)
        : m_i(i)
        , m_s(std::move(s))
        , m_f(f)
        , m_b(b)
        , m_ll(9999999999LL)
        {}

        [[nodiscard]] float getF() const { return m_f; }
        void setF(float f) { m_f = f; }

        int m_i;
        std::string m_s;
        std::vector<int> m_v;
        bool m_b;
        long long m_ll;

    private:
        float m_f;
    };

    class Ref
    {
    public:
        Ref() : m_ref(nullptr) {}
        ~Ref() = default;

        Simple m_instance;
        Simple *m_ref;
    };

    struct Complex
    {
        std::vector<Simple> m_v;
    };

    struct SuperComplex
    {
        std::vector<Complex> m_v;
    };

    template <typename T>
    struct Param
    {
        T value;
    };

    using Param_i = Param<int>;
    using Param_d = Param<double>;

    enum class ParamType
    {
        i,
        d,
        a,
    };

    struct Params
    {
        ParamType type;
        Param_i value_i;
        Param_d value_d;
        std::vector<int> value_a;
    };

    struct TestA
    {
        std::string name;
        std::vector<Params> params;
    };

    static void declare()
    {
        ponder::Class::declare<Simple>()
            .constructor()
            .property("int", &Simple::m_i)
            .property("float", &Simple::getF, &Simple::setF)
            .property("string", &Simple::m_s)
            .property("vector", &Simple::m_v)
            .property("bool", &Simple::m_b)
            .property("longlong", &Simple::m_ll)
            ;

        ponder::Class::declare<Ref>()
            .property("instance", &Ref::m_instance)
//            .property("ref", &Ref::m_ref)
            ;

        ponder::Class::declare<Complex>()
            .property("vect", &Complex::m_v)
            ;

        ponder::Class::declare<SuperComplex>()
            .property("complex_vector", &SuperComplex::m_v)
            ;

        ponder::Class::declare<Param_i>()
            .property("value", &Param_i::value)
            ;

        ponder::Class::declare<Param_d>()
            .property("value", &Param_d::value)
            ;

        ponder::Enum::declare<ParamType>()
            .value("i",  ParamType::i)
            .value("d",  ParamType::d)
            .value("a",  ParamType::a)
            ;

        ponder::Class::declare<Params>()
            .property("type", &Params::type)
            .property("i", &Params::value_i)
            .property("d", &Params::value_d)
            .property("a", &Params::value_a)
            ;

        ponder::Class::declare<TestA>()
            .property("name", &TestA::name)
            .property("params", &TestA::params)
            ;
    }
}

PONDER_AUTO_TYPE(SerialiseTest::Simple, &SerialiseTest::declare)
PONDER_AUTO_TYPE(SerialiseTest::Ref, &SerialiseTest::declare)
PONDER_AUTO_TYPE(SerialiseTest::Complex, &SerialiseTest::declare)
PONDER_AUTO_TYPE(SerialiseTest::SuperComplex, &SerialiseTest::declare)
PONDER_AUTO_TYPE(SerialiseTest::Param_i, &SerialiseTest::declare)
PONDER_AUTO_TYPE(SerialiseTest::Param_d, &SerialiseTest::declare)
PONDER_AUTO_TYPE(SerialiseTest::ParamType, &SerialiseTest::declare)
PONDER_AUTO_TYPE(SerialiseTest::Params, &SerialiseTest::declare)
PONDER_AUTO_TYPE(SerialiseTest::TestA, &SerialiseTest::declare)

using namespace SerialiseTest;

//-----------------------------------------------------------------------------
//                         Tests for serialisation
//-----------------------------------------------------------------------------

TEST_CASE("Can serialise using RapidXML")
{
    SECTION("Member values")
    {
        std::string storage;

        {
            std::unique_ptr<Simple> s = std::make_unique<Simple>(78, std::string("yadda"), 99.25f, true);
            REQUIRE(s != nullptr);
            s->m_v = {3,6,9};

            rapidxml::xml_document<> doc;
            auto rootNode = doc.allocate_node(rapidxml::node_element, "simple");
            REQUIRE(rootNode != nullptr);
            doc.append_node(rootNode);

            ponder::archive::RapidXmlArchive<> archive;
            ponder::archive::ArchiveWriter writer(archive);
            writer.write(rootNode, ponder::UserObject::makeRef(*s));

            std::cout << doc;

            std::ostringstream ostrm;
            ostrm << doc;
            storage = ostrm.str();
            doc.clear();
        }

        {
            std::unique_ptr<Simple> s2 = std::make_unique<Simple>(0, "", 0.f, true);
            REQUIRE(s2 != nullptr);

            rapidxml::xml_document<> doc;
            doc.parse<rapidxml::parse_non_destructive>(storage.data());
            auto rootNode = doc.first_node();
            REQUIRE(rootNode != nullptr);

            ponder::archive::RapidXmlArchive<> archive;
            ponder::archive::ArchiveReader reader(archive);
            reader.read(rootNode, ponder::UserObject::makeRef(*s2));

            CHECK(s2->m_i == 78);
            CHECK(s2->getF() == 99.25f);
            CHECK(s2->m_s == std::string("yadda"));
            CHECK(s2->m_v == std::vector<int>({3,6,9}));
        }
    }

    // Simple nested object
    SECTION("Nested object")
    {
        std::string storage;

        {
            std::unique_ptr<Ref> r = std::make_unique<Ref>();
            REQUIRE(r != nullptr);

            r->m_instance.m_i = 89;
            r->m_instance.setF(0.75f);
            r->m_instance.m_s = "stringy";

            rapidxml::xml_document<> doc;
            auto rootNode = doc.allocate_node(rapidxml::node_element, "ref");
            REQUIRE(rootNode != nullptr);
            doc.append_node(rootNode);

            ponder::archive::RapidXmlArchive<> archive;
            ponder::archive::ArchiveWriter writer(archive);
            writer.write(rootNode, ponder::UserObject::makeRef(*r));

            std::cout << doc;

            std::ostringstream ostrm;
            ostrm << doc;
            storage = ostrm.str();
            doc.clear();
        }

        {
            std::unique_ptr<Ref> r = std::make_unique<Ref>();
            REQUIRE(r != nullptr);

            {
                auto& metacls = ponder::classByType<Ref>();
                auto& inst = metacls.property("instance");
                CHECK(inst.isReadable());
                CHECK(inst.isWritable());
            }

            rapidxml::xml_document<> doc;
            doc.parse<rapidxml::parse_non_destructive>(storage.data());
            auto rootNode = doc.first_node();
            REQUIRE(rootNode != nullptr);

            ponder::archive::RapidXmlArchive<> archive;
            ponder::archive::ArchiveReader reader(archive);
            reader.read(rootNode, ponder::UserObject::makeRef(*r));

            CHECK(r->m_instance.m_i == 89);
            CHECK(r->m_instance.getF() == 0.75f);
            CHECK(r->m_instance.m_s == std::string("stringy"));
        }
    }

    SECTION("Complex values")
    {
        std::string storage;

        {
            Complex c;
            c.m_v.emplace_back(78, std::string("yadda"), 99.25f, true);
            c.m_v.emplace_back(11, std::string("wooby"), 66.75f, false);

            rapidxml::xml_document<> doc;
            auto rootNode = doc.allocate_node(rapidxml::node_element, "complex");
            REQUIRE(rootNode != nullptr);
            doc.append_node(rootNode);

            ponder::archive::RapidXmlArchive<> archive;
            ponder::archive::ArchiveWriter writer(archive);
            writer.write(rootNode, ponder::UserObject::makeRef(c));

            std::cout << doc;

            std::ostringstream ostrm;
            ostrm << doc;
            storage = ostrm.str();
            doc.clear();
        }

        {
            Complex c2;

            rapidxml::xml_document<> doc;
            doc.parse<rapidxml::parse_non_destructive>(storage.data());
            auto rootNode = doc.first_node();
            REQUIRE(rootNode != nullptr);

            ponder::archive::RapidXmlArchive<> archive;
            ponder::archive::ArchiveReader reader(archive);
            reader.read(rootNode, ponder::UserObject::makeRef(c2));

            CHECK(c2.m_v.size() == 2);
        }
    }

    SECTION("SuperComplex values")
    {
        std::string storage;

        {
            SuperComplex sc;

            Complex c;
            c.m_v.emplace_back(78, std::string("yadda"), 99.25f, true);
            c.m_v.emplace_back(11, std::string("wooby"), 66.75f, false);
            c.m_v[0].m_v = {1,2,3};
            c.m_v[1].m_v = {4,5,6,7,8};

            sc.m_v.push_back(c);

            rapidxml::xml_document<> doc;
            auto rootNode = doc.allocate_node(rapidxml::node_element, "superComplex");
            REQUIRE(rootNode != nullptr);
            doc.append_node(rootNode);

            ponder::archive::RapidXmlArchive<> archive;
            ponder::archive::ArchiveWriter writer(archive);
            writer.write(rootNode, ponder::UserObject::makeRef(sc));

            std::cout << doc;

            std::ostringstream ostrm;
            ostrm << doc;
            storage = ostrm.str();
            doc.clear();
        }

        {
            SuperComplex sc2;

            rapidxml::xml_document<> doc;
            doc.parse<rapidxml::parse_non_destructive>(storage.data());
            auto rootNode = doc.first_node();
            REQUIRE(rootNode != nullptr);

            ponder::archive::RapidXmlArchive<> archive;
            ponder::archive::ArchiveReader reader(archive);
            reader.read(rootNode, ponder::UserObject::makeRef(sc2));

            CHECK(sc2.m_v.size() == 1);
        }
    }

    SECTION("TestA")
    {
        std::string storage;

        {
            TestA testA{ "testA" };
            testA.params.emplace_back(Params{ParamType::d, 0, 2.3});
            testA.params.emplace_back(Params{ParamType::i, 10, 0});
            testA.params.emplace_back(Params{ParamType::a, 0, 0, {1,2,3}});

            rapidxml::xml_document<> doc;
            auto rootNode = doc.allocate_node(rapidxml::node_element, "testA");
            REQUIRE(rootNode != nullptr);
            doc.append_node(rootNode);

            ponder::archive::RapidXmlArchive<> archive;
            ponder::archive::ArchiveWriter writer(archive);
            writer.write(rootNode, ponder::UserObject::makeRef(testA));

            std::cout << doc;

            std::ostringstream ostrm;
            ostrm << doc;
            storage = ostrm.str();
            doc.clear();
        }

        {
            TestA testA;

            rapidxml::xml_document<> doc;
            doc.parse<rapidxml::parse_non_destructive>(storage.data());
            auto rootNode = doc.first_node();
            REQUIRE(rootNode != nullptr);

            ponder::archive::RapidXmlArchive<> archive;
            ponder::archive::ArchiveReader reader(archive);
            reader.read(rootNode, ponder::UserObject::makeRef(testA));

            CHECK(testA.params.size() == 3);
            CHECK(testA.params[0].type == ParamType::d);
            CHECK(testA.params[1].type == ParamType::i);
            CHECK(testA.params[2].type == ParamType::a);
        }
    }
}


TEST_CASE("Can serialise using RapidJSON")
{
    SECTION("Member values")
    {
        std::string storage;

        {
            std::unique_ptr<Simple> s = std::make_unique<Simple>(78, std::string("yadda"), 99.25f, true);
            REQUIRE(s != nullptr);
            s->m_v = { 3,6,9 };

            rapidjson::StringBuffer sb;
            rapidjson::Writer jwriter(sb);
            jwriter.StartObject();

            using Archive = ponder::archive::RapidJsonArchiveWriter<rapidjson::Writer<rapidjson::StringBuffer>>;
            Archive archive(jwriter);
            Archive::Node rootNode{};
            ponder::archive::ArchiveWriter writer(archive);
            writer.write(rootNode, ponder::UserObject::makeRef(*s));

            jwriter.EndObject();

            std::cout << sb.GetString() << std::endl;

            storage = sb.GetString();
        }

        {
            std::unique_ptr<Simple> s2 = std::make_unique<Simple>(0, "", 0.f, true);
            REQUIRE(s2 != nullptr);

            rapidjson::Document jdoc;
            REQUIRE(!jdoc.Parse(storage.data()).HasParseError());
            REQUIRE(jdoc.IsObject());

            using Archive = ponder::archive::RapidJsonArchiveReader;
            Archive archive(jdoc);
            Archive::Node rootNode{ jdoc };
            REQUIRE(archive.isValid(rootNode));

            ponder::archive::ArchiveReader reader(archive);
            reader.read(rootNode, ponder::UserObject::makeRef(*s2));

            CHECK(s2->m_i == 78);
            CHECK(s2->getF() == 99.25f);
            CHECK(s2->m_s == std::string("yadda"));
            CHECK(s2->m_v == std::vector<int>({ 3,6,9 }));
        }
    }

    // Simple nested object
    SECTION("Nested object")
    {
        std::string storage;

        {
            std::unique_ptr<Ref> r = std::make_unique<Ref>();
            REQUIRE(r != nullptr);

            r->m_instance.m_i = 89;
            r->m_instance.setF(0.75f);
            r->m_instance.m_s = "stringy";

            rapidjson::StringBuffer sb;
            rapidjson::Writer jwriter(sb);
            jwriter.StartObject();

            using Archive = ponder::archive::RapidJsonArchiveWriter<rapidjson::Writer<rapidjson::StringBuffer>>;
            Archive archive(jwriter);
            Archive::Node rootNode{};
            ponder::archive::ArchiveWriter writer(archive);
            writer.write(rootNode, ponder::UserObject::makeRef(*r));

            jwriter.EndObject();

            std::cout << sb.GetString() << std::endl;

            storage = sb.GetString();
        }

        {
            std::unique_ptr<Ref> r = std::make_unique<Ref>();
            REQUIRE(r != nullptr);

            {
                auto& metacls = ponder::classByType<Ref>();
                auto& inst = metacls.property("instance");
                CHECK(inst.isReadable());
                CHECK(inst.isWritable());
            }

            rapidjson::Document jdoc;
            REQUIRE(!jdoc.Parse(storage.data()).HasParseError());

            using Archive = ponder::archive::RapidJsonArchiveReader;
            Archive archive(jdoc);
            Archive::Node rootNode{ jdoc };
            REQUIRE(archive.isValid(rootNode));

            ponder::archive::ArchiveReader reader(archive);
            reader.read(rootNode, ponder::UserObject::makeRef(*r));

            CHECK(r->m_instance.m_i == 89);
            CHECK(r->m_instance.getF() == 0.75f);
            CHECK(r->m_instance.m_s == std::string("stringy"));
        }
    }

    SECTION("Complex values")
    {
        std::string storage;

        {
            Complex c;
            c.m_v.emplace_back(78, std::string("yadda"), 99.25f, true);
            c.m_v.emplace_back(11, std::string("wooby"), 66.75f, false);
            c.m_v[0].m_v = {1,2,3};
            c.m_v[1].m_v = {4,5,6,7,8};

            rapidjson::StringBuffer sb;
            rapidjson::Writer jwriter(sb);
            jwriter.StartObject();

            using Archive = ponder::archive::RapidJsonArchiveWriter<rapidjson::Writer<rapidjson::StringBuffer>>;
            Archive archive(jwriter);
            Archive::Node rootNode{};
            ponder::archive::ArchiveWriter writer(archive);
            writer.write(rootNode, ponder::UserObject::makeRef(c));

            jwriter.EndObject();

            std::cout << sb.GetString() << std::endl;

            storage = sb.GetString();
        }

        {
            Complex c2;

            rapidjson::Document jdoc;
            REQUIRE(!jdoc.Parse(storage.data()).HasParseError());

            using Archive = ponder::archive::RapidJsonArchiveReader;
            Archive archive(jdoc);
            Archive::Node rootNode{ jdoc };
            REQUIRE(archive.isValid(rootNode));

            ponder::archive::ArchiveReader reader(archive);
            reader.read(rootNode, ponder::UserObject::makeRef(c2));

            CHECK(c2.m_v.size() == 2);
        }
    }

    SECTION("SuperComplex values")
    {
        std::string storage;

        {
            SuperComplex sc;

            Complex c;
            c.m_v.emplace_back(78, std::string("yadda"), 99.25f, true);
            c.m_v.emplace_back(11, std::string("wooby"), 66.75f, false);
            c.m_v[0].m_v = {1,2,3};
            c.m_v[1].m_v = {4,5,6,7,8};

            sc.m_v.push_back(c);

            rapidjson::StringBuffer sb;
            rapidjson::Writer jwriter(sb);
            jwriter.StartObject();

            using Archive = ponder::archive::RapidJsonArchiveWriter<rapidjson::Writer<rapidjson::StringBuffer>>;
            Archive archive(jwriter);
            Archive::Node rootNode{};
            ponder::archive::ArchiveWriter writer(archive);
            writer.write(rootNode, ponder::UserObject::makeRef(sc));

            jwriter.EndObject();

            std::cout << sb.GetString() << std::endl;

            storage = sb.GetString();
        }

        {
            SuperComplex sc2;

            rapidjson::Document jdoc;
            REQUIRE(!jdoc.Parse(storage.data()).HasParseError());

            using Archive = ponder::archive::RapidJsonArchiveReader;
            Archive archive(jdoc);
            Archive::Node rootNode{ jdoc };
            REQUIRE(archive.isValid(rootNode));

            ponder::archive::ArchiveReader reader(archive);
            reader.read(rootNode, ponder::UserObject::makeRef(sc2));

            CHECK(sc2.m_v.size() == 1);
        }
    }

    SECTION("TestA")
    {
        std::string storage;

        {
            TestA testA{ "testA" };
            testA.params.emplace_back(Params{ParamType::d, 0, 2.3});
            testA.params.emplace_back(Params{ParamType::i, 10, 0});
            testA.params.emplace_back(Params{ParamType::a, 0, 0, {1,2,3}});

            rapidjson::StringBuffer sb;
            rapidjson::Writer jwriter(sb);
            jwriter.StartObject();

            using Archive = ponder::archive::RapidJsonArchiveWriter<rapidjson::Writer<rapidjson::StringBuffer>>;
            Archive archive(jwriter);
            Archive::Node rootNode{};
            ponder::archive::ArchiveWriter writer(archive);
            writer.write(rootNode, ponder::UserObject::makeRef(testA));

            jwriter.EndObject();

            std::cout << sb.GetString() << std::endl;

            storage = sb.GetString();
        }

        {
            TestA testA;

            rapidjson::Document jdoc;
            REQUIRE(!jdoc.Parse(storage.data()).HasParseError());

            using Archive = ponder::archive::RapidJsonArchiveReader;
            Archive archive(jdoc);
            Archive::Node rootNode{ jdoc };
            REQUIRE(archive.isValid(rootNode));

            ponder::archive::ArchiveReader reader(archive);
            reader.read(rootNode, ponder::UserObject::makeRef(testA));

            CHECK(testA.params.size() == 3);
            CHECK(testA.params[0].type == ParamType::d);
            CHECK(testA.params[1].type == ParamType::i);
            CHECK(testA.params[2].type == ParamType::a);
        }
    }
}
