#ifndef _RCLC_RCLC_HPP
#define _RCLC_RCLC_HPP

#include "rclc/detail/cache_base.hpp"

namespace rclc {

// This class has the same sematic as an unordered_map
template < class Key,
           class Value,
           class Hash = std::hash<Key>,
           class Pred = std::equal_to<Key>,
           class Sizer = count_sizer,
           class Alloc = std::allocator< std::pair<const Key, Value> >
           >
class cache : public detail::cache_base<Key, Value, Hash, Pred, Alloc, Sizer>
{
   typedef typename detail::cache_base<Key, Value, Hash, Pred, Alloc, Sizer> super_t;
public:

   cache(size_t maxSize): super_t(maxSize)
   {}

};
}

#endif

