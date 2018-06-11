#ifndef SYNTENY_HPP
#define SYNTENY_HPP

#include "Gene.hpp"
#include <iostream>
#include <list>
#include <vector>

/**
 * A synteny is an ordered block of genes.
 */
class Synteny : public std::list<Gene>
{
public:
    using std::list<Gene>::list;

    /**
     * Generate all possible subsequences for this synteny.
     * @return List of all possible syntenies that are subsequences.
     */
    std::vector<Synteny> generateSubsequences() const;

    /**
     * Compute the minimum number of segmental losses required to turn this
     * synteny into another one.
     * @example Four segemental losses (or two with the `substring` flag) are
     * required to turn the `from` synteny into the `to` synteny:
     *
     * from = (a b c d e f a b c d e f)
     *             | | |   | |     |
     * to   = (    c d e   a b     e  )
     * @param to New synteny.
     * @param [substring=false] When set to true, does not count the initial
     * and terminal segmental losses. In this case, the result is the minimum
     * number of segmental losses required to turn a substring of this synteny
     * into `to`.
     * @return Minimum number of segmental losses required to turn this synteny
     * into the `to` synteny.
     */
    int distanceTo(const Synteny&, bool = false) const;
};

/**
 * Print a synteny on an output stream.
 * @param out Output stream to print on.
 * @param synteny Synteny to print.
 * @return Used output stream.
 */
std::ostream& operator<<(std::ostream&, const Synteny&);

#endif // SYNTENY_HPP
