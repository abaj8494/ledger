cmake_minimum_required(VERSION 3.10)
project(ledger_api)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

include_directories(include)

# Find required packages
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)

# Include system Asio
include_directories(/usr/include/asio)
add_definitions(-DASIO_STANDALONE)

# Include Crow headers
include_directories(${CMAKE_SOURCE_DIR}/ext/crow/include)

# Create executable
file(GLOB SOURCES "src/*.cpp")
add_executable(ledger_api ${SOURCES})

# Link libraries
target_link_libraries(ledger_api PRIVATE
    ${Boost_LIBRARIES}
    Threads::Threads
)

# Copy any config files to build directory
configure_file(
    ${CMAKE_SOURCE_DIR}/ledger_api_config.json
    ${CMAKE_BINARY_DIR}/ledger_api_config.json
    COPYONLY
)

# Install target
install(TARGETS ledger_api DESTINATION bin) 