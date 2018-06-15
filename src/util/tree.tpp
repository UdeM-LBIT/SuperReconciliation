#include <tree.hh>
#include <algorithm>

namespace detail
{
    template<typename Source, typename Dest>
    ::tree<Dest> tree_cast_helper(
        const ::tree<Source>& src,
        typename ::tree<Source>::iterator_base root)
    {
        ::tree<Dest> result{static_cast<Dest>(*root)};

        for (auto it = src.begin(root); it != src.end(root); ++it)
        {
            auto subtree = tree_cast_helper<Source, Dest>(src, it);
            result.append_child(result.begin(), subtree.begin());
        }

        return result;
    }
}

template<typename Source, typename Dest>
::tree<Dest> tree_cast(const ::tree<Source>& src)
{
    auto result = detail::tree_cast_helper<Source, Dest>(src, src.begin());
    return result;
}
