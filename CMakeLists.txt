cmake_minimum_required(VERSION 2.6)
project(hclc)

add_executable(hclc main.cpp)

install(TARGETS hclc RUNTIME DESTINATION bin)
