#include "MultivaluedNumber.hpp"
#include <catch.hpp>
#include <list>
#include <sstream>

TEST_CASE("Supports monovalued numbers")
{
    SECTION("Default constructor should create monovalued number")
    {
        MultivaluedNumber<int> a;
        REQUIRE_FALSE(a.isMultivalued());
    }

    SECTION("Single-valued unwrapping")
    {
        MultivaluedNumber<int> a;
        MultivaluedNumber<int> b(5);
        MultivaluedNumber<int> c(1, 10, 1);

        REQUIRE(*a == 0);
        REQUIRE(*b == 5);
        REQUIRE_THROWS_AS(*c, std::logic_error);

        REQUIRE(a.getValues() == std::vector<int>{0});
        REQUIRE(b.getValues() == std::vector<int>{5});
    }

    SECTION("Iterate on a single value")
    {
        MultivaluedNumber<int> a;
        int count = 0;

        for (int val : a)
        {
            REQUIRE(val == 0);
            ++count;
        }

        REQUIRE(count == 1);
    }
}

TEST_CASE("Supports multivalued numbers")
{
    SECTION("Construct from any container")
    {
        std::vector<int> a_val{1, 2, 3, 10};
        MultivaluedNumber<int> a = a_val;

        std::list<int> b_val{1, 2, 9, 10};
        MultivaluedNumber<int, std::list<int>> b = b_val;

        auto a_it = std::begin(a);
        REQUIRE(*a_it == 1);
        REQUIRE(*++a_it == 2);
        REQUIRE(*++a_it == 3);
        REQUIRE(*++a_it == 10);
        REQUIRE(++a_it == std::end(a));

        auto b_it = std::begin(b);
        REQUIRE(*b_it == 1);
        REQUIRE(*++b_it == 2);
        REQUIRE(*++b_it == 9);
        REQUIRE(*++b_it == 10);
        REQUIRE(++b_it == std::end(b));
    }

    SECTION("Range construction with default step")
    {
        MultivaluedNumber<int> a(1, 100);
        auto it = std::begin(a);

        for (int i = 1; i <= 100; ++i, ++it)
        {
            REQUIRE(i == *it);
        }

        REQUIRE(it == std::end(a));
    }

    SECTION("Range construction with arbitrary step")
    {
        MultivaluedNumber<int> a(1, 99, 10);
        auto it = std::begin(a);

        for (int i = 1; i <= 99; i += 10, ++it)
        {
            REQUIRE(i == *it);
        }

        REQUIRE(it == std::end(a));
    }

    SECTION("Access values")
    {
        MultivaluedNumber<int> a(1, 5, 2);
        REQUIRE(a.getValues() == std::vector<int>{1, 3, 5});
    }
}

TEST_CASE("Input from istream")
{
    SECTION("Single value")
    {
        std::istringstream in1("5abc");
        MultivaluedNumber<int> a;
        in1 >> a;
        REQUIRE(in1);
        REQUIRE(*a == 5);

        std::istringstream in2("   105.2 ");
        MultivaluedNumber<double> b;
        in2 >> b;
        REQUIRE(in2);
        REQUIRE(*b == Approx(105.2));

        std::istringstream in3(" 5   100");
        MultivaluedNumber<short> c;
        in3 >> c;
        REQUIRE(in3);
        REQUIRE(*c == 5);

        std::istringstream in4("?");
        MultivaluedNumber<int> d;
        in4 >> d;
        REQUIRE(in4.fail());
    }

    SECTION("Set of values")
    {
        std::istringstream in1("{1, 2, 3, 4,} test");
        MultivaluedNumber<int> a;
        in1 >> a;

        REQUIRE(in1);
        auto it_a = std::begin(a);

        REQUIRE(*it_a == 1);
        REQUIRE(*++it_a == 2);
        REQUIRE(*++it_a == 3);
        REQUIRE(*++it_a == 4);
        REQUIRE(++it_a == std::end(a));

        std::istringstream in2("\t\t{ 0.12 , 1.2, 0 ,4}");
        MultivaluedNumber<double> b;
        in2 >> b;

        REQUIRE(in2);
        auto it_b = std::begin(b);

        REQUIRE(*it_b == Approx(0.12));
        REQUIRE(*++it_b == Approx(1.2));
        REQUIRE(*++it_b == Approx(0));
        REQUIRE(*++it_b == Approx(4));
        REQUIRE(++it_b == std::end(b));

        std::istringstream in3("{1, 2, 3");
        MultivaluedNumber<int> c;
        in3 >> c;
        REQUIRE(in3.fail());

        std::istringstream in4("{1 2 3}");
        MultivaluedNumber<int> d;
        in4 >> d;
        REQUIRE(in4.fail());

        std::istringstream in5("{}");
        MultivaluedNumber<int> e;
        in5 >> e;

        REQUIRE(in5);
        REQUIRE(std::begin(e) == std::end(e));
    }

    SECTION("Range of values")
    {
        std::istringstream in1("[1:5]");
        MultivaluedNumber<int> a;
        in1 >> a;

        REQUIRE(in1);
        auto it_a = std::begin(a);

        REQUIRE(*it_a == 1);
        REQUIRE(*++it_a == 2);
        REQUIRE(*++it_a == 3);
        REQUIRE(*++it_a == 4);
        REQUIRE(*++it_a == 5);
        REQUIRE(++it_a == std::end(a));

        std::istringstream in2("[  1: 100: 25 ]");
        MultivaluedNumber<int> b;
        in2 >> b;

        REQUIRE(in2);
        auto it_b = std::begin(b);

        REQUIRE(*it_b == 1);
        REQUIRE(*++it_b == 26);
        REQUIRE(*++it_b == 51);
        REQUIRE(*++it_b == 76);
        REQUIRE(++it_b == std::end(b));

        std::istringstream in3("[0:1: 0.1]");
        MultivaluedNumber<double> c;
        in3 >> c;

        REQUIRE(in3);
        auto it_c = std::begin(c);

        REQUIRE(*it_c == Approx(0));
        REQUIRE(*++it_c == Approx(0.1));
        REQUIRE(*++it_c == Approx(0.2));
        REQUIRE(*++it_c == Approx(0.3));
        REQUIRE(*++it_c == Approx(0.4));
        REQUIRE(*++it_c == Approx(0.5));
        REQUIRE(*++it_c == Approx(0.6));
        REQUIRE(*++it_c == Approx(0.7));
        REQUIRE(*++it_c == Approx(0.8));
        REQUIRE(*++it_c == Approx(0.9));
        REQUIRE(*++it_c == Approx(1));
        REQUIRE(++it_c == std::end(c));
    }
}
