#ifndef MODEL_EVENT_HPP
#define MODEL_EVENT_HPP

#include "../io/nhx.hpp"
#include "Synteny.hpp"
#include <utility>

/**
 * An event that happened at a node in a synteny tree.
 */
struct Event
{
    /**
     * Types of events.
     */
    enum class Type
    {
        /**
         * No event: it is a leaf node.
         */
        None,

        /**
         * Duplication event: the two child syntenies belong to the same
         * species and were created following the duplication of a segment
         * of the current synteny.
         */
        Duplication,

        /**
         * Speciation event: the two child syntenies belong to two different
         * species that evolved from this common ancestor.
         */
        Speciation,

        /**
         * Loss event:
         *
         *  * if `synteny` is empty, represents a full loss of the
         *    ancestral synteny;
         *  * otherwise, a segment of the current synteny was lost in
         *    the child.
         */
        Loss,
    };

    /**
     * Type of the current event.
     */
    Type type = Type::None;

    /**
     * Synteny of the current event.
     */
    Synteny synteny;

    /**
     * Segment of the current synteny which is involved in this event.
     * The meaning of this interval is specific to each event type.
     */
    Synteny::Segment segment;

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
