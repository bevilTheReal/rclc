#ifndef _RCLC_DETAIL_CACHE_BASE_HPP
#define _RCLC_DETAIL_CACHE_BASE_HPP

#include <unordered_map>
#include <list>
#include <utility>

namespace rclc {

// Tags to define the type of eviction {{{
struct item_count_eviction_tag {};
struct item_size_eviction_tag {};

template<class Tag>
struct base_sizer
{
   typedef Tag eviction_policy_t;
};

struct count_sizer : public base_sizer<item_count_eviction_tag>
{
};
// }}}

namespace detail {
// Just the underlying data, without behavior {{{
template < class Key,
           class Value,
           class Hash,
           class Pred,
           class Alloc
           >
class cache_base_container
{
protected:
   typedef std::list<Key, Alloc> eviction_list_t;
   typedef std::pair<typename eviction_list_t::iterator, Value> Acc;
   typedef std::unordered_map<Key, Acc, Hash, Pred, Alloc> cache_t;
public:
   typedef Key key_type;
   typedef Value mapped_type;
   typedef std::pair<const key_type, mapped_type> value_type;
   typedef Hash hasher;
   typedef Pred key_equal;
   typedef Alloc allocator_type;
   typedef value_type& reference;
   typedef const value_type& const_reference;
   typedef typename std::allocator_traits<Alloc>::pointer pointer;
   typedef typename std::allocator_traits<Alloc>::const_pointer const_pointer;
   typedef typename cache_t::size_type size_type;
   typedef typename cache_t::difference_type difference_type;
   // No iterator!

protected:
   eviction_list_t m_evictionList;
   cache_t m_cache;
};
// }}}

// Definition of eviction strategy {{{
// Base
template < class Key,
           class Value,
           class Hash,
           class Pred,
           class Alloc,
           class Sizer,
           class EP
           >
class cache_base_with_evictor;

// Eviction on number of items
template < class Key,
           class Value,
           class Hash,
           class Pred,
           class Alloc,
           class Sizer
           >
class cache_base_with_evictor<Key, Value, Hash, Pred, Alloc, Sizer, item_count_eviction_tag> : public cache_base_container<Key, Value, Hash, Pred, Alloc>
{
   typedef cache_base_container<Key, Value, Hash, Pred, Alloc> super_t;
   size_t m_maxCount;
protected:
   typedef typename super_t::eviction_list_t eviction_list_t;
   typedef typename super_t::key_type key_type;
public:
   cache_base_with_evictor(size_t maxCount)
      : m_maxCount(maxCount){}

   // Access {{{
   Value& operator[](const key_type& key)
   {
      auto& item = this->m_cache[key];
      this->m_evictionList.push_front(key);
      if (item.first != typename eviction_list_t::iterator())
      {
         // Not new item
         this->m_evictionList.erase(item.first);
      }
      item.first = this->m_evictionList.begin();
      this->check_for_eviction();
      return item.second;
   }

   Value& operator[](key_type&& key)
   {
      auto& item = this->m_cache[key];
      this->m_evictionList.push_front(std::move(key));
      if (item.first != typename eviction_list_t::iterator())
      {
         // Not new item
         this->m_evictionList.erase(item.first);
      }
      item.first = this->m_evictionList.begin();
      this->check_for_eviction();
      return item.second;
   }
   // }}}
protected:
   void check_for_eviction()
   {
      if (this->m_cache.size() > m_maxCount)
      {
         const auto& keyToEvict = this->m_evictionList.back();
         this->m_cache.erase(keyToEvict);
         this->m_evictionList.pop_back();
      }
   }
};

// Eviction on total size
template < class Key,
           class Value,
           class Hash,
           class Pred,
           class Alloc,
           class Sizer
           >
class cache_base_with_evictor<Key, Value, Hash, Pred, Alloc, Sizer, item_size_eviction_tag> : public cache_base_container<Key, Value, Hash, Pred, Alloc>
{
   typedef cache_base_container<Key, Value, Hash, Pred, Alloc> super_t;
   typedef cache_base_with_evictor<Key, Value, Hash, Pred, Alloc, Sizer, item_size_eviction_tag> this_type;
   size_t m_maxSize;
   size_t m_currentSize;
   Sizer m_sizer;

