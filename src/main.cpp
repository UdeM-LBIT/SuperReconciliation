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
        std::cerr << "Input the ancestral synteny followed "
            "by a line feed:\n";
    }

    std::string ancestral_data;
    std::getline(std::cin, ancestral_data);

    std::istringstream ancestral_tok{ancestral_data};
    std::istream_iterator<Gene> it{ancestral_tok};
    std::istream_iterator<Gene> end;
    Synteny ancestral(it, end);

    if (is_interactive())
    {
        std::cerr << "\nInput the tree to be reconciled and "
            "finish with Ctrl-D:\n";
    }

    std::ostringstream tree_string;
    tree_string << std::cin.rdbuf();

    tree<Event> tree = string_to_event_tree(tree_string.str());
    small_philogeny(tree, ancestral);

    if (is_interactive())
    {
        std::cerr << "\nReconciled tree (use `event_tree_viz` "
            "to visualize):\n";
    }

    std::cout << event_tree_to_string(tree) << '\n';
    return EXIT_SUCCESS;
}
