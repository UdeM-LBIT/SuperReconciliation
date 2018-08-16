#include <stdexcept>

template<typename T, typename Container>
MultivaluedNumber<T, Container>::MultivaluedNumber()
: values{0}
{}

template<typename T, typename Container>
MultivaluedNumber<T, Container>::MultivaluedNumber(const T& value)
: values{value}
{}

namespace detail
{
template<typename Container>
void create_range(
    Container& dest,
    typename Container::value_type min,
    typename Container::value_type max,
    typename Container::value_type step)
{
    dest.reserve(static_cast<std::size_t>((max - min + 1) / step));

    for (auto i = min; i <= max; i += step)
    {
        dest.push_back(i);
    }
}
}

template<typename T, typename Container>
MultivaluedNumber<T, Container>::MultivaluedNumber(
    const T& min, const T& max, const T& step)
{
    detail::create_range(this->values, min, max, step);
}

template<typename T, typename Container>
MultivaluedNumber<T, Container>::MultivaluedNumber(Container list)
: values{std::move(list)}
{}

template<typename T, typename Container>
bool MultivaluedNumber<T, Container>::isMultivalued() const noexcept
{
    return this->values.size() != 1;
}

template<typename T, typename Container>
typename Container::iterator MultivaluedNumber<T, Container>::begin()
{
    return this->values.begin();
}

template<typename T, typename Container>
typename Container::const_iterator MultivaluedNumber<T, Container>::cbegin()
    const
{
    return this->values.cbegin();
}

template<typename T, typename Container>
typename Container::iterator MultivaluedNumber<T, Container>::end()
{
    return this->values.end();
}

template<typename T, typename Container>
typename Container::const_iterator MultivaluedNumber<T, Container>::cend()
    const
{
    return this->values.cend();
}

template<typename T, typename Container>
std::size_t MultivaluedNumber<T, Container>::size() const
{
    return this->values.size();
}

template<typename T, typename Container>
T MultivaluedNumber<T, Container>::operator*() const
{
    if (this->isMultivalued())
    {
        throw std::logic_error{"This multivalued number contains "
            + std::to_string(this->values.size()) + " values and thus "
            "cannot be converted to a single value"};
    }

    return this->values.at(0);
}

template<typename T, typename Container>
const Container& MultivaluedNumber<T, Container>::getValues() const
{
    return this->values;
}

template<typename T, typename Container>
std::ostream& operator<<(std::ostream& out, const MultivaluedNumber<T, Container>& v)
{
    if (v.isMultivalued())
    {
        out << '{';

        for (auto it = std::begin(v.values); it != std::end(v.values); ++it)
        {
            out << *it;

            if (std::next(it) != std::end(v.values))
            {
                out << ", ";
            }
        }

        out << '}';
    }
    else
    {
        out << *v;
    }

    return out;
}

template<typename T, typename Container>
std::istream& operator>>(std::istream& in, MultivaluedNumber<T, Container>& v)
{
    in >> std::ws;
    v.values.clear();

    if (in.peek() == '[')
    {
        // Read an interval of values
        T min, max, step = 1;

        in.ignore();
        in >> std::ws >> min >> std::ws;

        if (!in || in.peek() != ':')
        {
            in.setstate(std::ios_base::failbit);
            return in;
        }

        in.ignore();
        in >> std::ws >> max >> std::ws;

        if (in.peek() == ':')
        {
            in.ignore();
            in >> std::ws >> step >> std::ws;
        }

        if (!in || in.peek() != ']')
        {
            in.setstate(std::ios_base::failbit);
            return in;
        }

        in.ignore();
        detail::create_range(v.values, min, max, step);
    }
    else if (in.peek() == '{')
    {
        // Read a set of values
        T val;

        in.ignore();
        in >> std::ws;

        while (in && in.peek() != '}')
        {
            in >> val >> std::ws;

            if (!in || (in.peek() != ',' && in.peek() != '}'))
            {
                in.setstate(std::ios_base::failbit);
                return in;
            }

            if (in.peek() == ',')
            {
                in.ignore();
                in >> std::ws;
            }

            v.values.push_back(val);
        }
    }
    else
    {
        // Read a single value
        T val;
        in >> val;

        if (!in)
        {
            in.setstate(std::ios_base::failbit);
            return in;
        }

        v.values.push_back(val);
    }

    return in;
}
