#ifndef TREE_PARSER_HPP
#define TREE_PARSER_HPP

#include "Event.hpp"
#include <tree.hh>

tree<Event> newick_to_tree(const std::string& input);

#endif // TREE_PARSER_HPP
