#include "algo/super_reconciliation.hpp"
#include "io/nhx_parser.hpp"
#include "io/util.hpp"
#include "util/tree.hpp"
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

    std::ostringstream nhx_tree;
    nhx_tree << std::cin.rdbuf();

    auto input_tree = parse_nhx_tree(nhx_tree.str());
    auto event_tree = tree_cast<TaggedNode, Event>(input_tree);
    auto cost = super_reconciliation(event_tree);
    auto result_tree = tree_cast<Event, TaggedNode>(event_tree);

    std::cerr << "Reconciled tree with cost " << cost
        << " (use `viz` to visualize):\n";
    std::cout << stringify_nhx_tree(result_tree) << '\n';

    return EXIT_SUCCESS;
}
