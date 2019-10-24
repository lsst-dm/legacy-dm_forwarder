cmake_minimum_required(VERSION 3.6)

project(daq VERSION ${Daq_VERSION})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    URL "${Daq_URL}"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
ExternalProject_Get_property(${PROJECT_NAME} SOURCE_DIR)
message(status ${SOURCE_DIR})

install(
    DIRECTORY ${SOURCE_DIR}/x86/lib/
    DESTINATION lib64
    COMPONENT daq
    USE_SOURCE_PERMISSIONS
)
install(
    DIRECTORY ${SOURCE_DIR}/include/
    DESTINATION include
    COMPONENT daq
    USE_SOURCE_PERMISSIONS
)
install(
    DIRECTORY ${SOURCE_DIR}/x86/bin/
    DESTINATION bin/daq
    COMPONENT daq
    USE_SOURCE_PERMISSIONS
)
