cmake_minimum_required(VERSION 3.0.2)
project(homeplug_av_stats)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src)
add_subdirectory(bin)

find_package(CMocka)

if(CMOCKA_FOUND)
    enable_testing()
    add_subdirectory(test)
else()
    message(WARNING "*** UNIT TESTS DISABLED ***")
endif(CMOCKA_FOUND)