cmake_minimum_required(VERSION 3.1)

project(rabbitmq-c VERSION ${Rabbitmq-C_VERSION})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    BUILD_IN_SOURCE 1
    URL "${Rabbitmq-C_URL}"
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)
ExternalProject_Get_property(${PROJECT_NAME} INSTALL_DIR)

install(
    DIRECTORY ${INSTALL_DIR}/include/
    DESTINATION include
    COMPONENT rabbitmq-c
    USE_SOURCE_PERMISSIONS
)

install(
    DIRECTORY ${INSTALL_DIR}/lib64/
    DESTINATION lib
    COMPONENT rabbitmq-c
    USE_SOURCE_PERMISSIONS
)
