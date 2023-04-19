project(plv-player-sdk-demo VERSION ${PLV_PLAYER_SDK_VERSION})
set(APP_SUFFIX "32")
if (CMAKE_CL_64)
    set(APP_SUFFIX "64")
endif()
set(WLIB_TARGET "x86")
if (CMAKE_CL_64)
    set(WLIB_TARGET "x64")
endif()

set_property(GLOBAL PROPERTY AUTOGEN_SOURCE_GROUP "Generated Files")
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON) # incompatible of VS 2017
#set(CMAKE_AUTOUIC ON) # incompatible of VS 2017
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

if(DEFINED QTDIR${APP_SUFFIX})
	list(APPEND CMAKE_PREFIX_PATH "${QTDIR${APP_SUFFIX}}")
elseif(DEFINED QTDIR)
	list(APPEND CMAKE_PREFIX_PATH "${QTDIR}")
elseif(DEFINED ENV{QTDIR${APP_SUFFIX}})
	list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR${APP_SUFFIX}}")
elseif(DEFINED ENV{QTDIR})
	list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}")
endif()

set(QT_DEPENDENCY
    Core Widgets Gui Svg
    CACHE INTERNAL "Qt Dependency" FORCE
    )
find_package(Qt5 COMPONENTS ${QT_DEPENDENCY} REQUIRED)
message(STATUS "Found Qt5 Version:" ${Qt5Core_VERSION})

set(CAINFO_FILE ${CMAKE_MODULE_PATH}/cacert.pem)

set(APP_GENERATE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generate)
set(APP_GENERATE_HEADER_DIR ${APP_GENERATE_DIR}/Headers)
configure_file(${CMAKE_MODULE_PATH}/version.h.in
    ${APP_GENERATE_HEADER_DIR}/version.h)

#set(QT_DEPENDENCY Core Widgets Gui Svg winextras CACHE STRING "Qt Dependency")
#find_package(Qt5 COMPONENTS ${QT_DEPENDENCY})

set(app_HEADERS
	${app_HEADERS}
	configure.h
	qt-helper.h
	text-lookup.h
	translator.h
	platform.h
	AppDef.h
	SdkManager.h
	Application.h
	TitleBar.h
	SliderControl.h
	VideoControl.h
	VolumeControl.h
	TipsWidget.h
	ToolButton.h
	StatusButton.h
	WidgetHelper.h
	MyVideoList.h
	MainWindow.h
	PlayerWidget.h
	LoginDialog.h
	ParamDialog.h
	SettingDialog.h
	VideoTableWidget.h
	VidDialog.h
    DeviceWarnDialog.h
	MsgBoxDialog.h
	Player.h
	Downloader.h
	MultiPlayerDialog.h
	)
set(app_SOURCES
	${app_SOURCES}
	configure.cpp
	qt-helper.cpp
	text-lookup.cpp
	translator.cpp
	Main.cpp
	SdkManager.cpp
	Application.cpp
	TitleBar.cpp
	SliderControl.cpp
	VideoControl.cpp
	VolumeControl.cpp
	TipsWidget.cpp
	ToolButton.cpp
	StatusButton.cpp
	WidgetHelper.cpp
	MyVideoList.cpp
	MainWindow.cpp
	PlayerWidget.cpp
	LoginDialog.cpp
	ParamDialog.cpp
	SettingDialog.cpp
	VideoTableWidget.cpp
	VidDialog.cpp
    DeviceWarnDialog.cpp
	MsgBoxDialog.cpp
	Player.cpp
	Downloader.cpp
	MultiPlayerDialog.cpp
	)
if(WIN32)
	set(app_HEADERS
		${app_HEADERS}
		frameless-helper.h
	)
	set(app_PLATFORM_SOURCES 
		platform-window.cpp
		frameless-helper.cpp
	)
endif()

set(app_ui_FILES
	${app_ui_FILES}
	forms/MainWindow.ui
	forms/LoginDialog.ui
	forms/ParamDialog.ui
	forms/SettingDialog.ui
	forms/VidDialog.ui
	forms/DeviceWarnDialog.ui
	forms/MsgBoxDialog.ui
	forms/MultiPlayerDialog.ui
	)
set(app_qrc_FILES
	${app_qrc_FILES}
	Resource.qrc
	)
