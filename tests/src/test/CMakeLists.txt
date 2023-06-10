add_executable(sysexconf-test
    test.cpp
)

target_link_libraries(sysexconf-test
    PRIVATE
    sysexconf-test-common
    sysexconf-lib
)

target_compile_definitions(sysexconf-test
    PRIVATE
    TEST
)

target_compile_definitions(sysexconf-lib
    PUBLIC
    EMU_EEPROM_PAGE_SIZE=128
)

add_test(
    NAME test_build
    COMMAND
    "${CMAKE_COMMAND}"
    --build "${CMAKE_BINARY_DIR}"
    --config "$<CONFIG>"
    --target sysexconf-test
)

set_tests_properties(test_build
    PROPERTIES
    FIXTURES_SETUP
    test_fixture
)

add_test(
    NAME test
    COMMAND $<TARGET_FILE:sysexconf-test>
)

set_tests_properties(test
    PROPERTIES
    FIXTURES_REQUIRED
    test_fixture
)