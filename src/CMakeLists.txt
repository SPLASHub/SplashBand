# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

#if($ENV{TEST} STREQUAL "1")
#    list(REMOVE_ITEM app_sources "${CMAKE_SOURCE_DIR}/src/main.c")
#endif()

idf_component_register(SRCS ${app_sources}
                       INCLUDE_DIRS "."
                       REQUIRES bt nvs_flash nimble_peripheral_utils)
              
