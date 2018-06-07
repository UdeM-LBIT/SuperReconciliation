#include "Event.hpp"

Event::Event()
: type(Type::None)
{}

Event::Event(Synteny synteny)
: type(Type::None), synteny(synteny)
{}

Event::Event(Type type)
: type(type)
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
