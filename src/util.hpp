#include <functional>
#include <tree.hh>

namespace detail
{
    template<typename T>
    void _print_tree_dot_rec(
        const tree<T>& tree,
        typename ::tree<T>::iterator root,
        std::ostream& out)
    {
        for (typename ::tree<T>::sibling_iterator it = tree.begin(root);
                it != tree.end(root); ++it)
        {
            out << "    "
                << reinterpret_cast<unsigned long long int>(&*root)
                << " -> "
                << reinterpret_cast<unsigned long long int>(&*it) << ";\n";

            _print_tree_dot_rec(tree, it, out);
        }
    }
}

template<typename T>
void print_tree_dot(const tree<T>& tree, std::ostream& out, std::function<void(std::ostream&, const T&)> printer)
{
    out << "digraph {\n";

    // Chaque nœud de l’arbre est identifié par son adresse mémoire.
    // Pour l’affichage, on indique à GraphViz l’étiquette associée
    // à chaque nœud
    for (const auto& node : tree)
    {
        out << "    "
            << reinterpret_cast<unsigned long long int>(&node)
            << " [";
        printer(out, node);
        out << "];\n";
    }

    // Traversée de l’arbre en largeur, en marquant chaque arête dans
    // la sortie
    for (typename ::tree<T>::sibling_iterator it = tree.begin();
            it != tree.end(); ++it)
    {
        detail::_print_tree_dot_rec<T>(tree, it, out);
    }

    out << "}\n";
}

