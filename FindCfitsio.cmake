#[======================================================================[.rst:
FindCfitsio
-----------

Finds the Cfitsio library.

Imported Targets
^^^^^^^^^^^^^^^^

This modules provides the following imported targets, if found:
``Cfitsio::Cfitsio``
  The cfitsio library

Result Variables
^^^^^^^^^^^^^^^^

This module will define the following variables:

``Cfitsio_FOUND``
  True if the system has the cfitsio library.
``Cfitsio_VERSION``
  The version of the cfitsio library which was found.
``Cfitsio_INCLUDE_DIRS``
  Include directories needed to use cfitsio.
``Cfitsio_LIBRARIES``
  Libraries needed to link to cfitsio.

Cache Variables
^^^^^^^^^^^^^^^

``Cfitsio_INCLUDE_DIR``
  The directory containing ``fitsio.h``.
``Cfitsio_LIBRARY``
  The path to the cfitsio library.

#]======================================================================]

find_package (PkgConfig)
pkg_check_modules (PC_Cfitsio QUIET Cfitsio)
find_path (Cfitsio_INCLUDE_DIR
    NAMES fitsio.h
    PATHS ${PC_Cfitsio_INCLUDE_DIRS}
    PATH_SUFFIXES cfitsio
)
find_library (Cfitsio_LIBRARY
    NAMES cfitsio
    PATHS ${PC_Cfitsio_LIBRARY_DIRS}
)
set (Cfitsio_VERSION ${PC_Cfitsio_VERSION})
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args (Cfitsio
    FOUND_VAR Cfitsio_FOUND
    REQUIRED_VARS
        Cfitsio_LIBRARY
        Cfitsio_INCLUDE_DIR
    VERSION_VAR Cfitsio_VERSION
)
if (Cfitsio_FOUND)
    set (Cfitsio_LIBRARIES ${Cfitsio_LIBRARY})
    set (Cfitsio_INCLUDE_DIRS ${Cfitsio_INCLUDE_DIR})
    set (Cfitsio_DEFINITIONS ${PC_Cfitsio_CFLAGS_OTHER})
endif ()

if (Cfitsio_FOUND AND NOT TARGET Cfitsio::Cfitsio)
    add_library (Cfitsio::Cfitsio UNKNOWN IMPORTED)
    set_target_properties (Cfitsio::Cfitsio PROPERTIES
        IMPORTED_LOCATION "${Cfitsio_LIBRARY}"
        INTERFACE_COMPILE_OPTIONS "${PC_Cfitsio_CFLAGS_OTHER}"
        INTERFACE_INCLUDE_DIRECTORIES "${Cfitsio_INCLUDE_DIR}"
    )
endif ()

mark_as_advanced (Cfitsio_INCLUDE_DIR Cfitsio_LIBRARY)
