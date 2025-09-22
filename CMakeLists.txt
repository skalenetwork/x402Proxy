cmake_minimum_required(VERSION 3.16)
project(proxygen_x402_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find Proxygen + Folly (must be installed and discoverable via CMake config files).
find_package(folly REQUIRED CONFIG)
find_package(Wangle REQUIRED CONFIG)
find_package(Proxygen REQUIRED CONFIG)

add_executable(x402_demo
        src/main.cpp
        src/X402Handler.cpp
)

target_include_directories(x402_demo PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(x402_demo
        PRIVATE
        Proxygen::proxygenhttpserver
        Proxygen::proxygenlib
        Wangle::wangle
        Folly::folly
)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    target_compile_options(x402_demo PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(x402_demo PRIVATE -fsanitize=address)
endif()