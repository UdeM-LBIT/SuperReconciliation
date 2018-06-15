#include "io/nhx_parser.hpp"
#include "io/util.hpp"
#include "util/tree.hpp"
#include "model/Synteny.hpp"
#include "model/Event.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>

std::string event_to_graphviz(const Event& event, Synteny parent)
{
    std::string result;

    switch (event.type)
    {
    case Event::Type::None:
        result += "shape=\"none\", ";
        break;

    case Event::Type::Loss:
        result += "shape=\"none\", fontcolor=\"red\", ";
        break;

    case Event::Type::Duplication:
        result += "shape=\"box\", ";
        break;

    case Event::Type::Speciation:
        result += "shape=\"oval\", ";
        break;
    }

    result += "label=\"";

    if (event.type == Event::Type::Loss)
    {
        result += parent.difference(event.synteny);
    }
    else
    {
        result += event.synteny.difference(event.synteny);
    }

    result += "\"";
    return result;
}

std::string event_subtree_to_graphviz(
    const tree<Event>& tree,
    typename ::tree<Event>::iterator root)
{
    std::string result;

    for (typename ::tree<Event>::sibling_iterator it = tree.begin(root);
            it != tree.end(root); ++it)
    {
        result += "    "
            + std::to_string(reinterpret_cast<unsigned long long int>(&*root))
            + " -- "
            + std::to_string(reinterpret_cast<unsigned long long int>(&*it))
            + ";\n";
        result += event_subtree_to_graphviz(tree, it);
    }

    return result;
}

std::string event_tree_to_graphviz(const tree<Event>& tree)
{
    std::string result = "graph {\n";

    for (
        auto it = tree.begin();
        it != tree.end();
        ++it)
    {
        auto parent_node = tree.parent(it);
        Synteny parent_synteny = it->synteny;

        // If we are considering the root of the tree, it has no parent node
        if (tree.is_valid(parent_node))
        {
            parent_synteny = parent_node->synteny;
        }

        // We use a node’s address in memory as a unique identifier
        result += "    "
            + std::to_string(reinterpret_cast<unsigned long long int>(&*it))
            + " [" + event_to_graphviz(*it, parent_synteny) + "];\n";
    }

    for (typename ::tree<Event>::sibling_iterator it = tree.begin();
            it != tree.end(); ++it)
    {
        result += event_subtree_to_graphviz(tree, it);
    }

    result += "}\n";
    return result;
}

int main()
{
    if (is_interactive())
    {
        std::cerr << "Input the tree to be visualized and "
            "finish with Ctrl-D:\n";
    }

    std::ostringstream nhx_tree;
    nhx_tree << std::cin.rdbuf();

    auto input_tree = parse_nhx_tree(nhx_tree.str());
    auto event_tree = tree_cast<TaggedNode, Event>(input_tree);

    std::cerr << "Tree in Graphviz format (can be piped into `dot`):\n";
    std::cout << event_tree_to_graphviz(event_tree);

    return EXIT_SUCCESS;
}
