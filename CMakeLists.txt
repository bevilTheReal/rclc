cmake_minimum_required(VERSION 2.8)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(rclc CXX C)

include_directories(include)

set(CMAKE_CXX_STANDARD 11)

set(_file_list
  include/rclc/rclc.hpp
)

# For gtest
find_package(Threads REQUIRED)

set(EXT_PROJECTS_DIR ${PROJECT_SOURCE_DIR}/ext)
add_subdirectory(${EXT_PROJECTS_DIR}/gtest)

enable_testing()

include_directories(
    ${GTEST_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/include)
  
add_executable(
    rclc-test
    test/main.cpp
    test/basic_test.cpp)

add_dependencies(rclc-test googletest)

target_link_libraries(
    rclc-test
    ${GTEST_LIBS_DIR}/libgtest.a
    ${GTEST_LIBS_DIR}/libgtest_main.a
    ${CMAKE_THREAD_LIBS_INIT})

set_target_properties(rclc-test PROPERTIES
    PREFIX ""
    SUFFIX ""
    COMPILE_FLAGS "-W -Wall -pedantic")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CSS_FLAGS_RELEASE "-03 -g")

add_test(
    NAME rclc-test
    COMMAND rclc-test)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS rclc-test)
