cmake_minimum_required (VERSION 3.1)
project (lsst_dm_forwarder_daq)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(OBJ
    "Data.cpp"
    "ScienceBuffer.cpp"
    "WavefrontBuffer.cpp"
    "GuidingBuffer.cpp"
    "Pixel3d.cpp"
    "DAQDecoder.cpp"
    "DAQFetcher.cpp"
)

add_library(lsst_dm_forwarder_daq STATIC ${OBJ})
target_compile_definitions(lsst_dm_forwarder_daq PRIVATE BOOST_LOG_DYN_LINK)
target_include_directories(lsst_dm_forwarder_daq PRIVATE
    "${Boost_INCLUDE_DIRS}"
    "${Daq_INCLUDE_DIRS}"
    "../../include"
)
