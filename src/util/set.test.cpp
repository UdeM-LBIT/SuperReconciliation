#include "set.hpp"
#include <catch.hpp>

TEST_CASE("Operator overloads on sets")
{
    SECTION("Set union")
    {
        REQUIRE(std::set<int>{1, 2, 3}
                + std::set<int>{-1, 0, 2, 5, 6}
                == std::set<int>{-1, 0, 1, 2, 3, 5, 6});

        REQUIRE(std::set<int>{1, 3, 5, 7, 9}
                + std::set<int>{0, 2, 4, 6, 8}
                == std::set<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9});
    }

    SECTION("Set difference")
    {
        REQUIRE(std::set<int>{2, 3, 5, 7}
                - std::set<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
                == std::set<int>{});

        REQUIRE(std::set<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
                - std::set<int>{0, 2, 4, 6, 8}
                == std::set<int>{1, 3, 5, 7, 9});
    }

    SECTION("Set intersection")
    {
        REQUIRE((std::set<char>{'a', 'b', 'c', 'd', 'e'}
                    & std::set<char>{'b', 'd', 'f', 'l'})
                == std::set<char>{'b', 'd'});

        REQUIRE((std::set<char>{'a', 'e', 'i', 'o', 'u', 'y'}
                    & std::set<char>{'b', 'c', 'd', 'f', 'g', 'h', 'j'})
                == std::set<char>{});
    }

    SECTION("Set symmetrical difference")
    {
        REQUIRE((std::set<char>{'a', 'b', 'c', 'd', 'e'}
                    ^ std::set<char>{'c', 'd', 'f', 'g'})
                == std::set<char>{'a', 'b', 'e', 'f', 'g'});
    }

    SECTION("Subset test")
    {
        REQUIRE(std::set<int>{0, 1, 2} << std::set<int>{-1, 0, 1, 2, 3});
        REQUIRE_FALSE(std::set<int>{0, 5, 10} << std::set<int>{});
        REQUIRE(std::set<int>{} << std::set<int>{0});
        REQUIRE(std::set<int>{} << std::set<int>{});
    }
}
