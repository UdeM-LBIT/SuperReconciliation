#include "nhx_parser.hpp"
#include <tree.hh>
#include <boost/fusion/include/std_pair.hpp>
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
 * Given a root node and a list of subtrees, builds a parent tree.
 *
 * @param event Root node for the parent tree.
 * @param children Optional list of subtrees to graft to the root.
 * @return Resulting parent tree.
 */
template<typename Node>
::tree<Node> build_tree(
    Node node,
    boost::optional<std::vector<::tree<Node>>> children)
{
    ::tree<Node> result{node};

    if (children)
    {
        for (const auto& subtree : *children)
        {
            result.append_child(result.begin(), subtree.begin());
        }
    }

    return result;
}

// This is to circumvent not being able to put commas
// inside a macro invocation (below)
using MapStringString = std::map<std::string, std::string>;

// Make Boost see the TaggedNode structure as a tuple
BOOST_FUSION_ADAPT_STRUCT(
    TaggedNode,
    (std::string, name)
    (double, length)
    (MapStringString, tags)
);

/**
 * Grammar for lexemes that must be skipped while parsing a NHX tree.
 */
template<typename Iterator>
struct NHXSkipGrammar : qi::grammar<Iterator>
{
    NHXSkipGrammar() : NHXSkipGrammar::base_type(skip, "NHXSkipGrammar")
    {
        skip = whitespace | comment;
        whitespace = ascii::space;

        // Comments are enclosed by square brackets and must not start
        // with the NHX start sequence that is reserved for custom
        // tag lists
        comment = open_bracket >> !nhx_start
            >> qi::omit[*(qi::char_ - qi::char_(']'))]
            >> close_bracket;

        nhx_start = qi::lit("&&NHX");
        open_bracket = qi::lit('[');
        close_bracket = qi::lit(']');

        skip.name("skip");
        whitespace.name("whitespace");
        comment.name("comment");

        nhx_start.name("&&NHX");
        open_bracket.name("[");
        close_bracket.name("]");
    }

    qi::rule<Iterator> skip;
    qi::rule<Iterator> whitespace;
    qi::rule<Iterator> comment;

    qi::rule<Iterator> nhx_start;
    qi::rule<Iterator> open_bracket;
    qi::rule<Iterator> close_bracket;
};

/**
 * Grammar for NHX trees defined using Boost Spirit.
 */
template<typename Iterator, typename Skipper = NHXSkipGrammar<Iterator>>
struct NHXGrammar : qi::grammar<Iterator, Skipper, ::tree<TaggedNode>()>
{
    NHXGrammar() : NHXGrammar::base_type(tree, "NHXGrammar")
    {
        // This rule forces the root to be followed by a semicolon. But
        // otherwise, the root node has the exact same syntax as others
        tree = subtree > semicolon;

        // A subtree is composed of an optional list of child subtrees
        // (omitted for leaf nodes) and of a root node, in this order
        subtree = (-children >> node)
                [qi::_val = phx::bind(&build_tree<TaggedNode>, qi::_2, qi::_1)];

        // The child subtrees are separated by a comma and enclosed
        // by parentheses
        children = open_paren > subtree > *(comma > subtree) > close_paren;

        // A node is composed of a name, a length tag and a list of
        // custom tags. Each of those components is optional but the
        // order must be respected
        node = (name | qi::attr(std::string{}))
            >> (length | qi::attr(0.))
            >> (tagmap | qi::attr(std::map<std::string, std::string>{}));

        name = ident.alias();
        length = colon > qi::double_;

        // The custom tag list contains attributes that can be attached
        // to any node in the tree. Each tag is stored in a :key=value format
        tagmap = nhx_start > +tag > nhx_end;
        tag = colon > ident > equals > ident;

        // Identifiers can either by unquoted or quoted. Unquoted identifiers
        // may not contain the following chars: (, ), [, ], ',', :, ;, = or
        // whitespace. Quoted identifiers may contain any char, but double
        // quotes must be double-escaped
        ident = quoted_string | unquoted_string;
        unquoted_string
            = qi::lexeme[+(qi::char_ - qi::char_("()[],:;= \t\r\n"))];
        quoted_string = quote > qi::no_skip[*(
            (qi::lit('"') >> qi::char_('"')) // escaped double quotes
            | (qi::char_ - qi::char_('"')) // normal chars
        )] > quote;

        // Definition of literal symbols
        semicolon = qi::lit(';');
        open_paren = qi::lit('(');
        close_paren = qi::lit(')');
        comma = qi::lit(',');
        nhx_start = qi::lit("[&&NHX");
        nhx_end = qi::lit(']');
        colon = qi::lit(':');
        equals = qi::lit('=');
        quote = qi::lit('"');

        // Naming of all rules to ease debugging
        tree.name("tree");
        subtree.name("subtree");
        children.name("children");
        node.name("node");
        name.name("name");
        length.name("length");
        tagmap.name("tagmap");
        tag.name("tag");
        ident.name("ident");
        unquoted_string.name("unquoted_string");
        quoted_string.name("quoted_string");

        semicolon.name(";");
        open_paren.name("(");
        close_paren.name(")");
        comma.name(",");
        nhx_start.name("[&&NHX;");
        nhx_end.name("]");
        colon.name(":");
        equals.name("=");
        quote.name("\"");
    }

