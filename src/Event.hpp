#ifndef EVENT_HPP
#define EVENT_HPP

#include "Synteny.hpp"

/**
 * Événement s’étant produit au niveau d’un nœud d’un super-arbre de synténies.
 */
class Event
{
public:
    /**
     * Type d’événement.
     */
    enum class Type
    {
        // Aucun événement : il s’agit d’un nœud feuille
        None,

        // Événement de duplication segmentale de la synténie
        Duplication,

        // Événement de spéciation de la synténie
        Speciation,

        // Événement de perte segmentale de la synténie
        Loss
    };

    /**
     * Instancie un événement de type « aucun » sur une synténie vide.
     */
    Event();

    /**
     * Instancie un événement de type « aucun » sur une synténie.
     *
     * @param synteny Synténie de la feuille en question.
     */
    Event(Synteny);

    /**
     * Instancie un événement sur une synténie vide.
     *
     * @param type Type de l’événement à instancier.
     */
    Event(Type);

    /**
     * Instancie un événement.
     *
     * @param type Type de l’événement à instancier.
     * @param synteny Synténie sur laquelle s’est produit l’événement.
     */
    Event(Type, Synteny);

    // Accesseurs
    Type getType() const noexcept;
    const Synteny& getSynteny() const noexcept;
    void setSynteny(const Synteny&) noexcept;

private:
    Type type;
    Synteny synteny;
};

#endif // EVENT_HPP
