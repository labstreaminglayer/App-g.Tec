set(gUSBampCAPI_ROOT_DIR
    "${gUSBampCAPI_ROOT_DIR}"
    CACHE
    PATH
    "Directory to search for gUSBamp SDK")

# TODO: if(WIN32)
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

find_library(gUSBamp_LIBRARY
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
# TODO: elseif(UNIX)
# TODO: endif()  # No apple support.

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(gUSBamp DEFAULT_MSG gUSBamp_INCLUDE_DIRS gUSBamp_LIBRARY)

if(gUSBamp_FOUND)
  set(gUSBamp_LIBRARIES ${gUSBamp_LIBRARY})
endif()

mark_as_advanced(
    gUSBamp_FOUND
    gUSBamp_INCLUDE_DIRS
    gUSBamp_LIBRARIES
    gUSBamp_BINARIES
)