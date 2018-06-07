#include "ExtendedNumber.hpp"
#include <catch.hpp>

TEST_CASE("Supporte les opérations sur les nombres réels")
{
    ExtendedNumber<int> a = 10, b = 8;
    ExtendedNumber<double> c = 22, d = 7;

    SECTION("Conversion de type")
    {
        REQUIRE(static_cast<int>(a) == 10);
    }

    SECTION("Comparaisons internes")
    {
        REQUIRE_FALSE(a < b);
        REQUIRE_FALSE(a == b);
        REQUIRE(a != b);
        REQUIRE_FALSE(a <= b);
        REQUIRE(a > b);
        REQUIRE(a >= b);
    }

    SECTION("Comparaisons avec le type enveloppé")
    {
        REQUIRE(8 < a);
        REQUIRE(a < 12);
        REQUIRE(a == 10);
        REQUIRE(10 == a);
        REQUIRE(a != 8);
        REQUIRE(8 != a);
        REQUIRE(10 <= a);
        REQUIRE(a <= 10);
        REQUIRE(12 > a);
        REQUIRE(a > 8);
        REQUIRE(10 >= a);
        REQUIRE(a >= 9);
    }

    SECTION("Arithmétique immuable")
    {
        REQUIRE(a + b == 18);
        REQUIRE(a - b == 2);
        REQUIRE(a * b == 80);
        REQUIRE(a / b == 1);
        REQUIRE(a / 2 == 5);
        REQUIRE(c / d == Approx(3.142857143));
        REQUIRE(a + 2 == 12);
        REQUIRE(20. * c == Approx(440));
        REQUIRE(-a == -10);
        REQUIRE(a == +a);
    }

    SECTION("Arithmétique modifiante")
    {
        a += b;

        REQUIRE(a == 18);
        REQUIRE(b == 8);

        b -= a;

        REQUIRE(a == 18);
        REQUIRE(b == -10);

        a *= b * b;

        REQUIRE(a == 1800);
        REQUIRE(b == -10);

        b /= a;

        REQUIRE(a == 1800);
        REQUIRE(b == -0);
    }
}

