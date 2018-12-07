cmake_minimum_required(VERSION 3.0)
project(gNEEDaccess VERSION 0.0.1)

if(NOT TARGET LSL::lsl)
    # when building out of tree LSL_ROOT needs to be specified on the cmd line
    file(TO_CMAKE_PATH "${LSL_INSTALL_ROOT}" LSL_INSTALL_ROOT)
    list(APPEND LSL_INSTALL_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../LSL/liblsl/build/install")
    find_package(LSL HINTS ${LSL_INSTALL_ROOT}/share/LSL/ ${LSL_INSTALL_ROOT}/LSL/share/LSL QUIET)
    if(NOT LSL_FOUND)
        message(FATAL_ERROR "Precompiled LSL was not found. See https://github.com/labstreaminglayer/labstreaminglayer/blob/master/doc/BUILD.md#lsl_install_root for more information.")
    endif()
    list(APPEND CMAKE_MODULE_PATH ${LSL_DIR})
    message(STATUS "Looking for LSLCMake in ${LSL_DIR}")
    include(LSLCMake)
endif()

# GENERAL CONFIG #
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# THIRD PARTY LIBRARIES #

# Qt
# If Qt5_DIR is not supplied, use default directory defined in ${CMAKE_CURRENT_LIST_DIR}/../../../cmake/LSLSaneDefaults
find_package(Qt5 REQUIRED COMPONENTS Core Xml Gui Widgets)

# Boost
# find_package(Boost REQUIRED)

# g.Tec gNEEDaccessSDK
find_package(gNEEDaccessSDK)


# APPLICATION #

# Collect requirements
SET(gNEEDaccess_SRC)
SET(gNEEDaccess_INCL_DIRS)
SET(gNEEDaccess_REQ_LIBS)

#...sources
LIST(APPEND gNEEDaccess_SRC
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mainwindow.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mainwindow.h
    ${CMAKE_CURRENT_LIST_DIR}/src/mainwindow.ui
    ${CMAKE_CURRENT_LIST_DIR}/src/nautilus_dlg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/nautilus_dlg.h
    ${CMAKE_CURRENT_LIST_DIR}/src/nautilus_dlg.ui
    ${CMAKE_CURRENT_LIST_DIR}/src/gUSB_dlg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/gUSB_dlg.h
    ${CMAKE_CURRENT_LIST_DIR}/src/gUSB_dlg.ui
    ${CMAKE_CURRENT_LIST_DIR}/src/gHIamp_dlg.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/gHIamp_dlg.h
    ${CMAKE_CURRENT_LIST_DIR}/src/gHIamp_dlg.ui
)

#...headers
#LIST(APPEND gNEEDaccess_INCL_DIRS
#    ${Boost_INCLUDE_DIRS}
#)

#...libraries
LIST(APPEND gNEEDaccess_REQ_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Xml
    LSL::lsl
    gtec::gNEEDaccessSDK
    # ${Boost_LIBRARIES}
)

# TARGET #
SET(target gNEEDaccess)
add_executable(${target}
    ${gNEEDaccess_SRC}
)
target_include_directories(${target}
    PRIVATE
    ${gNEEDaccess_INCL_DIRS})
target_link_libraries(${target}
    PRIVATE
    ${gNEEDaccess_REQ_LIBS}
)

#if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#SET_TARGET_PROPERTIES(${target}
#    PROPERTIES
#        LINK_FLAGS "/NODEFAULTLIB:\"liblsl64\""
#)
#else()
#SET_TARGET_PROPERTIES(${target}
#    PROPERTIES
#        LINK_FLAGS "/NODEFAULTLIB:\"liblsl32\""
#)
#endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
# target_compile_features(${PROJECT_NAME} PRIVATE cxx_auto_type cxx_lambda_init_captures)

# Deployment # 
installLSLApp(${target})  # Will handle Qt and LSL libraries
installLSLAuxFiles(${target} ${CMAKE_CURRENT_LIST_DIR}/gneedaccess_config.cfg)
installLSLAuxFiles(${target} ${CMAKE_CURRENT_LIST_DIR}/gNautilus_default.cfg)
installLSLAuxFiles(${target} ${CMAKE_CURRENT_LIST_DIR}/gHIamp_default.cfg)
installLSLAuxFiles(${target} ${CMAKE_CURRENT_LIST_DIR}/gUSBamp_default.cfg)
#installLSLAuxFiles(${target} ${gNEEDaccessSDK_BINARIES})  # Not necessary because g.Tec gNEEDaccess installer puts libraries on the path.

LSLGenerateCPackConfig()