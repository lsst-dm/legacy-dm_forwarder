cmake_minimum_required(VERSION 3.1)

project(SimpleAmqpClient VERSION ${SimpleAmqpClient_VERSION})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    BUILD_IN_SOURCE 1
    URL "${SimpleAmqpClient_URL}"
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DBOOST_INCLUDEDIR=${Boost_INCLUDE_DIRS}
        -DBOOST_LIBRARYDIR=${Boost_LIBRARY_DIRS}
)
ExternalProject_Get_property(${PROJECT_NAME} INSTALL_DIR)

install(
    DIRECTORY ${INSTALL_DIR}/include/
    DESTINATION include
    COMPONENT SimpleAmqpClient
    USE_SOURCE_PERMISSIONS
)

install(
    DIRECTORY ${INSTALL_DIR}/lib/
    DESTINATION lib
    COMPONENT SimpleAmqpClient
    USE_SOURCE_PERMISSIONS
)