TEST_CASE("Supporte les opérations sur les réels étendus")
{
    ExtendedNumber<int> a = -10, b = 0, c = 10;
    ExtendedNumber<int> pinf = ExtendedNumber<int>::positiveInfinity();
    ExtendedNumber<int> ninf = ExtendedNumber<int>::negativeInfinity();

    SECTION("Représentation de l’infini")
    {
        REQUIRE_FALSE(a.isInfinity());
        REQUIRE_FALSE(a.isPositiveInfinity());
        REQUIRE_FALSE(a.isNegativeInfinity());

        REQUIRE(pinf.isInfinity());
        REQUIRE(pinf.isPositiveInfinity());
        REQUIRE_FALSE(pinf.isNegativeInfinity());

        REQUIRE(ninf.isInfinity());
        REQUIRE_FALSE(ninf.isPositiveInfinity());
        REQUIRE(ninf.isNegativeInfinity());
    }

    SECTION("Conversion de type")
    {
        REQUIRE_THROWS_AS(static_cast<int>(pinf), std::domain_error);
        REQUIRE_THROWS_AS(static_cast<int>(ninf), std::domain_error);
    }

    SECTION("Comparaisons internes")
    {
        REQUIRE(a < pinf);
        REQUIRE(b < pinf);
        REQUIRE(c < pinf);
        REQUIRE_FALSE(a == pinf);
        REQUIRE_FALSE(b == pinf);
        REQUIRE_FALSE(c == pinf);
        REQUIRE(a <= pinf);
        REQUIRE(b <= pinf);
        REQUIRE(c <= pinf);
        REQUIRE_FALSE(a > pinf);
        REQUIRE_FALSE(b > pinf);
        REQUIRE_FALSE(c > pinf);
        REQUIRE(a > ninf);
        REQUIRE(b > ninf);
        REQUIRE(c > ninf);
        REQUIRE(a >= ninf);
        REQUIRE(b >= ninf);
        REQUIRE(c >= ninf);
        REQUIRE_FALSE(a == ninf);
        REQUIRE_FALSE(b == ninf);
        REQUIRE_FALSE(c == ninf);
        REQUIRE(pinf == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(ninf == ExtendedNumber<int>::negativeInfinity());
        REQUIRE_FALSE(pinf == ninf);
        REQUIRE(pinf != ninf);
    }

    SECTION("Arithmétique immuable")
    {
        REQUIRE(pinf + a == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(pinf + b == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(pinf + c == ExtendedNumber<int>::positiveInfinity());

        REQUIRE(a + pinf == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(b + pinf == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(c + pinf == ExtendedNumber<int>::positiveInfinity());

        REQUIRE(pinf + 0 == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(pinf + 20 == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(pinf + -100 == ExtendedNumber<int>::positiveInfinity());

        REQUIRE(25 + pinf == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(-10 + pinf == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(4000 + pinf == ExtendedNumber<int>::positiveInfinity());

        REQUIRE(ninf + a == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(ninf + b == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(ninf + c == ExtendedNumber<int>::negativeInfinity());

        REQUIRE(a + ninf == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(b + ninf == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(c + ninf == ExtendedNumber<int>::negativeInfinity());

        REQUIRE(ninf + 0 == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(ninf + 20 == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(ninf + -100 == ExtendedNumber<int>::negativeInfinity());

        REQUIRE(25 + ninf == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(-10 + ninf == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(4000 + ninf == ExtendedNumber<int>::negativeInfinity());

        REQUIRE(pinf + pinf == pinf);
        REQUIRE(ninf + ninf == ninf);
        REQUIRE_THROWS_AS(pinf + ninf, std::domain_error);
        REQUIRE_THROWS_AS(ninf + pinf, std::domain_error);

        REQUIRE(pinf - a == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(pinf - b == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(pinf - c == ExtendedNumber<int>::positiveInfinity());

        REQUIRE(a - pinf == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(b - pinf == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(c - pinf == ExtendedNumber<int>::negativeInfinity());

        REQUIRE(pinf - 0 == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(pinf - 20 == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(pinf - -100 == ExtendedNumber<int>::positiveInfinity());

        REQUIRE(25 - pinf == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(-10 - pinf == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(4000 - pinf == ExtendedNumber<int>::negativeInfinity());

        REQUIRE(ninf - a == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(ninf - b == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(ninf - c == ExtendedNumber<int>::negativeInfinity());

        REQUIRE(a - ninf == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(b - ninf == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(c - ninf == ExtendedNumber<int>::positiveInfinity());

        REQUIRE(ninf - 0 == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(ninf - 20 == ExtendedNumber<int>::negativeInfinity());
        REQUIRE(ninf - -100 == ExtendedNumber<int>::negativeInfinity());

        REQUIRE(25 - ninf == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(-10 - ninf == ExtendedNumber<int>::positiveInfinity());
        REQUIRE(4000 - ninf == ExtendedNumber<int>::positiveInfinity());

        REQUIRE(pinf - ninf == pinf);
        REQUIRE(ninf - pinf == ninf);
        REQUIRE_THROWS_AS(pinf - pinf, std::domain_error);
        REQUIRE_THROWS_AS(ninf - ninf, std::domain_error);

        REQUIRE(a * pinf == ExtendedNumber<int>::negativeInfinity());
        REQUIRE_THROWS_AS(b * pinf, std::domain_error);
        REQUIRE(c * pinf == ExtendedNumber<int>::positiveInfinity());

        REQUIRE(pinf * a == ExtendedNumber<int>::negativeInfinity());
        REQUIRE_THROWS_AS(pinf * b, std::domain_error);
        REQUIRE(pinf * c == ExtendedNumber<int>::positiveInfinity());

        REQUIRE(-10 * pinf == ExtendedNumber<int>::negativeInfinity());
        REQUIRE_THROWS_AS(0 * pinf, std::domain_error);
        REQUIRE(10 * pinf == ExtendedNumber<int>::positiveInfinity());

        REQUIRE(pinf * -10 == ExtendedNumber<int>::negativeInfinity());
        REQUIRE_THROWS_AS(pinf * 0, std::domain_error);
        REQUIRE(pinf * 10 == ExtendedNumber<int>::positiveInfinity());

        REQUIRE(pinf * pinf == pinf);
        REQUIRE(ninf * pinf == ninf);
        REQUIRE(pinf * ninf == ninf);
        REQUIRE(ninf * ninf == pinf);

        REQUIRE(a / pinf == -0);
        REQUIRE(a / ninf == 0);
        REQUIRE(pinf / a == ninf);
        REQUIRE(ninf / a == pinf);
        REQUIRE(pinf / c == pinf);
        REQUIRE(ninf / c == ninf);
        REQUIRE_THROWS_AS(c / b, std::domain_error);
        REQUIRE_THROWS_AS(pinf / b, std::domain_error);
        REQUIRE_THROWS_AS(pinf / ninf, std::domain_error);
        REQUIRE_THROWS_AS(pinf / pinf, std::domain_error);
    }
}
