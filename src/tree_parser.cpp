#include "tree_parser.hpp"
#include "Event.hpp"
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

template<typename Iterator>
struct NewickTreeGrammar : qi::grammar<Iterator, ascii::space_type, ::tree<Event>()>
{
    NewickTreeGrammar() : NewickTreeGrammar::base_type(tree, "NewickTreeGrammar")
    {
        tree = ('(' > event > children > ')')
            [qi::_val = phx::bind(&build_tree, qi::_1, qi::_2)];

        children = *tree;
        event =
            (type >> ':' > synteny)
                [qi::_val = phx::construct<Event>(qi::_1, qi::_2)]
            | (type)[qi::_val = phx::construct<Event>(qi::_1)];

        type.add
            ("leaf", Event::Type::None)
            ("dupl", Event::Type::Duplication)
            ("spec", Event::Type::Speciation)
            ("loss", Event::Type::Loss);

        synteny = gene > *gene;
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

tree<Event> newick_to_tree(const std::string& input)
{
    tree<Event> result;
    NewickTreeGrammar<std::string::const_iterator> grammar;

    auto start = std::begin(input);
    auto end = std::end(input);

    qi::phrase_parse(start, end, grammar, ascii::space, result);
    return result;
}
