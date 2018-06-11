#ifndef SYNTENY_HPP
#define SYNTENY_HPP

#include "ExtendedNumber.hpp"
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
     * synteny into target subsequence.
     * @example base.distanceTo(target), where `base` and `target` are
     *
     * base   = (a b c d e f a b c d e f)
     *               | | |   | |     |
     * target = (    c d e   a b     e  )
     *
     * returns 4, because 4 segments are lost when transforming `base` into
     * `target`.
     * @param target Target subsequence synteny.
     * @param [substring=false] When set to true, does not count any initial
     * or terminal segmental loss. In this case, the result is the minimum
     * number of segmental losses required to turn a substring of this synteny
     * into `target`.
     * @throws std::invalid_argument If `target` is not a subsequence of the
     * current synteny.
     * @return Minimum number of segmental losses required to turn this synteny
     * into the target synteny.
     */
    int distanceTo(const Synteny&, bool = false) const;

    /**
     * Compute a reconciled synteny that is closer to a target subsequence than
     * the current one in terms of loss distance, but that contains no more than
     * a given amount of losses.
     * @example base.reconcile(target, 2), where `base` and `target` are
     *
     * base   = (a b c d e f a b c d e f)
     *               | | |   | |     |
     * target = (    c d e   a b     e  )
     *
     * returns (c d e a b c d e) and 2, because the reconciled synteny was
     * obtained by erasing 2 segments from `base`.
     * @param target Subsequence synteny to reconcile with.
     * @param max Maximum number of segments that can be erased.
     * @param [substring=false] When set to true, does not count the initial
     * and terminal segmental losses. In this case, the result is the minimum
     * number of segmental losses required to turn a substring of this synteny
     * into `target`.
     * @return Number of erased segments to turn this synteny into the
     * reconciled synteny, and the reconciled synteny, closer to the target
     * than this synteny, but containing no more than `max` losses.
     */
    std::pair<int, Synteny> reconcile(
        const Synteny&, ExtendedNumber<int>, bool = false) const;
};

/**
 * Print a synteny on an output stream.
 * @param out Output stream to print on.
 * @param synteny Synteny to print.
 * @return Used output stream.
 */
std::ostream& operator<<(std::ostream&, const Synteny&);

#endif // SYNTENY_HPP
