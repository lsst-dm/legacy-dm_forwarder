cmake_minimum_required(VERSION 3.6)
project(dm_forwarder VERSION 1.0.0)
enable_testing()

option(RPMS "Build rpms" OFF)
option(DAQv4 "Build with daq version 4.x" ON)


# set DAQ
if (DAQv4)
    set(Daq_SRC_DIR "./src/daq")
else()
    set(Daq_SRC_DIR "./src/forwarder")
endif()

# c++11 support
# -std=c++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# specifies whether compiler specific extension are requested. Sometimes, this
# results in -std=gnu++11 instead of -std=c++11. Default is ON.
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE RelWithDebInfo)


set(LSST_REPO "http://lsst-web.ncsa.illinois.edu/~hwin16/packages")
set(LSST_INSTALL_DIR "/opt/lsst")

# This should in theory work but doesn't work here because of unknown cmake
# name finding convention for finding boost. boost169 names folders with
# boost169, instead of boost.
# find_package(Boost 1.69.0)
set(Boost_VERSION 1.69.0)
set(Boost_INCLUDE_DIRS /usr/include/boost169)
set(Boost_LIBRARY_DIRS /usr/lib64/boost169)

# Daq rpm variables
set(Daq_VERSION 2.2.7)
set(Daq_URL "${LSST_REPO}/daq/R2-V2.7.tgz")
set(Daq_DIR "/srv/nfs/lsst-daq/daq-sdk/current")
set(Daq_RPM_DIR "/opt/lsst/daq")
set(Daq_INCLUDE_DIRS ${Daq_DIR}/include)
set(Daq_LIBRARY_DIRS ${Daq_DIR}/x86/lib)

# SimpleAmqpClient rpm variables
set(SimpleAmqpClient_VERSION 2.4.0)
set(SimpleAmqpClient_URL
    "${LSST_REPO}/SimpleAmqpClient/SimpleAmqpClient-2.4.0.tar.gz")

# yaml-cpp rpm variables
set(Yaml-cpp_VERSION 2.2.7)
set(Yaml-cpp_URL "${LSST_REPO}/yaml-cpp/yaml-cpp-0.6.3.tar.gz")

# cfitsio rpm variables
set(Cfitsio_VERSION 3.45.0)
set(Cfitsio_URL "${LSST_REPO}/cfitsio/cfitsio-3.450.tar.gz")

# find library
set(daq_libs
    "daq_ims"
    "daq_xds"
    "daq_daq"
    "daq_dcs"
    "daq_dsi"
    "daq_rms"
)
foreach(x ${daq_libs})
    find_library(
        ${x}
        name lib${x}.so
        HINTS
            "${Daq_LIBRARY_DIRS}"
            "/usr/lib64/daq"
    )
endforeach()

set(boost_libs
    "boost_log"
    "boost_filesystem"
    "boost_system"
    "boost_thread"
    "boost_unit_test_framework"
)
foreach(x ${boost_libs})
    find_library(
        ${x}
        name lib${x}.so.${Boost_VERSION}
        HINTS "${Boost_LIBRARY_DIRS}"
    )
endforeach()

if (RPMS)
    set(FWD_COMPONENTS "daq" "SimpleAmqpClient" "yaml-cpp" "cfitsio")
    add_subdirectory("./etc/rpms/daq")
    add_subdirectory("./etc/rpms/SimpleAmqpClient")
    add_subdirectory("./etc/rpms/yaml-cpp")
    add_subdirectory("./etc/rpms/cfitsio")
else()
    set(FWD_COMPONENTS "dm_forwarder")
    add_subdirectory("./src/core")
    add_subdirectory(${Daq_SRC_DIR})
    add_subdirectory("./src/forwarder")
    add_subdirectory("./tests")
endif()

# Cpack Specs
set(CPACK_GENERATOR RPM)
set(CPACK_COMPONENTS_ALL ${FWD_COMPONENTS})
set(CPACK_RPM_COMPONENT_INSTALL 1)

## daq config
set(CPACK_RPM_DAQ_FILE_NAME "lsst-daq-${Daq_VERSION}-x86_64.rpm")
set(CPACK_RPM_DAQ_PACKAGE_NAME "lsst-daq")
set(CPACK_RPM_DAQ_PACKAGE_REQUIRES "gcc-c++")
set(CPACK_RPM_DAQ_PACKAGE_PROVIDES
    "libdaq_ims.so,libdaq_daq.so,libdaq_dsi.so")
set(CPACK_RPM_DAQ_PACKAGE_ARCHITECTURE x86_64)

# SimpleAmqpClient config
set(CPACK_RPM_SIMPLEAMQPCLIENT_FILE_NAME
    "lsst-SimpleAmqpClient-${SimpleAmqpClient_VERSION}-x86_64.rpm")
set(CPACK_RPM_SIMPLEAMQPCLIENT_PACKAGE_NAME "lsst-SimpleAmqpClient")
set(CPACK_RPM_SIMPLEAMQPCLIENT_PACKAGE_REQUIRES
    "librabbitmq-devel, boost169")

# yaml-cpp config
set(CPACK_RPM_YAML-CPP_FILE_NAME "lsst-yaml-cpp-${Yaml-cpp_VERSION}-x86_64.rpm")
set(CPACK_RPM_YAML-CPP_PACKAGE_NAME "lsst-yaml-cpp")

# cfitsio config
set(CPACK_RPM_CFITSIO_FILE_NAME "lsst-cfitsio-${Cfitsio_VERSION}-x86_64.rpm")
set(CPACK_RPM_CFITSIO_PACKAGE_NAME "lsst-cfitsio")

# dm_forwarder config
set(CPACK_RPM_DM_FORWARDER_FILE_NAME
    "dm_forwarder-${PROJECT_VERSION}-x86_64.rpm")
set(CPACK_RPM_DM_FORWARDER_PACKAGE_NAME "dm_forwarder")
set(CPACK_RPM_DM_FORWARDER_POST_INSTALL_SCRIPT_FILE
    ${CMAKE_SOURCE_DIR}/bin/postinstall.sh)

include(CPack)
