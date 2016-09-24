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
rclc::LRUCache<int, std::string> cache(100);
// The cache has an unordered_map access semantic
cache[42] = "Answer to the Ultimate Question of Life, the Universe, and Everything";
const std::string& answer = cache[42];
```

### Eviction policy
RCLC provides two eviction policies:

1. Item-count based (the default)
2. Item-size based

TODO

## TODO

* Simplify class hierarchy
* Simplify declaration with item-size-based eviction policy
* Add listeners support
* Add logging

