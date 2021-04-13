cmake_minimum_required (VERSION 3.6)
project (lsst_iip_core)

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
