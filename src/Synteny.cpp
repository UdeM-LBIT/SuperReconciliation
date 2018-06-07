#include "Synteny.hpp"
#include <stdexcept>

std::vector<Synteny> Synteny::generateSubsequences() const
{
    if (this->empty())
    {
        return {{}};
    }

    auto rest = *this;
    rest.pop_front();
    auto result_rest = rest.generateSubsequences();

    std::vector<Synteny> result;

    for (auto subsequence : result_rest)
    {
        result.push_back(subsequence);
        subsequence.push_front(this->front());
        result.push_back(std::move(subsequence));
    }

    return result;
}

int Synteny::distanceTo(const Synteny& to, bool substring) const
{
    auto it_from = std::cbegin(*this);
    auto it_to = std::cbegin(to);

    // Nombre de pertes segmentales nécessaires pour transformer la
    // synténie [begin_from, it_from) en [begin_to, it_to)
    int result = 0;

    // Vaut vrai si et seulement si les gènes précédant it_from et it_to
    // dans leur synténie respective sont égaux. À l’initialisation, on
    // considère que oui seulement si on veut compter les éventuelles
    // pertes segmentales initiales (seulement hors du mode `substring`)
    bool coincides = !substring;

    // Parcours des deux synténies en même temps pour identifier les
    // segments de gènes perdus
    while (it_from != std::cend(*this) && it_to != std::cend(to))
    {
        if (*it_from != *it_to)
        {
            if (coincides)
            {
                // Démarrage d’un nouveau segment de perte
                ++result;
                coincides = false;
            }

            // Avancement dans la synténie originelle jusqu’à réidentifier
            // un segment conservé
            ++it_from;
        }
        else
        {
            // Progression au sein d’un segment conservé
            coincides = true;
            ++it_from;
            ++it_to;
        }
    }

    // Si la synténie originelle se finit avant la nouvelle synténie, la
    // nouvelle synténie n’est pas une sous-séquence de la synténie originelle
    if (it_from == std::cend(*this) && it_to != std::cend(to))
    {
        throw std::invalid_argument{"La synténie en paramètre doit être "
            "une sous-séquence de la synténie utilisée."};
    }

    // Si la synténie originelle se finit après la nouvelle synténie, un
    // un segment final a été perdu. On décompte ce segment final seulement
    // hors du mode `substring`.
    if (it_from != std::cend(*this) && it_to == std::cend(to) && !substring)
    {
        ++result;
    }

    return result;
}

std::ostream& operator<<(std::ostream& out, const Synteny& synteny)
{
    for (auto it = std::begin(synteny); it != std::end(synteny); ++it)
    {
        out << *it;

        if (std::next(it) != std::end(synteny))
        {
            out << " ";
        }
    }

    return out;
}
