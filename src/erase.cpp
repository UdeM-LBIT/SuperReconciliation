#include "algo/erase.hpp"
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
        std::cerr << "Input the tree to be erased and "
            "finish with Ctrl-D:\n";
    }

    std::ostringstream nhx_tree;
    nhx_tree << std::cin.rdbuf();

    auto input_tree = parse_nhx_tree(nhx_tree.str());
    auto event_tree = tree_cast<TaggedNode, Event>(input_tree);
    erase_tree(event_tree, std::begin(event_tree));
    auto out_erased_tree = tree_cast<Event, TaggedNode>(event_tree);

    std::cerr << "Erased tree:\n";
    std::cout << stringify_nhx_tree(out_erased_tree);

    return EXIT_SUCCESS;
}
