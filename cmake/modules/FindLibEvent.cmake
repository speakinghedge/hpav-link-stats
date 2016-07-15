# - Find LibEvent (a cross event library)
#
# copied from https://raw.githubusercontent.com/facebook/hhvm/master/CMake/FindLibEvent.cmake
#
# extended to find given component pthreads
#
# This module defines always
# LIBEVENT_INCLUDE_DIR, where to find LibEvent headers
# LIBEVENT_LIB, LibEvent libraries
# LibEvent_FOUND, If false, do not try to use libevent
# LIBEVENT_PTHREADS_LIB, LibEvent-pthreads extension libraries
# LibEventPthreads_FOUND, If false, do not try to use libevent_pthread

set(LibEvent_EXTRA_PREFIXES /usr/local /opt/local "$ENV{HOME}")
foreach(prefix ${LibEvent_EXTRA_PREFIXES})
    list(APPEND LibEvent_INCLUDE_PATHS "${prefix}/include")
    list(APPEND LibEvent_LIB_PATHS "${prefix}/lib")
endforeach()

find_path(LIBEVENT_INCLUDE_DIR event.h PATHS ${LibEvent_INCLUDE_PATHS})
find_library(LIBEVENT_LIB NAMES event PATHS ${LibEvent_LIB_PATHS})

if (LIBEVENT_LIB AND LIBEVENT_INCLUDE_DIR)
    set(LibEvent_FOUND TRUE)
    set(LIBEVENT_LIB ${LIBEVENT_LIB})
else ()
    set(LibEvent_FOUND FALSE)
endif ()

if(LibEvent_FOUND AND LIBEVENT_PTHREADS_LIB)
    set(LibEventPthreads_FOUND TRUE)
else()
    set(LibEventPthreads_FOUND FALSE)
endif ()

if (LibEvent_FOUND)
    if (NOT LibEvent_FIND_QUIETLY)
        message(STATUS "Found libevent: ${LIBEVENT_LIB}")
    endif ()
else ()
    if (LibEvent_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find libevent.")
    endif ()
    message(STATUS "libevent NOT found.")
endif ()

mark_as_advanced(
        LIBEVENT_LIB
        LIBEVENT_INCLUDE_DIR
)

if (LibEvent_FIND_REQUIRED_pthreads)

    find_library(LIBEVENT_PTHREADS_LIB NAMES event_pthreads PATHS ${LibEvent_LIB_PATHS})

    if (LIBEVENT_PTHREADS_LIB AND LIBEVENT_INCLUDE_DIR)
        set(LibEventPthreads_FOUND TRUE)
        if (NOT LibEvent_FIND_QUIETLY)
            message(STATUS "Found libevent_pthreads: ${LIBEVENT_PTHREADS_LIB}")
        endif ()
    else ()
        set(LibEventPthreads_FOUND FALSE)
        message(FATAL_ERROR "Could NOT find libevent_pthreads.")
    endif ()

    mark_as_advanced(LIBEVENT_PTHREADS_LIB)

endif (LibEvent_FIND_REQUIRED_pthreads)

