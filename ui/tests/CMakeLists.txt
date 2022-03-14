cmake_minimum_required(VERSION 3.10)

project(moth_ui_tests VERSION 1.0 DESCRIPTION "Unit tests for moth_ui")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_compile_definitions(_CRT_SECURE_NO_WARNINGS _ITERATOR_DEBUG_LEVEL=0)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD")

add_subdirectory(.. moth_ui)
include_directories(../external/imgui ${CONAN_INCLUDE_DIRS})

set(SOURCES ${SOURCES}
	src/test_color.cpp
	src/test_vector.cpp
	src/test_main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} moth_ui ${CONAN_LIBS})
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/W3 /WX")

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
