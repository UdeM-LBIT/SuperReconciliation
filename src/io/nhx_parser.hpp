#ifndef IO_NHX_PARSER_HPP
#define IO_NHX_PARSER_HPP

#include <map>
#include <string>
#include <tree.hh>

/**
 * Contain data for a tagged node in a tree, parsed from a NHX-formatted
 * input string.
 */
struct TaggedNode
{
    // Name of the node
    std::string name;

    // Length of the branch
    double length = 0.;

    // Custom tags attached to the node
    std::map<std::string, std::string> tags;
};

/**
 * Parse a NHX-formatted input string of characters into a tree of
 * tagged nodes. The grammar is:
 *
 * tree ::= subtree ';'
 * subtree ::= children? node
 * children ::= '(' subtree (',' subtree)* ')'
 * node ::= name? length? tagmap?
 * name ::= ident
 * length ::= ':' <double>
 * tagmap ::= '[&&NHX' tag+ ']'
 * tag ::= ':' ident '=' ident
 * ident ::= quoted_string | unquoted_string
 * quoted_string ::= '"' ('""' | [^"])* '"'
 * unquoted_string ::= [^()[],:;= \t\r\n]+
 *
 * (Derived from https://home.cc.umanitoba.ca/~psgendb/doc/atv/NHX.pdf)
 *
 * Whitespace and comments (enclosed by square brackets) are ignored
 * during parsing.
 *
 * @param input Input string to parse.
 *
 * @throws std::invalid_argument If the input string does not conform to the
 * above syntax.
 * @return Resulting labeled synteny tree.
 */
::tree<TaggedNode> parse_nhx_tree(const std::string&);

/**
 * Convert a tree of tagged nodes into a NHX-formatted string.
 *
 * @param input Input tree to convert into a string.
 *
 * @return Resulting string representation for the tree.
 */
std::string stringify_nhx_tree(const ::tree<TaggedNode>&);

#endif // IO_NHX_PARSER_HPP
