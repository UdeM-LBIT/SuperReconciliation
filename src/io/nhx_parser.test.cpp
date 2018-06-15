#include "nhx_parser.hpp"
#include <catch.hpp>

TEST_CASE("Parse NHX trees")
{
    SECTION("Untagged and unnamed tree")
    {
        std::string input = "((),());";
        auto tree = parse_nhx_tree(input);

        auto it = tree.begin();
        REQUIRE(it->name == "");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 2);

        ++it;
        REQUIRE(it->name == "");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 1);

        ++it;
        REQUIRE(it->name == "");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 0);

        ++it;
        REQUIRE(it->name == "");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 1);

        ++it;
        REQUIRE(it->name == "");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 0);

        ++it;
        REQUIRE(it == tree.end());
    }

    SECTION("Untagged and named tree")
    {
        std::string input = "((00, 01)0,(10,11)1)r;";
        auto tree = parse_nhx_tree(input);

        auto it = tree.begin();
        REQUIRE(it->name == "r");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 2);

        ++it;
        REQUIRE(it->name == "0");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 2);

        ++it;
        REQUIRE(it->name == "00");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 0);

        ++it;
        REQUIRE(it->name == "01");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 0);

        ++it;
        REQUIRE(it->name == "1");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 2);

        ++it;
        REQUIRE(it->name == "10");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 0);

        ++it;
        REQUIRE(it->name == "11");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 0);

        ++it;
        REQUIRE(it == tree.end());
    }

    SECTION("Length-tagged and partially-named tree")
    {
        std::string input = "(a:10,:8.5,:3.14159)r:0;";
        auto tree = parse_nhx_tree(input);

        auto it = tree.begin();
        REQUIRE(it->name == "r");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 3);

        ++it;
        REQUIRE(it->name == "a");
        REQUIRE(it->length == Approx(10.));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 0);

        ++it;
        REQUIRE(it->name == "");
        REQUIRE(it->length == Approx(8.5));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 0);

        ++it;
        REQUIRE(it->name == "");
        REQUIRE(it->length == Approx(3.14159));
        REQUIRE(it->tags.empty());
        REQUIRE(tree.number_of_children(it) == 0);

        ++it;
        REQUIRE(it == tree.end());
    }

    SECTION("Custom-tagged tree")
    {
        std::string input
            = "(child1[&&NHX:simple=attribute:00=01:test=134],child2[&&NHX:"
            "\"quoted attribute name\"=value],child3[&&NHX:\"quoted attribute "
            "name\"=\"quoted value\"])root:123.321[&&NHX:\"\"\"quoted quote\""
            "\" attribute\"=val];";
        auto tree = parse_nhx_tree(input);

        auto it = tree.begin();
        REQUIRE(it->name == "root");
        REQUIRE(it->length == Approx(123.321));
        REQUIRE(it->tags.size() == 1);
        REQUIRE(it->tags.count("\"quoted quote\" attribute"));
        REQUIRE(it->tags.at("\"quoted quote\" attribute") == "val");
        REQUIRE(tree.number_of_children(it) == 3);

        ++it;
        REQUIRE(it->name == "child1");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.size() == 3);
        REQUIRE(it->tags.count("simple"));
        REQUIRE(it->tags.at("simple") == "attribute");
        REQUIRE(it->tags.count("00"));
        REQUIRE(it->tags.at("00") == "01");
        REQUIRE(it->tags.count("test"));
        REQUIRE(it->tags.at("test") == "134");
        REQUIRE(tree.number_of_children(it) == 0);

        ++it;
        REQUIRE(it->name == "child2");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.size() == 1);
        REQUIRE(it->tags.count("quoted attribute name"));
        REQUIRE(it->tags.at("quoted attribute name") == "value");
        REQUIRE(tree.number_of_children(it) == 0);

        ++it;
        REQUIRE(it->name == "child3");
        REQUIRE(it->length == Approx(0.0));
        REQUIRE(it->tags.size() == 1);
        REQUIRE(it->tags.count("quoted attribute name"));
        REQUIRE(it->tags.at("quoted attribute name") == "quoted value");
        REQUIRE(tree.number_of_children(it) == 0);

        ++it;
        REQUIRE(it == tree.end());
    }
}
