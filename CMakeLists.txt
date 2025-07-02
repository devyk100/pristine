cmake_minimum_required(VERSION 3.14)
project(ReverseProxyLoadBalancer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system)

# Include directories
include_directories(include ${Boost_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/ProxyServer.cpp
    src/LoadBalancer.cpp
    src/WebSocketHandler.cpp
    src/Http2Handler.cpp
    src/Utils.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
