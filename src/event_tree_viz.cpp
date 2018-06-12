#include "EventTree.hpp"
#include "util.hpp"
#include <cstdlib>
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

    if (is_interactive())
    {
        std::cerr << "\nTree in Graphviz format (can be piped "
            "into `dot`):\n";
    }

    tree<Event> tree = string_to_event_tree(tree_string.str());
    std::cout << event_tree_to_graphviz(tree);
    return EXIT_SUCCESS;
}
