cmake_minimum_required(VERSION 3.5)
project(gNEEDaccess
		LANGUAGES CXX
		VERSION 0.14)


# GENERAL CONFIG #
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# THIRD PARTY LIBRARIES #

# LSL
find_package(LSL REQUIRED
	HINTS ${LSL_INSTALL_ROOT}
	"${CMAKE_CURRENT_LIST_DIR}/../../../LSL/liblsl/build/"
	"${CMAKE_CURRENT_LIST_DIR}/../../../LSL/liblsl/build/install"
	"${CMAKE_CURRENT_LIST_DIR}/../../../LSL/liblsl/out/build/x64-Release"
	"${CMAKE_CURRENT_LIST_DIR}/../../../LSL/liblsl/out/install/x64-Release"
	PATH_SUFFIXES share/LSL
)

# Qt
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED COMPONENTS Xml Widgets)

# g.Tec gNEEDaccessSDK
find_package(gNEEDaccessSDK)

find_package(Threads REQUIRED)


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


#...libraries
LIST(APPEND gNEEDaccess_REQ_LIBS
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Xml
    Threads::Threads
    LSL::lsl
    gtec::gNEEDaccessSDK
)

# TARGET #
SET(target gNEEDaccess)
add_executable(${target} MACOSX_BUNDLE WIN32
    ${gNEEDaccess_SRC}
)
target_include_directories(${target}
    PRIVATE
    ${gNEEDaccess_INCL_DIRS})
target_link_libraries(${target}
    PRIVATE
    ${gNEEDaccess_REQ_LIBS}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
# target_compile_features(${PROJECT_NAME} PRIVATE cxx_auto_type cxx_lambda_init_captures)

# Deployment # 
installLSLApp(${target})  # Will handle Qt and LSL libraries
installLSLAuxFiles(${target} ${CMAKE_CURRENT_LIST_DIR}/gneedaccess_config.cfg)
installLSLAuxFiles(${target} ${CMAKE_CURRENT_LIST_DIR}/gNautilus_default.cfg)
installLSLAuxFiles(${target} ${CMAKE_CURRENT_LIST_DIR}/gHIamp_default.cfg)
installLSLAuxFiles(${target} ${CMAKE_CURRENT_LIST_DIR}/gUSBamp_default.cfg)
#installLSLAuxFiles(${target} ${gNEEDaccessSDK_BINARIES})  # Not necessary because g.Tec gNEEDaccess installer puts libraries on the path.

if (WIN32)
	get_target_property(QT5_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
	get_filename_component(QT5_WINDEPLOYQT_EXECUTABLE ${QT5_QMAKE_EXECUTABLE} PATH)
	set(QT5_WINDEPLOYQT_EXECUTABLE "${QT5_WINDEPLOYQT_EXECUTABLE}/windeployqt.exe")

	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${QT5_WINDEPLOYQT_EXECUTABLE} --qmldir
			${CMAKE_SOURCE_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different  
			$<TARGET_FILE:LSL::lsl>
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_SOURCE_DIR}//gNautilus_default.cfg
			$<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif(WIN32)

LSLGenerateCPackConfig()
