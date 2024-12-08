add_executable(libsysexconf-test
    test.cpp
)

target_link_libraries(libsysexconf-test
    PRIVATE
    libsysexconf-test-common
    libsysexconf
)

target_compile_definitions(libsysexconf-test
    PRIVATE
    TEST
)

add_test(
    NAME test_build
    COMMAND
    "${CMAKE_COMMAND}"
    --build "${CMAKE_BINARY_DIR}"
    --config "$<CONFIG>"
    --target libsysexconf-test
)

set_tests_properties(test_build
    PROPERTIES
    FIXTURES_SETUP
    test_fixture
)

add_test(
    NAME test
    COMMAND $<TARGET_FILE:libsysexconf-test>
)

set_tests_properties(test
    PROPERTIES
    FIXTURES_REQUIRED
    test_fixture
)