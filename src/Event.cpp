#include "Event.hpp"

Event::Event()
: type(Type::None)
{}

Event::Event(Type type, Synteny synteny)
: type(type), synteny(synteny)
{}

Event::Type Event::getType() const noexcept
{
    return this->type;
}

const Synteny& Event::getSynteny() const noexcept
{
    return this->synteny;
}

void Event::setSynteny(const Synteny& synteny) noexcept
{
    this->synteny = synteny;
}

std::ostream& operator<<(std::ostream& out, const Event::Type& type)
{
    switch (type)
    {
    case Event::Type::None:
        return out << "None";

    case Event::Type::Duplication:
        return out << "Duplication";

    case Event::Type::Speciation:
        return out << "Speciation";

    case Event::Type::Loss:
        return out << "Loss";
    }

    return out << "[Invalid type]";
}

std::ostream& operator<<(std::ostream& out, const Event& event)
{
    return out << "Event {\n\ttype = " << event.getType() << "\n\tsynteny = "
        << event.getSynteny() << "\n}";
}
