cmake_minimum_required(VERSION 3.8)
project(SOCKS5-Performance-Testing)

set(CMAKE_CXX_STANDARD 17)

set(    INCLUDE_PATHS
        Client
        Timer
        )

set(    SOURCE_FILES
        Client/Client.cpp
        main.cpp
        Timer/Timer.cpp
        )

add_executable(SOCKS5-Performance-Testing ${SOURCE_FILES})

target_include_directories(SOCKS5-Performance-Testing PUBLIC ${INCLUDE_PATHS})

find_package(Boost 1.71.0 REQUIRED COMPONENTS chrono system thread program_options)
target_link_libraries(SOCKS5-Performance-Testing PRIVATE Boost::chrono Boost::system Boost::thread Boost::program_options)