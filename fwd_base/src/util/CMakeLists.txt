cmake_minimum_required (VERSION 3.6)
project (lsst_iip_util)

# Build core objects
set(OBJ
        "FitsComparator.cpp"
        "FitsComparatorCMD.cpp"
)

add_library(lsst_iip_util STATIC ${OBJ})
target_compile_definitions(lsst_iip_util PRIVATE BOOST_LOG_DYN_LINK)
target_include_directories(lsst_iip_util PRIVATE
        "${Boost_INCLUDE_DIRS}"
        "../../include"
)

add_executable(fc_exe FitsComparatorCMD.cpp FitsComparator.cpp)
target_compile_definitions(fc_exe PRIVATE BOOST_LOG_DYN_LINK)
target_compile_definitions(fc_exe PRIVATE BOOST_TEST_DYN_LINK)
target_include_directories(fc_exe PRIVATE
        "${Boost_INCLUDE_DIRS}"
        "../../include"
        )
target_link_libraries(fc_exe PRIVATE
        lsst_iip_core
        ${boost_log}
        ${boost_program_options}
        cfitsio
)