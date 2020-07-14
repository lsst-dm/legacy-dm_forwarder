cmake_minimum_required (VERSION 3.1)
project (lsst_iip_tests)

# Build forwarder objects
set(OBJ
    "./core/RabbitConnectionTest.cpp"
    "./core/RedisConnectionTest.cpp"
    #"./daq/DAQFetcherTest.cpp"
    #"./daq/DataTest.cpp"
    "./forwarder/miniforwarderTest.cpp"
    "./forwarder/YAMLFormatterTest.cpp"
    "./forwarder/ReadoutPatternTest.cpp"
    "./daq/ScannerTest.cpp"
)

add_library(lsst_iip_tests SHARED ${OBJ})
target_compile_definitions(lsst_iip_tests PRIVATE BOOST_LOG_DYN_LINK)
target_compile_definitions(lsst_iip_tests PRIVATE BOOST_TEST_DYN_LINK)
target_include_directories(lsst_iip_tests PRIVATE
    "${Boost_INCLUDE_DIRS}"
    "${Daq_INCLUDE_DIRS}"
    "../include"
)

# Build test executable
add_executable(test_exe Runner.cpp)
target_compile_definitions(test_exe PRIVATE BOOST_LOG_DYN_LINK)
target_compile_definitions(test_exe PRIVATE BOOST_TEST_DYN_LINK)
target_include_directories(test_exe PRIVATE
    "${Boost_INCLUDE_DIRS}"
    "${Daq_INCLUDE_DIRS}"
    "../include"
)
target_link_libraries(test_exe PRIVATE
    lsst_iip_tests
    lsst_dm_forwarder
    lsst_dm_forwarder_daq
    lsst_iip_core
    lsst_iip_util
    ${daq_ims}
    ${daq_xds}
    ${daq_daq}
    ${daq_dcs}
    ${daq_dsi}
    ${daq_rms}
    ${daq_net}
    ${daq_osa}
    ${boost_log}
    ${boost_thread}
    ${boost_filesystem}
    ${boost_unit_test_framework}
    SimpleAmqpClient
    curl
    yaml-cpp
    pthread
    cfitsio
    hiredis
)

set(FWD_TESTS
    "RabbitConnectionTest/constructor"
    "ReadoutPatternTest/constructor"
    "ReadoutPatternTest/pattern"
    "RedisConnectionTest/constructor"
    "miniforwarderTest/end_readout"
    "miniforwarderTest/check_valid_board"
)

foreach (x ${FWD_TESTS})
    add_test(NAME ${x} COMMAND test_exe --run_test=${x})
endforeach()
set_property(TEST ${FWD_TESTS} PROPERTY ENVIRONMENT IIP_CONFIG_DIR=${IIP_CONFIG_DIR})
