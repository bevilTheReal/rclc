cmake_minimum_required(VERSION 2.8)
project(gtest CXX)
include(ExternalProject)

ExternalProject_Add(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.8.0
    CMAKE_ARGS -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=DebugLibs
               -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=ReleaseLibs
               -DCMAKE_CXX_FLAGS=${MSVC_COMPILER_DEFS}
               -Dgtest_force_shared_crt=ON
               -DBUILD_GTEST=ON
               -DBUILD_GMOCK=OFF
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}"
    INSTALL_COMMAND ""
    UPDATE_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)

ExternalProject_Get_Property(googletest source_dir)
set(GTEST_INCLUDE_DIRS ${source_dir}/googletest/include PARENT_SCOPE)

ExternalProject_Get_Property(googletest binary_dir)

add_library(gtest STATIC IMPORTED GLOBAL)
add_library(gtest_main STATIC IMPORTED GLOBAL)

if(UNIX)

   set(GTEST_LIBRARY ${binary_dir}/googletest/libgtest.a)
   set(GTEST_MAIN_LIBRARY ${binary_dir}/googletest/libgtest_main.a)

   set_target_properties(gtest PROPERTIES IMPORTED_LOCATION ${GTEST_LIBRARY})
   set_target_properties(gtest_main PROPERTIES IMPORTED_LOCATION ${GTEST_MAIN_LIBRARY})

elseif(WIN32)

   set(GTEST_LIBRARY_DEBUG ${binary_dir}/googletest/DebugLibs/gtest.lib)
   set(GTEST_LIBRARY_RELEASE ${binary_dir}/googletest/ReleaseLibs/gtest.lib)
   set(GTEST_MAIN_LIBRARY_DEBUG ${binary_dir}/googletest/DebugLibs/gtest_main.lib)
   set(GTEST_MAIN_LIBRARY_RELEASE ${binary_dir}/googletest/ReleaseLibs/gtest_main.lib)

   set_target_properties(gtest PROPERTIES
      IMPORTED_LOCATION_DEBUG ${GTEST_LIBRARY_DEBUG}
      IMPORTED_LOCATION_RELEASE ${GTEST_LIBRARY_RELEASE}
      IMPORTED_LOCATION_RELWITHDEBINFO ${GTEST_LIBRARY_RELEASE}
      IMPORTED_LOCATION_MINSIZEREL ${GTEST_LIBRARY_RELEASE}
      )

   set_target_properties(gtest_main PROPERTIES
      IMPORTED_LOCATION_DEBUG ${GTEST_MAIN_LIBRARY_DEBUG}
      IMPORTED_LOCATION_RELEASE ${GTEST_MAIN_LIBRARY_RELEASE}
      IMPORTED_LOCATION_RELWITHDEBINFO ${GTEST_MAIN_LIBRARY_RELEASE}
      IMPORTED_LOCATION_MINSIZEREL ${GTEST_MAIN_LIBRARY_RELEASE}
      )

else()
   message(FATAL_ERROR "Unsupported platform")
endif()

