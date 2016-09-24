#include "gtest/gtest.h"
#include "rclc/rclc.hpp"
#include <string>
#include <type_traits>

TEST(BasicUse, canInsertAndAccess)
{
   rclc::cache<int, std::string> cache(100);
   cache[0] = "zero";
   cache[1] = "one";
   cache[2] = "two";
   ASSERT_EQ(3U, cache.size());
   ASSERT_EQ("zero", cache[0]);
   ASSERT_EQ("one", cache[1]);
   ASSERT_EQ("two", cache[2]);
   // New elements are default constructed
   ASSERT_EQ(std::string(), cache[4]);

   // Change a value
   cache[1] = "un";
   ASSERT_EQ("un", cache[1]);
}

TEST(BasicUse, eviction)
{
   rclc::cache<int, std::string> cache(3);
   cache[0] = "zero";
   cache[1] = "un";
   cache[2] = "two";
   cache[1] = "one";
   ASSERT_EQ(3U, cache.size());
   ASSERT_EQ("zero", cache[0]);
   ASSERT_EQ("one", cache[1]);
   ASSERT_EQ("two", cache[2]);

   // Least recently accessed is zero
   cache[4] = "four";
   ASSERT_EQ(3U, cache.size());
   ASSERT_EQ("four", cache[4]);
   ASSERT_EQ("one", cache[1]);
   ASSERT_EQ("two", cache[2]);
}

TEST(BasicUse, modifiers)
{
   rclc::cache<int, std::string> cache(100);
   cache[0] = "zero";
   cache[1] = "one";
   cache[2] = "two";
   cache.erase(2);
   ASSERT_EQ(2U, cache.size());
   ASSERT_EQ("zero", cache[0]);
   ASSERT_EQ("one", cache[1]);

   rclc::cache<int, std::string> anotherCache(100);
   cache.swap(anotherCache);
   ASSERT_TRUE(cache.empty());
   ASSERT_EQ(2U, anotherCache.size());
   ASSERT_EQ("zero", anotherCache[0]);
   ASSERT_EQ("one", anotherCache[1]);
   anotherCache.clear();
   ASSERT_TRUE(anotherCache.empty());
   ASSERT_EQ(0U, anotherCache.size());
}

TEST(BasicUse, typesChecks)
{
   typedef rclc::cache<int, std::string> Cache;
   static_assert(std::is_integral<Cache::size_type>::value, "");
   static_assert(std::is_same<Cache::key_type, int>::value, "");
}

TEST(BasicUse, sizeEviction)
{
   struct Sizer : rclc::base_sizer<rclc::item_size_eviction_tag>
   {
      size_t operator()(const std::string& s) const
      {
         return s.size();
      }
   };
   // TODO: make it simpler
   rclc::cache<int, std::string, std::hash<int>, std::equal_to<int>, std::allocator<std::pair<const int, std::string>>, Sizer> cache(10);
   cache[0] = "one"; // mem used = 3
   cache[1] = "two"; // mem used = 6
   ASSERT_EQ(6U, cache.memory());
   ASSERT_EQ(2U, cache.size());
   cache[1] = "xx"; // mem used = 5
   ASSERT_EQ(5U, cache.memory());
   ASSERT_EQ(2U, cache.size());
   cache[2] = "abcde"; // mem used = 10
   ASSERT_EQ(10U, cache.memory());
   ASSERT_EQ(3U, cache.size());
   cache[3] = "a"; // mem used = 11 -> eviction of oldest, which is 0
   ASSERT_EQ(8U, cache.memory());
   ASSERT_EQ(3U, cache.size());
   // Must cast to std::string so that the conversion takes place
   ASSERT_EQ("", (std::string)cache[0]);
   ASSERT_EQ(8U, cache.memory());
   ASSERT_EQ(4U, cache.size());

   // copy or const-ref works, but not non-const ref
   std::string copy = cache[1];
   const std::string& ref = cache[1];
   ASSERT_EQ(copy, ref);
}

