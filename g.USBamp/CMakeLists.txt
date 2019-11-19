cmake_minimum_required(VERSION 3.5)

project(gUSBampLSL
	LANGUAGES CXX
	VERSION 0.1)
list(APPEND LSL_INSTALL_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../../build/install")
include(Findliblsl.cmake)

# Using find_package in Module mode
# https://stackoverflow.com/questions/20746936/cmake-of-what-use-is-find-package-if-you-need-to-specify-cmake-module-path-an
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(gUSBamp REQUIRED)  # Defines gUSBamp_INCLUDE_DIRS and gUSBamp_LIBRARIES

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt5 REQUIRED COMPONENTS Widgets)

find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32
	main.cpp
	mainwindow.cpp
	mainwindow.h
	mainwindow.ui
	gUSBamp_LSL_interface.h
	gUSBamp_LSL_interface.cpp
	gUSBamp_config.h
	gUSB_dlg.cpp
	gUSB_dlg.h
	gUSB_dlg.ui
)

target_include_directories(${PROJECT_NAME} PRIVATE ${gUSBamp_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
	PRIVATE
	Qt5::Widgets
	Threads::Threads
	LSL::lsl
	${gUSBamp_LIBRARIES}
)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 99)  # BOOL
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
# target_compile_features(${PROJECT_NAME} PRIVATE cxx_auto_type cxx_lambda_init_captures)

installLSLApp(${PROJECT_NAME})
installLSLAuxFiles(${PROJECT_NAME}
	${PROJECT_NAME}.cfg
)
LSLGenerateCPackConfig()
