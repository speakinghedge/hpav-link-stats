find_package(LibEvent COMPONENTS pthreads)

set(COMMON_LIBRARIES
        helper
        events
        pthread
        ${LIBEVENT_LIB}
        ${LIBEVENT_PTHREADS_LIB}
        net
        ieee1901_2010
)

add_executable(loop-mm-types loop-mm-types.c)
target_link_libraries(loop-mm-types ${COMMON_LIBRARIES})

add_executable(hpav-discover hpav-discover.c)
target_link_libraries(hpav-discover ${COMMON_LIBRARIES})

add_executable(hpav-stats hpav-stats.c)
target_link_libraries(hpav-stats ${COMMON_LIBRARIES})