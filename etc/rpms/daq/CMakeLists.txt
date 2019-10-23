cmake_minimum_required(VERSION 3.6)

project(daq VERSION 2.7.0)

set(TAR_GZ "R2-V2.7")

include(ExternalProject)
ExternalProject_Add(
    ${PROJECT_NAME}
    URL "${LSST_REPO}/${PROJECT_NAME}/${TAR_GZ}.tgz"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)
ExternalProject_Get_property(${PROJECT_NAME} SOURCE_DIR)

install(
    DIRECTORY ${SOURCE_DIR} 
    DESTINATION daq
    COMPONENT DAQ
    USE_SOURCE_PERMISSIONS
)
