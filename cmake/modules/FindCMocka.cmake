# - Find LibCMocka (a unit test library)
#
# This module defines always
#
# CMOCKA_INCLUDE_DIR, cmocke header file location
# CMOCKA_LIB, cmocka libraries to link again
# CMOCKA_FOUND, If set to false, CMOCKA is not available
#

set(CMocka_EXTRA_PREFIXES /usr/local /opt/local "$ENV{HOME}")
foreach(prefix ${CMocka_EXTRA_PREFIXES})
    list(APPEND CMocka_INCLUDE_PATHS "${prefix}/include")
    list(APPEND CMocka_LIB_PATHS "${prefix}/lib")
endforeach()

find_path(CMOCKA_INCLUDE_DIR cmocka.h PATHS ${LibCMocka_INCLUDE_PATHS})
find_library(CMOCKA_LIB NAMES cmocka PATHS ${LibCMocka_LIB_PATHS})

if (CMOCKA_INCLUDE_DIR AND CMOCKA_LIB)
    set(CMOCKA_FOUND TRUE)
    if (NOT CMocka_FIND_QUIETLY)
        message(STATUS "Found cmocka: ${CMOCKA_LIB}")
    endif ()
else ()
    set(CMOCKA_FOUND FALSE)
    if (CMocka_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find cmocka.")
    endif ()
    message(STATUS "cmocka NOT found.")
endif()

mark_as_advanced(
    CMOCKA_INCLUDE_DIR
    CMOCKA_LIB
)