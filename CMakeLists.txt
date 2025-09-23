cmake_minimum_required(VERSION 3.16)
project(x402proxy LANGUAGES CXX)

# Acknowledge removal of FindBoost; use BoostConfig instead (vcpkg provides it).
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


file(GLOB SRC_FILES src/*.cpp src/*.h)

# ---- Proxygen / Folly / Wangle ----
find_package(folly    REQUIRED CONFIG)
find_package(Wangle   REQUIRED CONFIG)
find_package(Proxygen REQUIRED CONFIG)
find_package(Boost REQUIRED CONFIG COMPONENTS unit_test_framework system thread)
find_package(CURL  REQUIRED)



# ---- App target ----
add_executable(x402proxy
        main.cpp
        ${SRC_FILES}
)
target_include_directories(x402proxy PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(x402proxy
        PRIVATE
        CURL::libcurl
        proxygen::proxygenhttpserver
        proxygen::proxygen
        wangle::wangle
        Folly::folly
)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    target_compile_options(x402proxy PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(x402proxy PRIVATE -fsanitize=address)
endif()

# ---- Tests: Boost.Test + libcurl ----
include(CTest)
if(BUILD_TESTING)
    enable_testing()


    add_executable(x402_boost_test
            tests/X402HandlerBoostTest.cpp
            ${SRC_FILES}
    )
    target_compile_features(x402_boost_test PRIVATE cxx_std_17)
    target_include_directories(x402_boost_test
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    target_link_libraries(x402_boost_test
            PRIVATE
            CURL::libcurl
            proxygen::proxygenhttpserver
            proxygen::proxygen
            wangle::wangle
            Folly::folly
            Boost::unit_test_framework
            Boost::boost
            Boost::system
            Boost::thread
            Boost::headers
    )

    if(ENABLE_ASAN)
        target_compile_options(x402_boost_test PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(x402_boost_test PRIVATE -fsanitize=address)
    endif()

    add_test(NAME x402_boost_test COMMAND x402_boost_test)
endif()

