#ifndef EXTENDED_NUMBER_HPP
#define EXTENDED_NUMBER_HPP

#include <ostream>

/**
 * Type nombre, étendu avec les infinis positif et négatif. Ce type gère
 * de façon appropriée les opérations impliquant des infinis. Des exceptions
 * de type std::domain_error sont levées pour les opérations indéfinies.
 */
template<typename T>
class ExtendedNumber
{
public:
    /**
     * Instancie un nombre étendu avec pour valeur 0.
     */
    ExtendedNumber();

    /**
     * Instancie un nombre étendu avec une valeur donnée.
     *
     * @param value Valeur à donner au nombre.
     */
    ExtendedNumber(const T&);

    /**
     * Instancie l’infini positif.
     *
     * @return Instance représentant l’infini positif.
     */
    static ExtendedNumber positiveInfinity() noexcept;

    /**
     * Instancie l’infini négatif.
     *
     * @return Instance représentant l’infini négatif.
     */
    static ExtendedNumber negativeInfinity() noexcept;

    /**
     * Vérifie si l’instance actuelle est l’infini positif.
     *
     * @return Vrai si et seulement si l’instance actuelle est l’infini
     * positif.
     */
    bool isPositiveInfinity() const noexcept;

    /**
     * Vérifie si l’instance actuelle est l’infini négatif.
     *
     * @return Vrai si et seulement si l’instance actuelle est l’infini
     * négatif.
     */
    bool isNegativeInfinity() const noexcept;

    /**
     * Vérifie si l’instance actuelle code un infini.
     *
     * @return Vrai si et seulement si l’instance actuelle code un infini.
     */
    bool isInfinity() const noexcept;

    /**
     * Convertit explicitement un nombre étendu en son type natif.
     *
     * @throws std::domain_error Si le nombre étendu code un infini.
     */
    explicit operator T() const;

    // Opérateurs de comparaison entre nombres étendus
    bool operator<(const ExtendedNumber&) const noexcept;
    bool operator==(const ExtendedNumber&) const noexcept;
    bool operator!=(const ExtendedNumber&) const noexcept;
    bool operator<=(const ExtendedNumber&) const noexcept;
    bool operator>(const ExtendedNumber&) const noexcept;
    bool operator>=(const ExtendedNumber&) const noexcept;

    // Plus et moins unaires
    ExtendedNumber operator+() const noexcept;
    ExtendedNumber operator-() const noexcept;

    /**
     * Ajoute au nombre actuel un autre nombre étendu et stocke le résultat
     * dans le nombre actuel.
     *
     * @param rhs Second opérande de l’opération.
     * @throws std::domain_error Pour l’ajout de deux infinis opposés.
     * @return Instance actuelle.
     */
    ExtendedNumber& operator+=(const ExtendedNumber&);
    ExtendedNumber operator+(const ExtendedNumber&) const;

    /**
     * Retranche au nombre actuel un autre nombre étendu et stocke le résultat
     * dans le nombre actuel.
     *
     * @param rhs Second opérande de l’opération.
     * @throws std::domain_error Pour la soustraction de deux infinis.
     * @return Instance actuelle.
     */
    ExtendedNumber& operator-=(const ExtendedNumber&);
    ExtendedNumber operator-(const ExtendedNumber&) const;

    /**
     * Multiplie le nombre actuel par un autre nombre étendu et stocke le
     * résultat dans le nombre actuel.
     *
     * @param rhs Second opérande de l’opération.
     * @throws std::domain_error Pour la multiplication d’un infini par zéro.
     * @return Instance actuelle.
     */
    ExtendedNumber& operator*=(const ExtendedNumber&);
    ExtendedNumber operator*(const ExtendedNumber&) const;

    /**
     * Divise le nombre actuel par un autre nombre étendu et stocke le résultat
     * dans le nombre actuel.
     *
     * @param rhs Second opérande de l’opération.
     * @throws std::domain_error Pour la division par zéro.
     * @return Instance actuelle.
     */
    ExtendedNumber& operator/=(const ExtendedNumber&);
    ExtendedNumber operator/(const ExtendedNumber&) const;

private:
    // Valeur du nombre. S’il s’agit d’un infini, seul le bit de signe
    // est significatif et indique le signe de l’infini.
    T value;

    // Vrai si et seulement si l’instance code un infini, et dans ce cas
    // le signe de l’infini est le signe de `value`.
    bool infinity_flag = false;
};

template<typename T>
bool operator<(const T&, const ExtendedNumber<T>&) noexcept;

template<typename T>
bool operator==(const T&, const ExtendedNumber<T>&) noexcept;

template<typename T>
bool operator!=(const T&, const ExtendedNumber<T>&) noexcept;

template<typename T>
bool operator<=(const T&, const ExtendedNumber<T>&) noexcept;

template<typename T>
bool operator>(const T&, const ExtendedNumber<T>&) noexcept;

template<typename T>
bool operator>=(const T&, const ExtendedNumber<T>&) noexcept;

template<typename T>
ExtendedNumber<T> operator+(const T&, const ExtendedNumber<T>&);

template<typename T>
ExtendedNumber<T> operator-(const T&, const ExtendedNumber<T>&);

template<typename T>
ExtendedNumber<T> operator*(const T&, const ExtendedNumber<T>&);

template<typename T>
ExtendedNumber<T> operator/(const T&, const ExtendedNumber<T>&);

/**
 * Affiche un nombre étendu sur un flux de sortie.
 *
 * @param out Flux de sortie à utiliser.
 * @param number Nombre étendu à afficher.
 * @return Flux de sortie utilisé.
 */
template<typename T>
std::ostream& operator<<(std::ostream&, const ExtendedNumber<T>&);

#include "ExtendedNumber.tpp"

#endif // EXTENDED_NUMBER_HPP
