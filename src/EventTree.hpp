#ifndef TREE_PARSER_HPP
#define TREE_PARSER_HPP

#include "Event.hpp"
#include <string>
#include <tree.hh>

/**
 * Builds a labeled synteny tree based on a pseudo-Newick-formatted string
 * of characters. The grammar is:
 *
 * tree ::= '(' event children ')'
 * children ::= tree*
 * event ::= type ':' synteny | ':' synteny | type
 * type ::= 'leaf' | 'dupl' | 'spec' | 'loss'
 * synteny ::= gene gene*
 * gene ::= [^:() \t\r\n]+
 *
 * @param input Input string to parse.
 * @throws If the input string does not conform to the above syntax.
 * @return Resulting labeled synteny tree.
 */
tree<Event> string_to_event_tree(const std::string&);

std::string event_tree_to_string(const tree<Event>&);

#endif // TREE_PARSER_HPP