    template<typename Productions>
    using NaryRule = qi::rule<Iterator, Skipper, Productions>;

    NaryRule<::tree<TaggedNode>()> tree;
    NaryRule<::tree<TaggedNode>()> subtree;
    NaryRule<std::vector<::tree<TaggedNode>>()> children;
    NaryRule<TaggedNode()> node;
    NaryRule<std::string()> name;
    NaryRule<double()> length;
    NaryRule<std::map<std::string, std::string>> tagmap;
    NaryRule<std::pair<std::string, std::string>> tag;
    NaryRule<std::string()> ident;
    NaryRule<std::string()> unquoted_string;
    NaryRule<std::string()> quoted_string;

    using NullaryRule = qi::rule<Iterator, Skipper>;

    NullaryRule semicolon;
    NullaryRule open_paren;
    NullaryRule close_paren;
    NullaryRule comma;
    NullaryRule nhx_start;
    NullaryRule nhx_end;
    NullaryRule colon;
    NullaryRule equals;
    NullaryRule quote;
};

tree<TaggedNode> parse_nhx_tree(const std::string& input)
{
    using Iterator = std::string::const_iterator;

    tree<TaggedNode> result;
    NHXGrammar<Iterator> grammar;
    NHXSkipGrammar<Iterator> skip;

    auto start = std::begin(input);
    auto end = std::end(input);
    bool success = false;

    try
    {
        success = qi::phrase_parse(start, end, grammar, skip, result);
    }
    catch (const qi::expectation_failure<Iterator>& err)
    {
        std::ostringstream message;
        message << "Syntax error: expected '" << err.what_.tag << "' at"
            " character "
            << std::to_string(std::distance(std::cbegin(input), err.first))
            << " but found ";

        if (err.first == std::cend(input))
        {
            message << "<end>";
        }
        else
        {
            std::string sub(err.first, err.last);
            message << "'" << sub << "'";
        }

        throw std::invalid_argument{message.str()};
    }

    if (!success)
    {
        throw std::invalid_argument{"Syntax error: cannot parse input"};
    }

    if (start != end)
    {
        std::ostringstream message;
        std::string sub(start, end);

        message << "Syntax error: expected <end> at character "
            << std::to_string(std::distance(std::cbegin(input), start))
            << " but found \"" << sub << "\"";

        throw std::invalid_argument{message.str()};
    }

    return result;
}

/**
 * Helper function to espace a NHX identifier string if need be. Only
 * strings that contain (, ), [, ], ',', :, ;, = or whitespace need to
 * be escaped.
 *
 * @param source Original, unescaped string.
 *
 * @return Escaped version.
 */
std::string nhx_escape_ident(std::string source)
{
    if (source.find_first_of("()[],:;= \t\r\n") == std::string::npos)
    {
        // Nothing to be escaped
        return source;
    }

    std::string result = "\"";
    result.reserve(source.size() * 2 + 2);

    for (const auto& chr : source)
    {
        if (chr == '"')
        {
            // Escape contained double quotes as they occur
            result += "\"\"";
        }
        else
        {
            result += chr;
        }
    }

    result += "\"";
    return result;
}

/**
 * Convert a subtree to NHX format.
 *
 * @param tree Tree containing the subtree to be converted.
 * @param root Iterator to the root of the subtree.
 *
 * @return NHX representation for the subtree.
 */
std::string stringify_nhx_tree_helper(
    const ::tree<TaggedNode>& tree,
    typename ::tree<TaggedNode>::iterator root)
{
    std::string result;

    if (tree.number_of_children(root) > 0)
    {
        result += "(";

        for (auto it = tree.begin(root); it != tree.end(root); ++it)
        {
            result += stringify_nhx_tree_helper(tree, it);

            if (std::next(it) != tree.end(root))
            {
                result += ",";
            }
        }

        result += ")";
    }

    const TaggedNode& node = *root;

    if (!node.name.empty())
    {
        result += nhx_escape_ident(node.name);
    }

    if (node.length != 0.)
    {
        result += ":" + std::to_string(node.length);
    }

    if (!node.tags.empty())
    {
        result += "[&&NHX";

        for (const auto& [key, value] : node.tags)
        {
            result += ":" + nhx_escape_ident(key)
                + "=" + nhx_escape_ident(value);
        }

        result += "]";
    }

    return result;
}

std::string stringify_nhx_tree(const ::tree<TaggedNode>& tree)
{
    return stringify_nhx_tree_helper(tree, std::begin(tree)) + ";";
}
