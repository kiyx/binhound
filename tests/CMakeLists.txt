add_executable(unit_tests
    unit/main.cpp
    unit/test_error.cpp)

target_link_libraries(unit_tests PRIVATE
    binhound_core
    doctest::doctest)

include(${DOCTEST_CMAKE_HELPER})
doctest_discover_tests(unit_tests
    PROPERTIES
        ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;UBSAN_OPTIONS=halt_on_error=1")

add_test(NAME cli.version COMMAND binhound --version)
set_tests_properties(cli.version PROPERTIES
    PASS_REGULAR_EXPRESSION "binhound [0-9]+\\.[0-9]+\\.[0-9]+")

add_test(NAME cli.help COMMAND binhound --help)
set_tests_properties(cli.help PROPERTIES
    PASS_REGULAR_EXPRESSION "Usage:")

add_test(NAME cli.unknown_command COMMAND binhound frobnicate)
set_tests_properties(cli.unknown_command PROPERTIES WILL_FAIL TRUE)