set(app_data_FILES
	${app_data_FILES}
	${CAINFO_FILE}
	data/locale/en-US.ini
	data/locale/zh-CN.ini
	data/themes/Default.qss
	)

qt5_wrap_ui(qt_UI_SOURCES ${app_ui_FILES})
qt5_add_resources(qt_QRC_SOURCES ${app_qrc_FILES})

include(ResourceInfo)
if(MSVC)
	set(RESOURCE_FILE ${APP_GENERATE_DIR}/resource.rc)
	configure_file(${CMAKE_MODULE_PATH}/resource_demo.rc.in ${RESOURCE_FILE} @ONLY)

	# configure_file only support UTF-8, need encoding UTF-16LE
	configure_file(${CMAKE_MODULE_PATH}/resource_demo.rc.in ${APP_GENERATE_DIR}/resource-utf-8.rc @ONLY)

	# by python
	find_program(Python3_EXECUTABLE "py.exe")
	if(NOT Python3_EXECUTABLE_FOUND)
		find_package(Python3 QUIET)
	endif()
	if(Python3_EXECUTABLE)
		message(STATUS "Python3_EXECUTABLE:" ${Python3_EXECUTABLE})
		set(SRC_FILE "${APP_GENERATE_DIR}/resource-utf-8.rc")
		set(DST_FILE "${APP_GENERATE_DIR}/resource.rc")
		execute_process(
			COMMAND "${Python3_EXECUTABLE}"
				"${CMAKE_MODULE_PATH}/encode_change.py"
				"--input=\"${SRC_FILE}\""
				"--output=\"${DST_FILE}\""
				"--input-encoding=utf-8"
				"--output-encoding=utf-16"
			RESULT_VARIABLE RESULT)
		if(NOT "${RESULT}" STREQUAL "0")
			message(FATAL_ERROR "encoding convert failed: ${DST_FILE}")
		endif()
	else()
		message(WARNING "could not find python executable, it will used the default one")
	endif()
 
    list(APPEND app_UI_PLATFORM_SOURCES ${RESOURCE_FILE})
elseif (APPLE)
	set(BUNDLE_ICON_NAME "logo.icns")
	set(APP_ICON ${CMAKE_MODULE_PATH}/${BUNDLE_ICON_NAME})
	set(MACOSX_BUNDLE_ICON_FILE ${BUNDLE_ICON_NAME})
	set(app_UI_PLATFORM_SOURCES "${APP_ICON}")

	set_source_files_properties(${APP_ICON} PROPERTIES
    	MACOSX_PACKAGE_LOCATION "Resources")

	set(RESOURCE_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}.dir")
	set(RESOURCE_FILE ${RESOURCE_FILE_PATH}/Info.plist)
	configure_file(${CMAKE_MODULE_PATH}/Info.plist.in ${RESOURCE_FILE} @ONLY)
	list(APPEND app_UI_PLATFORM_SOURCES ${RESOURCE_FILE})

	set_source_files_properties(
        ${RESOURCE_FILE} PROPERTIES 
		MACOSX_BUNDLE_INFO_PLIST "Resources")
	set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual")
endif()

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
	${app_HEADERS}
	${app_SOURCES}
	${app_PLATFORM_SOURCES}
	${app_UI_PLATFORM_SOURCES}
	${qt_UI_SOURCES}
	${qt_QRC_SOURCES}
	${app_data_FILES}
 )
# add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
#    COMMAND ${CMAKE_COMMAND} -D "OUTPUT_DIR=${GENERATE_HEADER_DIR}" -P cmake/prebuild_define.cmake
#    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
#    VERBATIM)

source_group("ui" FILES ${app_ui_FILES} ${app_qrc_FILES} ${app_UI_PLATFORM_SOURCES})
source_group("data" FILES ${app_data_FILES})

set_source_files_properties(${CAINFO_FILE} PROPERTIES
    VS_COPY_TO_OUT_DIR "Always")

set(RESOURCE_FILES
    ${CAINFO_FILE})
set_target_properties(${PROJECT_NAME}
    PROPERTIES RESOURCE "${RESOURCE_FILES}")
#set_target_properties(${PROJECT_NAME}
#	PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")

