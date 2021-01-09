cmake_minimum_required(VERSION 3.19.2)

project(prnat_traversal)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(common src/common/udp_socket.cpp src/common/ip_address.cpp)
target_compile_features(common PUBLIC cxx_std_17)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(common PUBLIC -g)
endif()

add_executable(server src/server.cpp)
target_link_libraries(server common)

add_executable(client src/client.cpp)
target_link_libraries(client common Threads::Threads)
