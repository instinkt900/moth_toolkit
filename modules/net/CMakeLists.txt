cmake_minimum_required(VERSION 3.27)

project(moth_net
    VERSION 0.1.0
    DESCRIPTION "TCP networking (length-prefixed JSON messaging) for the Moth toolkit."
    LANGUAGES CXX)

find_package(nlohmann_json REQUIRED)
find_package(asio REQUIRED)

# Sibling module: in the superbuild the target already exists, standalone it is
# located via find_package.
if(NOT TARGET moth::core)
    find_package(moth_core CONFIG REQUIRED)
endif()

add_library(moth_net
    src/tcp_server.cpp
    src/tcp_client.cpp)
target_compile_features(moth_net PUBLIC cxx_std_17)
set_target_properties(moth_net PROPERTIES EXPORT_NAME net)

target_include_directories(moth_net PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_include_directories(moth_net PRIVATE src)

target_link_libraries(moth_net PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(moth_net PUBLIC asio::asio)
target_link_libraries(moth_net PUBLIC moth::core)

# Install + export only when built standalone; in the superbuild the modules are consumed
# in-tree and packaged individually via Conan.
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    install(TARGETS moth_net EXPORT moth_netTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT moth_netTargets
        FILE moth_netTargets.cmake
        NAMESPACE moth::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_net)

    configure_package_config_file(
        cmake/moth_netConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/moth_netConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_net)

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/moth_netConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/moth_netConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/moth_netConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moth_net)
endif()
