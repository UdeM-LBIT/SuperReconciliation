#include "small_philogeny.hpp"
#include "tree_parser.hpp"
#include "util.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>

int main()
{
    if (is_interactive())
    {
        std::cerr << "\nInput the tree to be reconciled and "
            "finish with Ctrl-D:\n";
    }

    std::ostringstream tree_string;
    tree_string << std::cin.rdbuf();

    tree<Event> tree = string_to_event_tree(tree_string.str());
    auto total_cost = small_philogeny(tree);

    std::cerr << "\nLabeled tree with cost " << total_cost
        << " (use `viz` to visualize):\n";
    std::cout << event_tree_to_string(tree) << '\n';

    return EXIT_SUCCESS;
}
