cmake_minimum_required(VERSION 3.22)

project(SysExConf)

include(CTest)

add_library(libsysexconf STATIC)

target_sources(libsysexconf
    PRIVATE
    src/sysexconf.cpp
)

target_include_directories(libsysexconf
    PUBLIC
    include
)

add_custom_target(libsysexconf-format
    COMMAND echo Checking code formatting...
    COMMAND ${CMAKE_CURRENT_LIST_DIR}/scripts/code_format.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

set_target_properties(libsysexconf-format PROPERTIES EXCLUDE_FROM_ALL TRUE)

if (CMAKE_EXPORT_COMPILE_COMMANDS STREQUAL ON)
    add_custom_target(libsysexconf-lint
        COMMAND ${CMAKE_CURRENT_LIST_DIR}/scripts/clang_tidy_wrapper.sh --output=${CMAKE_CURRENT_BINARY_DIR}/clang-tidy-fixes.yml
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    set_target_properties(libsysexconf-lint PROPERTIES EXCLUDE_FROM_ALL TRUE)
    add_dependencies(libsysexconf-lint libsysexconf)
endif()

if (BUILD_TESTING_SYS_EX_CONF STREQUAL ON)
    add_subdirectory(tests)
endif()