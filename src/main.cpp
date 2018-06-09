#include "Event.hpp"
#include "ExtendedNumber.hpp"
#include "tree_parser.hpp"
#include "util.hpp"
#include <iostream>
#include <limits>
#include <map>
#include <tree.hh>

/**
 * Détermine les assignations synténiques sur les nœuds internes d’un arbre de
 * synténies de telle sorte à minimiser le coût total en duplications et
 * pertes segmentales. Il s’agit du « small philogeny problem » appliqué aux
 * synténies.
 *
 * @param tree Arbre de synténies dont les feuilles sont déjà étiquetées avec
 * les synténies étudiées et les nœuds internes avec le type d’événement
 * (spéciation ou duplication). Cet arbre est modifié de telle sorte à ce que
 * l’assignation synténique optimale soit assignée à chaque nœud interne.
 * @param base Synténie ancestrale, étiquette de la racine.
 * @return Coût de l’assignation synténique optimale déterminée (nombre de
 * duplications et pertes segmentales).
 */
int small_philogeny_for_syntenies(tree<Event>& tree, const Synteny& base)
{
    // Résolution du problème par programmation dynamique, selon la méthode
    // décrite dans l’article « Reconstructing the history of syntenies through
    // Super-Reconciliation » (El-Mabrouk et al., 2015)

    // Les coûts (nombre de duplications et pertes segmentales) sont modélisés
    // par un entier étendu, qui permet de coder les infinis
    using Cost = ExtendedNumber<int>;

    // Pour chaque nœud, on appelle synténie candidate une possible affectation
    // synténique pour ce nœud. Cette structure permet de stocker les
    // informations relatives à un candidat
    struct Candidate
    {
    public:
        // Chaque synténie candidate dispose d’un coût (éventuellement infini).
        // Il s’agit de la valeur d(v, X) définie dans l’article, telle que
        // v est le nœud dont la synténie X est candidate
        Cost cost;

        // Si cette synténie candidate est le candidat optimal, alors ses deux
        // enfants doivent être affectés aux synténies suivantes. Si le nœud
        // est une feuille, ces valeurs ne sont pas significatives
        Synteny synteny_left;
        Synteny synteny_right;

        Candidate()
        : cost(0)
        {}

        Candidate(Cost cost)
        : cost(cost)
        {}

        Candidate(Cost cost, Synteny synteny_left, Synteny synteny_right)
        : cost(cost), synteny_left(synteny_left), synteny_right(synteny_right)
        {}
    };

    // Liste de toutes les synténies candidates possibles dérivée de la
    // synténie ancestrale
    auto possibilities = base.generateSubsequences();

    // Structure de données stockant toutes les synténies candidates pour un
    // nœud donné. On associe ici chaque synténie candidate (clé du
    // dictionnaire) aux informations qui lui sont associées (valeur)
    using CandidateMapping = std::map<Synteny, Candidate>;

    // Associe à chaque nœud de l’arbre (événement) ses synténies candidates
    std::map<Event*, CandidateMapping> candidates_per_node;

    // Remplissage du dictionnaire `candidates_per_node` par programmation
    // dynamique. On parcourt l’arbre en ordre postfixe
    for (
        auto it = tree.begin_post();
        it != tree.end_post();
        ++it)
    {
        CandidateMapping candidates;

        if (tree.number_of_children(it) == 0)
        {
            // Pour les feuilles, la seule synténie candidate possible est
            // celle qui lui est déjà affectée : son coût est de 0. On associe
            // à toutes les autres candidates un coût infini afin de forcer la
            // conservation des affectations existantes
            for (const Synteny& candidate : possibilities)
            {
                candidates.emplace(
                    candidate,
                    Candidate{candidate == it->getSynteny()
                        ? 0
                        : Cost::positiveInfinity()});
            }
        }
        else if (tree.number_of_children(it) == 2)
        {
            Event& child_left = *tree.child(it, 0);
            Event& child_right = *tree.child(it, 1);

            for (const Synteny& candidate : possibilities)
            {
                // Pour chaque candidate, on évalue les sous-candidates
                // possibles qui peuvent être affectées aux enfants. Ces
                // sous-candidates ont déjà un coût associé puisque l’arbre
                // est traversé en ordre postfixe
                auto sub_possibilities = candidate.generateSubsequences();

                // Recherche des synténies qui ont le moindre coût total et
                // partiel (autorise les sous-chaînes) pour les deux enfants
                auto best_total_left_cost = Cost::positiveInfinity();
                Synteny best_total_left_synteny;

                auto best_partial_left_cost = Cost::positiveInfinity();
                Synteny best_partial_left_synteny;

                auto best_total_right_cost = Cost::positiveInfinity();
                Synteny best_total_right_synteny;

                auto best_partial_right_cost = Cost::positiveInfinity();
                Synteny best_partial_right_synteny;

                for (const Synteny& sub_candidate : sub_possibilities)
                {
                    auto total_dist = candidate.distanceTo(sub_candidate);
                    auto partial_dist = candidate.distanceTo(sub_candidate, true);

                    auto total_left_cost = total_dist + candidates_per_node
                        .at(&child_left).at(sub_candidate).cost;

                    if (total_left_cost < best_total_left_cost)
                    {
                        best_total_left_cost = total_left_cost;
                        best_total_left_synteny = sub_candidate;
                    }

                    auto partial_left_cost = partial_dist + candidates_per_node
                        .at(&child_left).at(sub_candidate).cost;

                    if (partial_left_cost < best_partial_left_cost)
                    {
                        best_partial_left_cost = partial_left_cost;
                        best_partial_left_synteny = sub_candidate;
                    }

                    auto total_right_cost = total_dist + candidates_per_node
                        .at(&child_right).at(sub_candidate).cost;

                    if (total_right_cost < best_total_right_cost)
                    {
                        best_total_right_cost = total_right_cost;
                        best_total_right_synteny = sub_candidate;
                    }

                    auto partial_right_cost = partial_dist + candidates_per_node
                        .at(&child_right).at(sub_candidate).cost;

                    if (partial_right_cost < best_partial_right_cost)
                    {
                        best_partial_right_cost = partial_right_cost;
                        best_partial_right_synteny = sub_candidate;
                    }
                }

                auto best_total = best_total_left_cost + best_total_right_cost;
                auto best_total_partial
                    = best_total_left_cost + best_partial_right_cost;
                auto best_partial_total
                    = best_partial_left_cost + best_total_right_cost;

                switch (it->getType())
                {
                case Event::Type::Speciation:
                    // Pour les nœuds de spéciation, un seul scénario est
                    // possible : les deux enfants ont été intégralement
                    // copiés. S’il y a des pertes, elles sont nécessairement
                    // dues à des pertes segmentales suivant la spéciation
                    // et il faut donc les compter dans le coût
                    candidates.emplace(candidate, Candidate{best_total,
                        best_total_left_synteny, best_total_right_synteny});
                    break;

                case Event::Type::Duplication:
                    // Pour les nœuds de duplication, on peut autoriser au
                    // plus une duplication segmentale sur l’un des deux
                    // enfants. On considère le scénario le plus avantageux
                    // entre duplication totale, partielle à gauche et totale
                    // à droite ou totale à gauche et partielle à droite
                    if (best_total <= best_total_partial
                        && best_total <= best_partial_total)
                    {
                        candidates.emplace(candidate, Candidate{1 + best_total,
                            best_total_left_synteny, best_total_right_synteny});
                    }
                    else if (best_total_partial <= best_total
                        && best_total_partial <= best_partial_total)
                    {
                        candidates.emplace(candidate, Candidate{1 + best_total_partial,
                            best_total_left_synteny, best_partial_right_synteny});
                    }
                    else if (best_partial_total <= best_total
                        && best_partial_total <= best_total_partial)
                    {
                        candidates.emplace(candidate, Candidate{1 + best_partial_total,
                            best_partial_left_synteny, best_total_right_synteny});
                    }
                    break;

                default:
                    throw std::invalid_argument{"Type d’événement incorrect "
                        "sur un nœud à 2 enfants."};
                }
            }
        }

        candidates_per_node.emplace(&*it, candidates);
    }

    // Tout le dictionnaire des coûts des candidates a été rempli, en
    // particulier pour le nœud racine. Identification de la synténie
    // candidate ayant le moindre coût pour la racine
    Event* root = &*std::begin(tree);
    auto best_candidate_cost = Cost::positiveInfinity();
    Synteny best_synteny;

    for (const auto& candidate_pair : candidates_per_node.at(root))
    {
        if (candidate_pair.second.cost < best_candidate_cost)
        {
            best_candidate_cost = candidate_pair.second.cost;
            best_synteny = candidate_pair.first;
        }
    }

    if (best_candidate_cost.isInfinity())
    {
        throw std::invalid_argument{"L’ordre donné dans la synténie "
            "ancestrale n’est pas cohérent avec l’affectation d’au "
            "moins une des feuilles."};
    }

    root->setSynteny(best_synteny);

    // Il ne reste plus qu’a propager en partant de la racine toutes
    // les meilleures synténies candidates
    for (auto it = tree.begin(); it != tree.end(); ++it)
    {
        if (tree.number_of_children(it) == 2)
        {
            Event* child_left = &*tree.child(it, 0);
            Event* child_right = &*tree.child(it, 1);

            child_left->setSynteny(candidates_per_node
                .at(&*it).at(it->getSynteny()).synteny_left);
            child_right->setSynteny(candidates_per_node
                .at(&*it).at(it->getSynteny()).synteny_right);
        }
    }

    return static_cast<int>(best_candidate_cost);
}

int main()
{
    std::string ancestral_data;
    std::getline(std::cin, ancestral_data);

    std::istringstream ancestral_tok{ancestral_data};
    std::istream_iterator<Gene> it{ancestral_tok};
    std::istream_iterator<Gene> end;

    Synteny ancestral(it, end);

    std::ostringstream tree_newick;
    tree_newick << std::cin.rdbuf();

    tree<Event> tree = newick_to_tree(tree_newick.str());
    small_philogeny_for_syntenies(tree, ancestral);

    print_tree_dot<Event>(tree, std::cout, [](std::ostream& out, const Event& event)
    {
        out << "shape=\"";

        switch (event.getType())
        {
        case Event::Type::None:
            out << "none";
            break;

        case Event::Type::Duplication:
            out << "box";
            break;

        case Event::Type::Speciation:
            out << "oval";
            break;

        case Event::Type::Loss:
            out << "diamond";
            break;
        }

        out << "\", label=\"" << event.getSynteny() << "\"";
    });
}
