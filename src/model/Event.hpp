#ifndef MODEL_EVENT_HPP
#define MODEL_EVENT_HPP

#include "Synteny.hpp"

/**
 * An event that happened at a node in a synteny tree.
 */
class Event
{
public:
    /**
     * Kind of event.
     */
    enum class Type
    {
        None, //< No event: applicable for leaf nodes
        Duplication, //< Segmental duplication event
        Speciation, //< Speciation event
        Loss, //< Segmental loss event
    };

    /**
     * Create an empty event (type `Event::Type::None` and empty synteny).
     */
    Event();

    /**
     * Create an event.
     * @param type Event type to associate with the node.
     * @param synteny Synteny to associate with the node.
     */
    Event(Type, Synteny);

    /**
     * Get the event’s type.
     * @return Event type.
     */
    Type getType() const noexcept;

    /**
     * Get the event’s synteny.
     * @return Event synteny.
     */
    const Synteny& getSynteny() const noexcept;

    /**
     * Set the event’s synteny.
     * @param synteny New synteny to associate with the node.
     */
    void setSynteny(const Synteny&) noexcept;

private:
    // Type of event
    Type type;

    // Synteny associated with the node
    Synteny synteny;
};

/**
 * Print an event type on an output stream.
 * @param out Output stream to print on.
 * @param number Event type to print.
 * @return Used output stream.
 */
std::ostream& operator<<(std::ostream&, const Event::Type&);

/**
 * Print an event on an output stream.
 * @param out Output stream to print on.
 * @param number Event to print.
 * @return Used output stream.
 */
std::ostream& operator<<(std::ostream&, const Event&);

#endif // MODEL_EVENT_HPP
