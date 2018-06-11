#include "EventTree.hpp"
#include <cstdlib>
#include <sstream>

int main()
{
    std::ostringstream tree_string;
    tree_string << std::cin.rdbuf();

    tree<Event> tree = string_to_event_tree(tree_string.str());
    std::cout << event_tree_to_graphviz(tree);

    return EXIT_SUCCESS;
}
