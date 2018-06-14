#include "super_reconciliation.hpp"
#include "io/tree_parser.hpp"
#include "io/util.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>

int main()
{
    if (is_interactive())
    {
        std::cerr << "Input the tree to be reconciled and "
            "finish with Ctrl-D:\n";
    }

    std::ostringstream tree_string;
    tree_string << std::cin.rdbuf();

    tree<Event> tree = string_to_event_tree(tree_string.str());
    auto cost = super_reconciliation(tree);

    std::cerr << "\nReconciled tree with cost " << cost
        << " (use `viz` to visualize):\n";
    std::cout << event_tree_to_string(tree) << '\n';

    return EXIT_SUCCESS;
}
