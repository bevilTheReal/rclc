#include "catch.hpp"
#include "rclc/rclc.hpp"
#include <string>
#include <type_traits>

TEST_CASE("Cache can be filled and modified", "[basic]")
{
   rclc::cache<int, std::string> cache(100);
   cache[0] = "zero";
   cache[1] = "one";
   cache[2] = "two";

   REQUIRE(cache.size() == 3);
   REQUIRE(cache[0] == "zero");
   REQUIRE(cache[1] == "one");
   REQUIRE(cache[2] == "two");

   SECTION("Can insert and modify items")
   {
      // New elements are default constructed
      REQUIRE(cache[4] == "");

      // Change a value
      cache[1] = "un";
      REQUIRE(cache[1] == "un");

      // Passing key by const-reference
      int key = 5;
      cache[key] = "five";
      REQUIRE(cache[key] == "five");
   }

   SECTION("Check modifiers")
   {
      cache.erase(2);
      REQUIRE(cache.size() == 2);
      REQUIRE(cache[0] == "zero");
      REQUIRE(cache[1] == "one");

      rclc::cache<int, std::string> anotherCache(100);
      cache.swap(anotherCache);
      REQUIRE(cache.empty());
      REQUIRE(anotherCache.size() == 2);
      REQUIRE(anotherCache[0] == "zero");
      REQUIRE(anotherCache[1] == "one");
      anotherCache.clear();
      REQUIRE(anotherCache.empty());
      REQUIRE(anotherCache.size() == 0);
   }
}

TEST_CASE("Eviction works", "[basic][eviction][count-based eviction]")
{
   rclc::cache<int, std::string> cache(3);
   cache[0] = "zero";
   cache[1] = "un";
   cache[2] = "two";
   cache[1] = "one";
   REQUIRE(cache.size() == 3);
   REQUIRE(cache[0] == "zero");
   REQUIRE(cache[1] == "one");
   REQUIRE(cache[2] == "two");

   // Least recently accessed is zero
   cache[4] = "four";
   REQUIRE(cache.size() == 3);
   REQUIRE(cache[4] == "four");
   REQUIRE(cache[1] == "one");
   REQUIRE(cache[2] == "two");
}

TEST_CASE("Types checking", "[compilation]")
{
   typedef rclc::cache<int, std::string> Cache;
   static_assert(std::is_integral<Cache::size_type>::value, "The size type is integral");
   static_assert(std::is_same<Cache::key_type, int>::value, "key_type is the same type as the 1st template parameter");
}

namespace {
struct Sizer : rclc::base_sizer<rclc::item_size_eviction_tag>
{
   size_t operator()(const std::string& s) const
   {
      return s.size();
   }
};
}

TEST_CASE("Size-based eviction", "[eviction][size-based eviction]")
{
   // TODO: make it simpler
   rclc::cache<int, std::string, std::hash<int>, std::equal_to<int>, Sizer> cache(10);
   cache[0] = "one"; // mem used = 3
   cache[1] = "two"; // mem used = 6
   REQUIRE(cache.memory() == 6);
   REQUIRE(cache.size() == 2);
   cache[1] = "xx"; // mem used = 5
   REQUIRE(cache.memory() == 5);
   REQUIRE(cache.size() == 2);
   cache[2] = "abcde"; // mem used = 10
   REQUIRE(cache.memory() == 10);
   REQUIRE(cache.size() == 3);
   cache[3] = "a"; // mem used = 11 -> eviction of oldest, which is 0
   REQUIRE(cache.memory() == 8);
   REQUIRE(cache.size() == 3);
   // Must cast to std::string so that the conversion takes place
   REQUIRE((std::string)cache[0] == "");
   REQUIRE(cache.memory() == 8);
   REQUIRE(cache.size() == 4);

   // copy or const-ref works
   std::string copy = cache[1];
   const std::string& ref = cache[1];
   REQUIRE(ref == copy);

   // Passing key by const-reference
   int key = 5;
   cache[key] = "five"; // mem used = 12 -> eviction of oldest, which is 2
   REQUIRE((std::string)cache[key] == "five");
   REQUIRE(cache.memory() == 7);
   REQUIRE(cache.size() == 4);
}

TEST_CASE("Size-based eviction types checking", "[eviction][compilation][size-based eviction]")
{
   rclc::cache<int, std::string, std::hash<int>, std::equal_to<int>, Sizer> cache(10);
   auto unknown = cache[0];
   static_assert(std::is_convertible<decltype(unknown), std::string>::value, "Wrapper can be copied to the value type");
   static_assert(std::is_convertible<decltype(unknown), const std::string&>::value, "Wrapper can be converted to const ref to the value");
   static_assert(!std::is_convertible<decltype(unknown), std::string&>::value, "Wrapper cannot be converted to non-const ref to value");
}

