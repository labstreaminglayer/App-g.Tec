cmake_minimum_required(VERSION 3.5)
project(gNEEDaccess
		LANGUAGES CXX
		VERSION 0.0.1)

list(APPEND LSL_INSTALL_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../LSL/liblsl/build/install")
include(Findliblsl.cmake)


# GENERAL CONFIG #
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# THIRD PARTY LIBRARIES #

# Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED COMPONENTS Xml Widgets)

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
