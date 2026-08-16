cmake_minimum_required(VERSION 3.15)

project(moth_physics_tests VERSION 1.0 DESCRIPTION "Unit tests for moth_physics")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Catch2 3 REQUIRED)

add_subdirectory(.. moth_physics)

set(SOURCES
    src/main.cpp
    src/test_world.cpp
    src/test_body.cpp
    src/test_contacts.cpp
    src/test_filter.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE moth_physics Catch2::Catch2)

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(${PROJECT_NAME} PRIVATE /W3 /WX /utf-8)
endif()

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
