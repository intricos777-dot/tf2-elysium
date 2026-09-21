cmake_minimum_required(VERSION 3.20)
project(tf2-elysium CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json 3.11 QUIET)
if(NOT nlohmann_json_FOUND)
    include(FetchContent)
    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
    FetchContent_MakeAvailable(json)
endif()

set(DATA_SOURCES src/data/data_registries.cpp)
set(COMBAT_SOURCES src/combat/combat.cpp)
set(CORE_SOURCES src/core/game.cpp)

add_executable(tf2_elysium ${DATA_SOURCES} ${COMBAT_SOURCES} ${CORE_SOURCES})

target_include_directories(tf2_elysium PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(tf2_elysium PRIVATE nlohmann_json::nlohmann_json)

file(COPY Content DESTINATION ${CMAKE_BINARY_DIR})
