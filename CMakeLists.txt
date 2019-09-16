cmake_minimum_required (VERSION 3.13)

project (Forwarder)

set (LSST_DAQ_VERSION R2-V2.7)
set (BOOST_INCLUDEDIR /usr/include/boost169)
set (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

find_package (Boost 1.69.0 REQUIRED COMPONENTS date_time filesystem log)
find_package (Cfitsio)

include_directories ("${PROJECT_SOURCE_DIR}/include"
    ${Boost_INCLUDE_DIRS}
    ${Cfitsio_INCLUDE_DIRS}
    /opt/lsst/daq/${LSST_DAQ_VERSION}/include
)
link_directories (/opt/lsst/daq/${LSST_DAQ_VERSION}/x86/lib)

file (GLOB Sources src/forwarder/*.cpp src/core/*.cpp)
set (Excluded_Sources DAQForwarder.cpp FileManifold.cpp)
foreach (Source ${Excluded_Sources})
    list (REMOVE_ITEM Sources ${PROJECT_SOURCE_DIR}/src/forwarder/${Source})
endforeach ()

add_executable (fwd ${Sources})
set_property (TARGET fwd PROPERTY CXX_STANDARD 14)
set_property (TARGET fwd PROPERTY CXX_STANDARD_REQUIRED ON)
add_compile_definitions(BOOST_LOG_DYN_LINK=1)
# set_property (TARGET fwd PROPERTY COMPILE_DEFINITIONS BOOST_LOG_DYN_LINK=1)

target_link_libraries (fwd Boost::date_time Boost::filesystem Boost::log_setup Boost::log SimpleAmqpClient Cfitsio::Cfitsio curl hiredis rabbitmq yaml-cpp daq_daq daq_ims)
set (CMAKE_INSTALL_PREFIX /opt/lsst/dm-prompt)
install (TARGETS fwd DESTINATION bin)

set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "LSST Prompt Forwarder")
set (CPACK_PACKAGE_VENDOR "LSST")
set (CPACK_RPM_PACKAGE_LICENSE "GPL3")
set (CPACK_RPM_PACKAGE_REQUIRES "boost169-date-time,boost169-filesystem,boost169-log,cfitsio,curl,hiredis,librabbitmq,yaml-cpp")
set (CPACK_GENERATOR "RPM")
include (CPack)
