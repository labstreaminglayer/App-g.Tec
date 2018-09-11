set(gUSBampCAPI_ROOT_DIR
    "${gUSBampCAPI_ROOT_DIR}"
    CACHE
    PATH
    "Directory to search for gUSBamp SDK")

find_path(gUSBamp_INCLUDE_DIR
    NAMES
        gUSBamp.h
    PATHS
        ${gUSBampCAPI_ROOT_DIR}
        "C:/Program Files/gtec/gUSBampCAPI/API" 
        "C:/Program Files (x86)/gtec/gUSBampCAPI/API" 
        "C:/Program Files/gtec/gUSBampCAPI/API/Win32" 
        "C:/Program Files (x86)/gtec/gUSBampCAPI/API/Win32"
)
list(APPEND gUSBamp_INCLUDE_DIRS ${gUSBamp_INCLUDE_DIR})

find_library(gUSBamp_LIBRARIES
    NAMES
        gUSBamp
    PATHS
        ${gUSBamp_INCLUDE_DIRS}
)

find_file(gUSBamp_BINARIES
    NAMES
        ${CMAKE_SHARED_LIBRARY_PREFIX}gUSBamp${CMAKE_SHARED_LIBRARY_SUFFIX}
    PATHS
        ${gUSBamp_INCLUDE_DIRS}
)

SET(gUSBamp_FOUND TRUE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(gUSBamp
    DEFAULT_MSG
    gUSBamp_FOUND
    gUSBamp_INCLUDE_DIRS
    gUSBamp_LIBRARIES
    gUSBamp_BINARIES)

mark_as_advanced(
    gUSBamp_FOUND
    gUSBamp_INCLUDE_DIRS
    gUSBamp_LIBRARIES
    gUSBamp_BINARIES)