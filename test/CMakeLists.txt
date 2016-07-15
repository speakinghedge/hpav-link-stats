add_executable(base-tests base-tests.c)
target_link_libraries(base-tests cmocka)
add_test(base-tests base-tests)