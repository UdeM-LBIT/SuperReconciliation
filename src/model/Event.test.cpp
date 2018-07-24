#include "Event.hpp"
#include "../io/nhx_parser.hpp"
#include <catch.hpp>

TEST_CASE("Read a leaf node from a tagged node")
{
    TaggedNode tagged_node{
        "a b c d e f", // name
    };

    Event event = tagged_node;

    REQUIRE(event.type == Event::Type::None);
    REQUIRE(event.synteny == Synteny{"a", "b", "c", "d", "e", "f"});
    REQUIRE(event.segment.first == 0);
    REQUIRE(event.segment.second == 6);
}

TEST_CASE("Read a full loss event node from a tagged node")
{
    TaggedNode tagged_node{
        "     ", // name
    };

    Event event = tagged_node;

    REQUIRE(event.type == Event::Type::Loss);
    REQUIRE(event.synteny == Synteny{});
    REQUIRE(event.segment.first == 0);
    REQUIRE(event.segment.second == 0);
}

TEST_CASE("Read a full loss event node with extra info from a tagged node")
{
    TaggedNode tagged_node{
        "     ", // name
        0.,
        {
            {"event", "none"},
            {"segment", "12-24"},
        },
    };

    Event event = tagged_node;

    REQUIRE(event.type == Event::Type::Loss);
    REQUIRE(event.synteny == Synteny{});
    REQUIRE(event.segment.first == 0);
    REQUIRE(event.segment.second == 0);
}

TEST_CASE("Read a segmental loss event node from a tagged node")
{
    TaggedNode tagged_node{
        "a b c d e f", // name
        0.,
        {
            {"event", "loss"},
            {"segment", "1-4"},
        },
    };

    Event event = tagged_node;

    REQUIRE(event.type == Event::Type::Loss);
    REQUIRE(event.synteny == Synteny{"a", "b", "c", "d", "e", "f"});
    REQUIRE(event.segment.first == 1);
    REQUIRE(event.segment.second == 5);
}

TEST_CASE("Read a speciation event node from a tagged node")
{
    TaggedNode tagged_node{
        "t e st sy n	te\nn", // name
        0., // length
        {
            {"event", "speciation"},
        },
    };

    Event event = tagged_node;

    REQUIRE(event.type == Event::Type::Speciation);
    REQUIRE(event.synteny == Synteny{"t", "e", "st", "sy", "n", "te", "n"});
    REQUIRE(event.segment.first == 0);
    REQUIRE(event.segment.second == 7);
}

TEST_CASE("Read a duplication event node from a tagged node")
{
    TaggedNode tagged_node{
        "t e st sy n	te\nn", // name
        0., // length
        {
            {"event", "duplication"},
            {"segment", "2 - 5"},
        },
    };

    Event event = tagged_node;

    REQUIRE(event.type == Event::Type::Duplication);
    REQUIRE(event.synteny == Synteny{"t", "e", "st", "sy", "n", "te", "n"});
    REQUIRE(event.segment.first == 2);
    REQUIRE(event.segment.second == 6);
}

TEST_CASE("Read a duplication event node, missing segment, from a tagged node")
{
    TaggedNode tagged_node{
        "t e st sy n	te\nn", // name
        0., // length
        {
            {"event", "duplication"},
        },
    };

    Event event = tagged_node;

    REQUIRE(event.type == Event::Type::Duplication);
    REQUIRE(event.synteny == Synteny{"t", "e", "st", "sy", "n", "te", "n"});
    REQUIRE(event.segment.first == 0);
    REQUIRE(event.segment.second == 7);
}

TEST_CASE("Write a leaf node to a tagged node")
{
    Event event;
    event.type = Event::Type::None;
    event.synteny = Synteny{"leaf", "node"};

    TaggedNode tagged_node = event;

    REQUIRE(tagged_node.name == "leaf node");
    REQUIRE(tagged_node.length == Approx(0.));
    REQUIRE(tagged_node.tags == std::map<std::string, std::string>{});
}

TEST_CASE("Write a full loss node to a tagged node")
{
    Event event;
    event.type = Event::Type::Loss;

    TaggedNode tagged_node = event;

    REQUIRE(tagged_node.name == "");
    REQUIRE(tagged_node.length == Approx(0.));
    REQUIRE(tagged_node.tags == std::map<std::string, std::string>{
        {"event", "loss"},
    });
}

TEST_CASE("Write a segmental loss node to a tagged node")
{
    Event event;
    event.type = Event::Type::Loss;
    event.synteny = Synteny{"x", "x'", "x''", "x'''"};
    event.segment.first = 2;
    event.segment.second = 4;

    TaggedNode tagged_node = event;

    REQUIRE(tagged_node.name == "x x' x'' x'''");
    REQUIRE(tagged_node.length == Approx(0.));
    REQUIRE(tagged_node.tags == std::map<std::string, std::string>{
        {"event", "loss"},
        {"segment", "2 - 3"},
    });
}

TEST_CASE("Write a speciation event node to a tagged node")
{
    Event event;
    event.type = Event::Type::Speciation;
    event.synteny = Synteny{"a", "b", "c", "d", "e", "f", "g"};

    TaggedNode tagged_node = event;

    REQUIRE(tagged_node.name == "a b c d e f g");
    REQUIRE(tagged_node.length == Approx(0.));
    REQUIRE(tagged_node.tags == std::map<std::string, std::string>{
        {"event", "speciation"},
    });
}

TEST_CASE("Write a duplication event node to a tagged node")
{
    Event event;
    event.type = Event::Type::Duplication;
    event.synteny = Synteny{"a", "b", "c", "d", "e", "f", "g"};
    event.segment.first = 2;
    event.segment.second = 7;

    TaggedNode tagged_node = event;

    REQUIRE(tagged_node.name == "a b c d e f g");
    REQUIRE(tagged_node.length == Approx(0.));
    REQUIRE(tagged_node.tags == std::map<std::string, std::string>{
        {"event", "duplication"},
        {"segment", "2 - 6"},
    });
}
