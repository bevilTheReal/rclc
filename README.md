# RCLC

Welcome to **RCLC**, the **R**ather **C**onfigurable **L**RU **C**ache.

It is (obviously) a cache with LRU eviction policy, implemented in C++.

## How to compile and run tests

Since RCLC is a header-only library, you just have to build the tests:

```
mkdir build
cd build
cmake ..
make check
```

## How to use

### Basic usage
```
// Define a cache of strings with int keys; the maximum number of items is 100
rclc::cache<int, std::string> cache(100);
// The cache has an unordered_map access semantic
cache[42] = "Answer to the Ultimate Question of Life, the Universe, and Everything";
const std::string& answer = cache[42];
```

### Eviction policy
RCLC provides two eviction policies:

1. Item-count based (the default)
2. Item-size based

To use an item-size eviction policy, you have to provide a sizer that computes the size of each value:
```
struct StringSizer : rclc::base_sizer<rclc::item_size_eviction_tag>
{
   size_t operator()(const std::string& s) const
   {
      return s.size();
   }
};
// Create cache with maximum cumulated item sizes = 10
rclc::cache<int, std::string, std::hash<int>, std::equal_to<int>, StringSizer> cache(10);
```

## TODO

* Simplify class hierarchy
* Simplify declaration with item-size-based eviction policy (so that declaring hash and equal\_to is not mandatory)
* Add listeners support
* Add logging

