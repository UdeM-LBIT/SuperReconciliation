#include "erase.hpp"

void erase_tree(
    ::tree<Event> input,
    ::tree<Event>::sibling_iterator root,
    bool is_root)
{
    switch (root->type)
    {
    case Event::Type::None:
        return;

    case Event::Type::Loss:
    {
        root->synteny = Synteny{};

        if (input.number_of_children(root) != 0)
        {
            // Remove loss nodes, moving their only child up
            auto child = input.child(root, 0);
            input.flatten(root);
            input.erase(root);
            erase_tree(input, child, false);
        }

        return;
    }

    case Event::Type::Duplication:
    case Event::Type::Speciation:
        if (!is_root)
        {
            // Erase all syntenies in internal nodes except for the root node
            root->synteny = Synteny{};
        }

        erase_tree(input, input.child(root, 0), false);
        erase_tree(input, input.child(root, 1), false);
        return;
    }
}
