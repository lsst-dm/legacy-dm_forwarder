cmake_minimum_required(VERSION 3.6)

project(dm_forwarder)

set(LSST_REPO "http://lsst-web.ncsa.illinois.edu/~hwin16/packages")
set(INSTALL_DIR "/opt/lsst/dm_forwarder")
set(DAQ_DIR "/srv/nfs/lsst-daq/daq-sdk/current")

#add_subdirectory("./etc/rpms/daq")
#add_subdirectory("./etc/rpms/SimpleAmqpClient")

include(ExternalProject)
ExternalProject_Add(
    ${PROJECT_NAME}
    URL "${LSST_REPO}/daq/R2-V2.7.tgz"
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

set(CPACK_GENERATOR RPM)
set(CPACK_COMPONENTS_ALL DAQ)
set(CPACK_RPM_FILE_NAME "goodie.rpm")
set(CPACK_COMPONENTS_GROUPING IGNORE)
set(CPACK_RPM_COMPONENT_INSTALL 1)
include(CPack)
