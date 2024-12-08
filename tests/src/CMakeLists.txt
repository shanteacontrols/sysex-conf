find_package(glog 0.6.0 REQUIRED)

add_library(libsysexconf-test-common)

target_sources(libsysexconf-test-common
    PRIVATE
    main.cpp
)

target_include_directories(libsysexconf-test-common
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../include
)

target_link_libraries(libsysexconf-test-common
    PUBLIC
    glog::glog
    gtest
)

add_subdirectory(test)