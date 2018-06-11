#include "EventTree.hpp"
#include <tree.hh>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/phoenix_function.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phx = boost::phoenix;
namespace phxarg = phx::placeholders;

/**
 * Given a root event and a list of subtrees, builds a parent tree.
 *
 * @param event Root event for the parent tree.
 * @param children List of subtrees to graft to the root.
 * @return Resulting parent tree.
 */
tree<Event> build_tree(Event event, std::vector<tree<Event>> children)
{
    tree<Event> result{event};

    for (const auto& subtree : children)
    {
        auto tmp = result.append_child(result.begin());
        result.move_ontop(tmp, subtree.begin());
    }

    return result;
}

/**
 * Pseudo-Newick tree grammar describing a language of synteny
 * trees labelled with events.
 */
template<typename Iterator>
struct EventTreeGrammar : qi::grammar<Iterator, ascii::space_type, ::tree<Event>()>
{
    EventTreeGrammar() : EventTreeGrammar::base_type(tree, "EventTreeGrammar")
    {
        // A tree is composed of parenthesized a root node and of a list of
        // child nodes
        tree = ('(' > event > children > ')')
                            [qi::_val = phx::bind(&build_tree, qi::_1, qi::_2)];

        // Those child nodes are actually trees themselves, and a node may have
        // no child. If so, it is called a leaf
        children = *tree;

        // The nodes of the tree are labelled with events that describe what
        // occurred at that point. The general format for an event is
        // `event_type : synteny`, where `event_type` describes what kind
        // of event it is and `synteny` is the ordered block of genes at that
        // point. Either `event_type` or `: synteny` needs to be present:
        //
        //  — If `event_type` is omitted, it is assumed that the node is a leaf
        //    and thus the event is set to type `Event::Type::None`.
        //  – If `: synteny` is omitted, the event is associated with an
        //    empty synteny (which means that the actual synteny is unknown).
        event = (type >> ':' > synteny)
                             [qi::_val = phx::construct<Event>(qi::_1, qi::_2)]
            | (type)      [qi::_val = phx::construct<Event>(qi::_1, Synteny{})]
            | (':' > synteny)
                  [qi::_val = phx::construct<Event>(Event::Type::None, qi::_1)];

        type.add
            ("dupl", Event::Type::Duplication)
            ("spec", Event::Type::Speciation)
            ("loss", Event::Type::Loss);

        // A synteny is an ordered block of genes
        synteny = gene > *gene;

        // A gene name is a string of any characters, except whitespace,
        // parentheses or colons which are used to delimit genes
        gene = qi::lexeme[+(qi::char_ - qi::char_(":() \t\r\n"))];

        tree.name("tree");
        children.name("children");
        event.name("event");
        type.name("type");
        synteny.name("synteny");
        gene.name("gene");
    }

    qi::rule<Iterator, ascii::space_type, ::tree<Event>()> tree;
    qi::rule<Iterator, ascii::space_type, std::vector<::tree<Event>>()> children;
    qi::rule<Iterator, ascii::space_type, Event()> event;
    qi::symbols<char, Event::Type> type;
    qi::rule<Iterator, ascii::space_type, Synteny()> synteny;
    qi::rule<Iterator, ascii::space_type, Gene()> gene;
};

tree<Event> string_to_event_tree(const std::string& input)
{
    tree<Event> result;
    EventTreeGrammar<std::string::const_iterator> grammar;

    auto start = std::begin(input);
    auto end = std::end(input);

    qi::phrase_parse(start, end, grammar, ascii::space, result);
    return result;
}

std::string event_to_string(const Event& event)
{
    std::string result;

    switch (event.getType())
    {
    case Event::Type::None:
        // no-op
        break;

    case Event::Type::Duplication:
        result += "dupl ";
        break;

    case Event::Type::Speciation:
        result += "spec ";
        break;

    case Event::Type::Loss:
        result += "loss ";
        break;
    }

    if (!event.getSynteny().empty())
    {
        result += ": ";
        std::ostringstream synteny;
        synteny << event.getSynteny();
        result += synteny.str();
    }

    return result;
}

std::string event_subtree_to_string(
    const tree<Event>& tree,
    typename ::tree<Event>::iterator root)
{
    std::string result;
    result += "(" + event_to_string(*root);

    for (typename ::tree<Event>::sibling_iterator it = tree.begin(root);
            it != tree.end(root); ++it)
    {
        result += " " + event_subtree_to_string(tree, it);
    }

    result += ")";
    return result;
}

std::string event_tree_to_string(const tree<Event>& tree)
{
    return event_subtree_to_string(tree, std::begin(tree));
}

std::string event_to_graphviz(const Event& event)
{
    std::string result = "shape=\"";

    switch (event.getType())
    {
    case Event::Type::None:
    case Event::Type::Loss:
        result += "none";
        break;

    case Event::Type::Duplication:
        result += "box";
        break;

    case Event::Type::Speciation:
        result += "oval";
        break;
    }

    result += "\", label=\"";
    std::ostringstream synteny;
    synteny << event.getSynteny();
    result += synteny.str() + "\"";

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

    for (const auto& node : tree)
    {
        result += "    "
            + std::to_string(reinterpret_cast<unsigned long long int>(&node))
            + " [" + event_to_graphviz(node) + "];\n";
    }

    for (typename ::tree<Event>::sibling_iterator it = tree.begin();
            it != tree.end(); ++it)
    {
        result += event_subtree_to_graphviz(tree, it);
    }

    result += "}\n";
    return result;
}
