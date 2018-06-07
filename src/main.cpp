#include "Event.hpp"
#include "ExtendedNumber.hpp"
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
    // TODO: passer l’arbre en input

    tree<Event> test_tree{Event{Event::Type::Speciation}};
    auto node = std::begin(test_tree);

    auto node_0 = test_tree.append_child(node, Event{Event::Type::Speciation});
    auto node_00 = test_tree.append_child(node_0, Event{Synteny{"a", "d"}});
    auto node_01 = test_tree.append_child(node_0, Event{Event::Type::Duplication});
    auto node_010 = test_tree.append_child(node_01, Event{Synteny{"a", "b", "c"}});
    auto node_011 = test_tree.append_child(node_01, Event{Synteny{"a", "b", "d"}});
    auto node_1 = test_tree.append_child(node, Event{Event::Type::Duplication});
    auto node_10 = test_tree.append_child(node_1, Event{Event::Type::Speciation});
    auto node_100 = test_tree.append_child(node_10, Event{Synteny{"a", "b", "c", "d"}});
    auto node_101 = test_tree.append_child(node_10, Event{Synteny{"a", "b", "c"}});
    auto node_11 = test_tree.append_child(node_1, Event{Event::Type::Speciation});
    auto node_110 = test_tree.append_child(node_11, Event{Synteny{"b", "c", "d"}});
    auto node_111 = test_tree.append_child(node_11, Event{Event::Type::Speciation});
    auto node_1110 = test_tree.append_child(node_111, Event{Synteny{"b", "d"}});
    auto node_1111 = test_tree.append_child(node_111, Event{Synteny{"d"}});

    small_philogeny_for_syntenies(test_tree, Synteny{"a", "b", "c", "d"});

    print_tree_dot<Event>(test_tree, std::cout, [](std::ostream& out, const Event& event)
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

    /* tree<Event> simple_tree{Event{Event::Type::Duplication, Synteny{"x", "x'", "x''"}}}; */

    /**
     * (d (n : x x' x'') (s (n : x) (d (n : x x'') (n : x x')))
     */

    /* { */
    /*     auto node = std::begin(simple_tree); */
    /*     auto node_0 = simple_tree.append_child(node, Event{Synteny{"x", "x'", "x''"}}); */
    /*     auto node_1 = simple_tree.append_child(node, Event{Event::Type::Speciation}); */
    /*     auto node_10 = simple_tree.append_child(node_1, Event{Synteny{"x"}}); */
    /*     auto node_11 = simple_tree.append_child(node_1, Event{Event::Type::Duplication}); */
    /*     auto node_110 = simple_tree.append_child(node_11, Event{Synteny{"x", "x''"}}); */
    /*     auto node_111 = simple_tree.append_child(node_11, Event{Synteny{"x", "x'"}}); */
    /*     super_reconciliation(simple_tree); */
    /* } */

    // Ne fonctionne pas car ordre incohérent
    /* tree<Event> applic_tree{Event{Event::Type::Speciation, Synteny{"h", "s", "a", "n"}}}; */

    /* { */
    /*     auto node = std::begin(applic_tree); */
    /*     auto node_0 = applic_tree.append_child(node, Event{Event::Type::Duplication}); */
    /*     auto node_00 = applic_tree.append_child(node_0, Event{Event::Type::Duplication}); */
    /*     auto node_000 = applic_tree.append_child(node_00, Event{Event::Type::Duplication}); */
    /*     auto node_0000 = applic_tree.append_child(node_000, Event{Event::Type::Speciation}); */
    /*     auto node_00000 = applic_tree.append_child(node_0000, Event{Event::Type::Speciation}); */
    /*     auto node_000000 = applic_tree.append_child(node_00000, Event{Event::Type::Speciation}); */
    /*     auto node_0000000 = applic_tree.append_child(node_000000, Event{Synteny{"s", "a", "n", "h"}}); */
    /*     auto node_0000001 = applic_tree.append_child(node_000000, Event{Synteny{"h", "n", "a", "s"}}); */
    /*     auto node_000001 = applic_tree.append_child(node_00000, Event{Synteny{"a", "s", "h", "n"}}); */
    /*     auto node_00001 = applic_tree.append_child(node_0000, Event{Synteny{"h", "s", "a", "n"}}); */
    /*     auto node_0001 = applic_tree.append_child(node_000, Event{Event::Type::Speciation}); */
    /*     auto node_00010 = applic_tree.append_child(node_0001, Event{Event::Type::Speciation}); */
    /*     auto node_000100 = applic_tree.append_child(node_00010, Event{Synteny{"a"}}); */
    /*     auto node_000101 = applic_tree.append_child(node_00010, Event{Synteny{"n"}}); */
    /*     auto node_00011 = applic_tree.append_child(node_0001, Event{Event::Type::Speciation}); */
    /*     auto node_000110 = applic_tree.append_child(node_00011, Event{Synteny{"a", "n"}}); */
    /*     auto node_000111 = applic_tree.append_child(node_00011, Event{Event::Type::Speciation}); */
    /*     auto node_0001110 = applic_tree.append_child(node_000111, Event{Synteny{"a", "n"}}); */
    /*     auto node_0001111 = applic_tree.append_child(node_000111, Event{Synteny{"n", "a"}}); */
    /*     auto node_001 = applic_tree.append_child(node_00, Event{Event::Type::Speciation}); */
    /*     auto node_0010 = applic_tree.append_child(node_001, Event{Synteny{"n", "h", "a"}}); */
    /*     auto node_0011 = applic_tree.append_child(node_001, Event{Event::Type::Speciation}); */
    /*     auto node_00110 = applic_tree.append_child(node_0011, Event{Synteny{"a", "n", "h"}}); */
    /*     auto node_00111 = applic_tree.append_child(node_0011, Event{Event::Type::Speciation}); */
    /*     auto node_001110 = applic_tree.append_child(node_00111, Event{Synteny{"a", "h", "n"}}); */
    /*     auto node_001111 = applic_tree.append_child(node_00111, Event{Synteny{"a"}}); */
    /*     auto node_01 = applic_tree.append_child(node_0, Event{Event::Type::Speciation}); */
    /*     auto node_010 = applic_tree.append_child(node_01, Event{Synteny{"s", "n", "a", "h"}}); */
    /*     auto node_011 = applic_tree.append_child(node_01, Event{Event::Type::Speciation}); */
    /*     auto node_0110 = applic_tree.append_child(node_011, Event{Synteny{"n", "a", "s", "h"}}); */
    /*     auto node_0111 = applic_tree.append_child(node_011, Event{Event::Type::Speciation}); */
    /*     auto node_01110 = applic_tree.append_child(node_0111, Event{Synteny{"h", "n", "s", "a"}}); */
    /*     auto node_01111 = applic_tree.append_child(node_0111, Event{Synteny{"h", "n", "s", "a"}}); */
    /*     auto node_1 = applic_tree.append_child(node, Event{Event::Type::Speciation}); */
    /*     auto node_10 = applic_tree.append_child(node_1, Event{Synteny{"n"}}); */
    /*     auto node_11 = applic_tree.append_child(node_1, Event{Synteny{"h", "a"}}); */
    /*     super_reconciliation(applic_tree); */
    /* } */
}
