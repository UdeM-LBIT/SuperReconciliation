#ifndef UTIL_TREE_HPP
#define UTIL_TREE_HPP

/**
 * Cast a tree from one type of node to another, provided the appropriate
 * conversion operators exist for the underlying types.
 *
 * @param src Source tree to convert.
 *
 * @return Converted tree.
 */
template<typename Source, typename Dest>
::tree<Dest> tree_cast(const ::tree<Source>&);

#include "tree.tpp"

#endif // UTIL_TREE_HPP
