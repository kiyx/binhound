# Checks that `binhound scan` fails with exit code 2 and a message matching EXPECTED.
# Variables (passed with -D): BINHOUND_EXE, FIXTURE ("NONE" means no argument), EXPECTED.
if(FIXTURE STREQUAL "NONE")
    execute_process(
        COMMAND "${BINHOUND_EXE}" scan
        RESULT_VARIABLE exit_code
        OUTPUT_VARIABLE stdout_text
        ERROR_VARIABLE stderr_text)
else()
    execute_process(
        COMMAND "${BINHOUND_EXE}" scan "${FIXTURE}"
        RESULT_VARIABLE exit_code
        OUTPUT_VARIABLE stdout_text
        ERROR_VARIABLE stderr_text)
endif()

if(NOT exit_code EQUAL 2)
    message(FATAL_ERROR "expected exit code 2, got ${exit_code}")
endif()

if(NOT "${stdout_text}${stderr_text}" MATCHES "${EXPECTED}")
    message(FATAL_ERROR "expected output matching '${EXPECTED}'")
endif()