   class ValueWrapper
   {
      this_type& m_cache;
      Value& m_value;
   public:
      ValueWrapper(this_type& cache, Value& value): m_cache(cache), m_value(value)
      {
      }
      ValueWrapper& operator=(const Value& value)
      {
         m_cache.m_currentSize -= m_cache.m_sizer(m_value);
         m_value = value;
         m_cache.m_currentSize += m_cache.m_sizer(m_value);
         m_cache.check_for_eviction();
         return *this;
      }
      operator const Value&() const
      {
         return m_value;
      }
   };
protected:
   typedef typename super_t::eviction_list_t eviction_list_t;
   typedef typename super_t::key_type key_type;
public:
   cache_base_with_evictor(size_t maxSize)
      : m_maxSize(maxSize), m_currentSize(0){}

   // Access {{{
   ValueWrapper operator[](const key_type& key)
   {
      auto& item = this->m_cache[key];
      this->m_evictionList.push_front(key);
      if (item.first != typename eviction_list_t::iterator())
      {
         // Not new item
         this->m_evictionList.erase(item.first);
      }
      item.first = this->m_evictionList.begin();
      m_currentSize += m_sizer(item.second);
      return ValueWrapper(*this, item.second);
   }

   ValueWrapper operator[](key_type&& key)
   {
      auto& item = this->m_cache[key];
      this->m_evictionList.push_front(std::move(key));
      if (item.first != typename eviction_list_t::iterator())
      {
         // Not new item
         this->m_evictionList.erase(item.first);
      }
      item.first = this->m_evictionList.begin();
      return ValueWrapper(*this, item.second);
   }
   // }}}

   size_t memory() const
   {
      return m_currentSize;
   }
protected:
   void check_for_eviction()
   {
      while (m_currentSize > m_maxSize)
      {
         const auto& keyToEvict = this->m_evictionList.back();
         auto toRemove = this->m_cache.find(keyToEvict);
         m_currentSize -= m_sizer(toRemove->second.second);
         this->m_cache.erase(toRemove);
         this->m_evictionList.pop_back();
      }
   }
};

// }}}

// Implementation {{{
template < class Key,
           class Value,
           class Hash,
           class Pred,
           class Alloc,
           class Sizer
           >
class cache_base : public cache_base_with_evictor<Key, Value, Hash, Pred, Alloc, Sizer, typename Sizer::eviction_policy_t>
{
   typedef cache_base_with_evictor<Key, Value, Hash, Pred, Alloc, Sizer, typename Sizer::eviction_policy_t> super_t;
   typedef typename super_t::eviction_list_t eviction_list_t;
   typedef typename super_t::cache_t cache_t;
public:
   typedef typename super_t::size_type size_type;
   typedef typename super_t::mapped_type mapped_type;
   typedef typename super_t::key_type key_type;

   cache_base(size_t maxSize): super_t(maxSize)
   {}

   // Capacity {{{
   bool empty() const
   {
      return this->m_cache.empty();
   }
   size_type size() const
   {
      return this->m_cache.size();
   }
   // }}}

   // Modifier {{{
   size_type erase(const key_type& k)
   {
      auto item = this->m_cache.find(k);
      if (item == this->m_cache.end())
      {
         return 0;
      }
      this->m_evictionList.erase(item->second.first);
      this->m_cache.erase(item);
      return 1;
   }

   void clear() noexcept
   {
      this->m_cache.clear();
      this->m_evictionList.clear();
   }

   void swap(cache_base& that)
   {
      this->m_cache.swap(that.m_cache);
      this->m_evictionList.swap(that.m_evictionList);
   }
   // }}}

   // Observers {{{
   typename super_t::hasher hash_function() const
   {
      return this->m_cache.hash_function();
   }
   typename super_t::key_equal key_eq() const
   {
      return this->m_cache.key_eq();
   }
   typename super_t::allocator_type get_allocator() const noexcept
   {
      return this->m_cache.get_allocator();
   }
   // }}}
};
// }}}
}
}

#endif