if(WIN32)
	if(Python3_EXECUTABLE)
		add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
			COMMAND "${Python3_EXECUTABLE}"
				"${CMAKE_MODULE_PATH}/add_git_hash_to_rc.py"
				"--rc-file=\"${RESOURCE_FILE}\""
				"--encoding=utf-16"
				"--app-ver-major=${CMAKE_PROJECT_VERSION_MAJOR}"
				"--app-ver-minor=${CMAKE_PROJECT_VERSION_MINOR}"
				"--app-ver-patch=${CMAKE_PROJECT_VERSION_PATCH}"
			WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
			VERBATIM)
	endif()
elseif(APPLE)
	add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
		COMMAND bash
			"${CMAKE_MODULE_PATH}/add_git_hash_to_info_plist_version.sh"
			"${PLV_PLAYER_SDK_VERSION}"
			"${RESOURCE_FILE_PATH}"
		VERBATIM)
endif()

foreach(qt_module ${QT_DEPENDENCY})
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::${qt_module})
    target_compile_definitions(${PROJECT_NAME} PUBLIC ${Qt5${qt_module}_COMPILE_DEFINITIONS})
    # print_list(Qt5${qt_module}_DEFINITIONS "Qt5${qt_module}_DEFINITIONS:")
    # print_list(Qt5${qt_module}_COMPILE_DEFINITIONS "Qt5${qt_module}_COMPILE_DEFINITIONS:")
    # print_list(Qt5${qt_module}_EXECUTABLE_COMPILE_FLAGS "Qt5${qt_module}_EXECUTABLE_COMPILE_FLAGS:")
    # print_list(Qt5${qt_module}_PLUGINS "Qt5${qt_module}_PLUGINS:")
endforeach()


target_link_libraries(${PROJECT_NAME} PRIVATE log http)

if(WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${PLAYER_SDK_PATH}/windows/${WLIB_TARGET}/lib/plv-player-sdk.lib)
	set(PLAYERSDK_INCLUDE_DIRS ${PLAYER_SDK_PATH}/windows/${WLIB_TARGET}/include)
endif()
if(APPLE)
    find_library(CORE_FOUNDATION CoreFoundation)
    find_library(CORE_SERVICES CoreServices)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CORE_FOUNDATION})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CORE_SERVICES})
	target_link_libraries(${PROJECT_NAME} PRIVATE ${PLAYER_SDK_PATH}/mac/lib/libplv-player-sdk.dylib)
	set(PLAYERSDK_INCLUDE_DIRS ${PLAYER_SDK_PATH}/mac/include)

	set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,${PLAYER_SDK_PATH}/mac/lib")
endif()
target_include_directories(${PROJECT_NAME} PRIVATE ${PLAYERSDK_INCLUDE_DIRS})


if(PLV_PLAYER_SDK_INSIDE)
	add_dependencies(${PROJECT_NAME} BUILD_SDK)
else()
	include(Depend)
	add_dependencies(${PROJECT_NAME} Depend)

	include(PlvPlayerSDK)
	add_dependencies(${PROJECT_NAME} PlvPlayerSDK)
endif()


include(Helpers)
include(CMakePrintHelpers)
get_all_targets(ALL_TARGETS)
cmake_print_variables(ALL_TARGETS)

print_target_property(${PROJECT_NAME} INCLUDE_DIRECTORIES)
print_target_property(${PROJECT_NAME} LINK_LIBRARIES)

mark_as_advanced(BUILD_WINDEPLOY)
option(BUILD_WINDEPLOY "Build win deploy" ON)
if (MSVC)
	if(BUILD_WINDEPLOY)
		set(SDK_PATH "${PLAYER_SDK_PATH}/windows/${WLIB_TARGET}/lib")
		set(LIBSDK_DLL "${SDK_PATH}/*.dll")
		STRING(REGEX REPLACE "/" "\\\\" LIBSDK_DLL ${LIBSDK_DLL})

		set(APPOUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIGURATION>")
		STRING(REGEX REPLACE "/" "\\\\" APPOUTPUT_PATH ${APPOUTPUT_PATH})
	
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND xcopy /y
				"${LIBSDK_DLL}"
				"${APPOUTPUT_PATH}"	
		VERBATIM)

		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
			COMMAND "${QTDIR}/bin/windeployqt" $<TARGET_FILE:${PROJECT_NAME}> # --verbose=2
			VERBATIM)
	endif()
endif()


