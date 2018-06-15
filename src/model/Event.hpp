#ifndef MODEL_EVENT_HPP
#define MODEL_EVENT_HPP

#include "../io/nhx_parser.hpp"
#include "Synteny.hpp"

/**
 * An event that happened at a node in a synteny tree.
 */
struct Event
{
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

    // Type of event
    Type type = Type::None;

    // Synteny associated with the node
    Synteny synteny;

    Event() = default;

    /**
     * Convert a tagged node to an event node.
     *
     * @param tagnode Source tagged node.
     */
    Event(const TaggedNode&);

    /**
     * Convert an event node to a tagged node.
     *
     * @return Tagged node.
     */
    operator TaggedNode() const;
};

/**
 * Print an event type on an output stream.
 *
 * @param out Output stream to print on.
 * @param number Event type to print.
 *
 * @return Used output stream.
 */
std::ostream& operator<<(std::ostream&, const Event::Type&);

/**
 * Print an event on an output stream.
 *
 * @param out Output stream to print on.
 * @param number Event to print.
 *
 * @return Used output stream.
 */
std::ostream& operator<<(std::ostream&, const Event&);

#endif // MODEL_EVENT_HPP
