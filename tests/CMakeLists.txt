add_executable(unit_tests
    unit/main.cpp
    unit/test_bytes.cpp
    unit/test_elf_header.cpp
    unit/test_error.cpp
    unit/test_reader.cpp)

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

find_program(PYTHON3_EXECUTABLE python3)
if(PYTHON3_EXECUTABLE)
    execute_process(
        COMMAND "${PYTHON3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/generate_fixtures.py"
            "${CMAKE_CURRENT_BINARY_DIR}/fixtures"
        RESULT_VARIABLE BINHOUND_FIXTURE_RESULT)
    if(BINHOUND_FIXTURE_RESULT EQUAL 0)
        add_test(NAME cli.scan_minimal
            COMMAND binhound scan "${CMAKE_CURRENT_BINARY_DIR}/fixtures/elf64_minimal.bin")
        set_tests_properties(cli.scan_minimal PROPERTIES
            PASS_REGULAR_EXPRESSION "Class:.*ELF64;Type:.*EXEC;Machine:.*x86-64")
    else()
        message(WARNING "Fixture generation failed; skipping the scan smoke test")
    endif()
endif()

macro(binhound_expect_scan_error name fixture expected)
    add_test(NAME ${name}
        COMMAND "${CMAKE_COMMAND}"
            -DBINHOUND_EXE=$<TARGET_FILE:binhound>
            -DFIXTURE=${fixture}
            -DEXPECTED=${expected}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/check_scan_error.cmake")
endmacro()

binhound_expect_scan_error(cli.scan_no_args NONE "requires a file argument")
binhound_expect_scan_error(cli.scan_missing /file/che/non/esiste_xyz "FileNotFound")
binhound_expect_scan_error(cli.scan_text "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/not_elf.txt" "NotElf")
