cmake_minimum_required(VERSION 3.13)
project(nightfleet)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#------------------------------DEPENDENCIES------------------------------

find_package(Threads REQUIRED)
find_package(glfw3 REQUIRED)    #assume GLFW is installed on the system

# fetch GLM from Github
include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG 0.9.9.8
)
FetchContent_MakeAvailable(glm)

# build third party libraries from source
add_subdirectory(libraries)

#---------------------------------TARGETS-------------------------------
# nfcommon is for code shared between client & server
add_library(nfcommon "")
target_include_directories(nfcommon PUBLIC include/common) 
target_link_libraries(nfcommon Threads::Threads)

add_executable(nfclient "")
target_include_directories(nfclient PRIVATE include/client)
target_link_libraries(nfclient nfcommon glad glfw imgui)

add_executable(nfserver "")
target_include_directories(nfserver PRIVATE include/server)
target_link_libraries(nfserver nfcommon)

add_subdirectory(source)