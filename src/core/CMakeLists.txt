cmake_minimum_required (VERSION 3.6)
project (lsst_iip_core)

# c++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

# Build core objects
set(OBJ
    "Beacon.cpp"
    "Consumer.cpp"
    "Credentials.cpp"
    "FileOpener.cpp"
    "IIPBase.cpp"
    "RabbitConnection.cpp"
    "RedisConnection.cpp"
    "RedisResponse.cpp"
    "SimpleLogger.cpp"
    "SimplePublisher.cpp"
    "Watcher.cpp"
)

add_library(lsst_iip_core STATIC ${OBJ})
target_compile_definitions(lsst_iip_core PRIVATE BOOST_LOG_DYN_LINK)
target_include_directories(lsst_iip_core PRIVATE
    "${Boost_INCLUDE_DIRS}"
    "../../include"
)
