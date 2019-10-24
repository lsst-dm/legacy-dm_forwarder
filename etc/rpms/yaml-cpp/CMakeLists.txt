cmake_minimum_required(VERSION 3.1)

project(yaml-cpp VERSION ${Yaml-cpp_VERSION})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    BUILD_IN_SOURCE 1
    URL "${Yaml-cpp_URL}"
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DYAML_BUILD_SHARED_LIBS=ON
)
ExternalProject_Get_property(${PROJECT_NAME} INSTALL_DIR)

install(
    DIRECTORY ${INSTALL_DIR}/include/
    DESTINATION include
    COMPONENT yaml-cpp
    USE_SOURCE_PERMISSIONS
)

install(
    DIRECTORY ${INSTALL_DIR}/lib/
    DESTINATION lib
    COMPONENT yaml-cpp
    USE_SOURCE_PERMISSIONS
)
