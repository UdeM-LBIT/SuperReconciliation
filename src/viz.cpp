#include "io/tree_parser.hpp"
#include "io/util.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>

int main()
{
    if (is_interactive())
    {
        std::cerr << "Input the tree to be visualized and "
            "finish with Ctrl-D:\n";
    }

    std::ostringstream tree_string;
    tree_string << std::cin.rdbuf();

    tree<Event> tree = string_to_event_tree(tree_string.str());

    std::cerr << "\nTree in Graphviz format (can be piped into `dot`):\n";
    std::cout << event_tree_to_graphviz(tree);

    return EXIT_SUCCESS;
}
