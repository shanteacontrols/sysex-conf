find_package(glog 0.6.0 REQUIRED)

add_library(sysexconf-test-common)

target_sources(sysexconf-test-common
    PRIVATE
    main.cpp
)

target_include_directories(sysexconf-test-common
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../include
)

target_link_libraries(sysexconf-test-common
    PUBLIC
    glog::glog
    gtest
)

add_subdirectory(test)