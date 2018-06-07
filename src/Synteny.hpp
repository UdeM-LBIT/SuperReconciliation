#ifndef SYNTENY_HPP
#define SYNTENY_HPP

#include "Gene.hpp"
#include <iostream>
#include <list>
#include <vector>

/**
 * Synténie : suite ordonnée de gènes.
 */
class Synteny : public std::list<Gene>
{
public:
    using std::list<Gene>::list;

    /**
     * Génère la liste des sous-séquences possibles de cette synténie.
     *
     * @return Liste de synténies qui sont des sous-séquences de celle-ci.
     */
    std::vector<Synteny> generateSubsequences() const;

    /**
     * Détermine le nombre minimum de pertes segmentales nécessaires pour
     * transformer cette synténie en une autre.
     *
     * @example Quatre pertes segmentales (ou deux en mode `substring`) :
     *
     * from = (a b c d e f a b c d e f)
     *             | | |   | |     |
     * to   = (    c d e   a b     e  )
     *
     * @param to Nouvelle synténie.
     * @param [substring=false] Si vrai, ne décompte pas les pertes segmentales
     * initiales et terminales. Dans ce cas, le résultat est le nombre minimum de
     * pertes segmentales nécessaires pour transformer une sous-chaîne de `from`
     * en `to`.
     * @return Nombre minimum de pertes segmentales nécessaires pour
     * transformer `from` en `to`.
     */
    int distanceTo(const Synteny&, bool = false) const;
};

/**
 * Affiche une synténie sur un flux de sortie.
 *
 * @param out Flux de sortie à utiliser.
 * @param synteny Synténie à afficher.
 * @return Flux de sortie utilisé.
 */
std::ostream& operator<<(std::ostream&, const Synteny&);

#endif // SYNTENY_HPP
