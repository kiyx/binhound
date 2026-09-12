add_test(NAME cli.version COMMAND binhound --version)
set_tests_properties(cli.version PROPERTIES
    PASS_REGULAR_EXPRESSION "binhound [0-9]+\\.[0-9]+\\.[0-9]+")

add_test(NAME cli.help COMMAND binhound --help)
set_tests_properties(cli.help PROPERTIES
    PASS_REGULAR_EXPRESSION "Usage:")

add_test(NAME cli.unknown_command COMMAND binhound frobnicate)
set_tests_properties(cli.unknown_command PROPERTIES WILL_FAIL TRUE)
