cmake_minimum_required(VERSION 3.22)

project(SysExConf)

include(CTest)

add_library(sysexconf-lib STATIC)

target_sources(sysexconf-lib
    PRIVATE
    src/SysExConf.cpp
)

target_include_directories(sysexconf-lib
    PUBLIC
    include
)

add_custom_target(sysexconf-format
    COMMAND echo Checking code formatting...
    COMMAND ${CMAKE_CURRENT_LIST_DIR}/scripts/code_format.sh
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)

set_target_properties(sysexconf-format PROPERTIES EXCLUDE_FROM_ALL TRUE)

if (CMAKE_EXPORT_COMPILE_COMMANDS STREQUAL ON)
    add_custom_target(sysexconf-lint
        COMMAND ${CMAKE_CURRENT_LIST_DIR}/scripts/clang_tidy_wrapper.sh --output=${CMAKE_CURRENT_BINARY_DIR}/clang-tidy-fixes.yml
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    set_target_properties(sysexconf-lint PROPERTIES EXCLUDE_FROM_ALL TRUE)
    add_dependencies(sysexconf-lint sysexconf-lib)
endif()

if (BUILD_TESTING_SYS_EX_CONF STREQUAL ON)
    add_subdirectory(tests)
endif()