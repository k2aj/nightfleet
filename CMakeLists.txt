cmake_minimum_required(VERSION 3.13)
project(nightfleet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

include_directories(include)

# targets
add_library(nfcommon "")
target_link_libraries(nfcommon Threads::Threads)

add_executable(nfclient "")
target_link_libraries(nfclient nfcommon)

add_executable(nfserver "")
target_link_libraries(nfserver nfcommon)

add_subdirectory(source)