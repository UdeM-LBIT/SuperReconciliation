#include "Synteny.hpp"
#include <catch.hpp>
#include <set>

TEST_CASE("Subsequence generation")
{
    Synteny s0 = {};
    Synteny s1 = {"x"};
    Synteny s2 = {"a", "b", "c"};

    auto s0_sub_vec = s0.generateSubsequences();
    std::set<Synteny> s0_sub_set;
    std::copy(begin(s0_sub_vec), end(s0_sub_vec),
        inserter(s0_sub_set, begin(s0_sub_set)));

    REQUIRE(s0_sub_set == std::set<Synteny>{{}});

    auto s1_sub_vec = s1.generateSubsequences();
    std::set<Synteny> s1_sub_set;
    std::copy(begin(s1_sub_vec), end(s1_sub_vec),
        inserter(s1_sub_set, begin(s1_sub_set)));

    REQUIRE(s1_sub_set == std::set<Synteny>{{"x"}, {}});

    auto s2_sub_vec = s2.generateSubsequences();
    std::set<Synteny> s2_sub_set;
    std::copy(begin(s2_sub_vec), end(s2_sub_vec),
        inserter(s2_sub_set, begin(s2_sub_set)));

    REQUIRE(s2_sub_set == std::set<Synteny>{
        {}, {"a"}, {"b"}, {"c"}, {"a", "b"}, {"b", "c"},
        {"a", "c"}, {"a", "b", "c"}});
}

TEST_CASE("Synteny distance computation")
{
    Synteny s0 = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};
    Synteny s1 = {"1", "4", "5", "6"};
    Synteny s2 = {"4", "5"};
    Synteny s3 = {"2", "4", "8"};

    REQUIRE(s0.distanceTo(s1) == 2);
    REQUIRE(s0.distanceTo(s1, true) == 1);
    REQUIRE(s0.distanceTo(s2) == 2);
    REQUIRE(s0.distanceTo(s2, true) == 0);
    REQUIRE(s0.distanceTo(s3) == 4);
    REQUIRE(s0.distanceTo(s3, true) == 2);
    REQUIRE(s1.distanceTo(s2) == 2);
    REQUIRE(s1.distanceTo(s2, true) == 0);
    REQUIRE_THROWS_AS(s3.distanceTo(s0), std::invalid_argument);
}

TEST_CASE("Synteny reconciliation")
{
    Synteny s0 = {"a", "b", "c", "d"};
    Synteny s1 = {"a", "d"};
    Synteny s2 = {"a", "b", "c"};
    Synteny s3 = {"a", "c"};

    REQUIRE(s0.reconcile(s1, 1).second == Synteny{"a", "d"});
    REQUIRE(s0.reconcile(s2, 1).second == Synteny{"a", "b", "c"});
    REQUIRE(s0.reconcile(s3, 1).second == Synteny{"a", "c", "d"});
    REQUIRE(s2.reconcile(s3, 1).second == Synteny{"a", "c"});
}

TEST_CASE("Synteny difference")
{
    Synteny s0 = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};
    Synteny s1 = {"1", "4", "5", "6"};
    Synteny s2 = {"4", "5"};
    Synteny s3 = {"2", "4", "8"};
    Synteny s4 = {"1", "2", "3", "8", "9"};

    REQUIRE(s0.difference(s1) == "1 [ 2 3 ] 4 5 6 [ 7 8 9 ]");
    REQUIRE(s0.difference(s2) == "[ 1 2 3 ] 4 5 [ 6 7 8 9 ]");
    REQUIRE(s0.difference(s3) == "[ 1 ] 2 [ 3 ] 4 [ 5 6 7 ] 8 [ 9 ]");
    REQUIRE(s0.difference(s4) == "1 2 3 [ 4 5 6 7 ] 8 9");
}
