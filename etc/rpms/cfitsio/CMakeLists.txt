cmake_minimum_required(VERSION 3.1)

project(cfitsio VERSION ${Cfitsio_VERSION})

include(ExternalProject)

ExternalProject_Add(
    ${PROJECT_NAME}
    BUILD_IN_SOURCE 1
    URL "${Cfitsio_URL}"
    PATCH_COMMAND cp FindPthreads.cmake Findpthreads.cmake
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DUSE_PTHREADS=ON
)
ExternalProject_Get_property(${PROJECT_NAME} INSTALL_DIR)

install(
    DIRECTORY ${INSTALL_DIR}/include/
    DESTINATION include
    COMPONENT cfitsio
    USE_SOURCE_PERMISSIONS
)

install(
    DIRECTORY ${INSTALL_DIR}/lib/
    DESTINATION lib
    COMPONENT cfitsio
    USE_SOURCE_PERMISSIONS
)
