project(plv-player-sdk-demo)
set(APP_SUFFIX "32")
if (CMAKE_CL_64)
    set(APP_SUFFIX "64")
endif()

set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON) # incompatible of VS 2017
# set(CMAKE_AUTOUIC ON) # incompatible of VS 2017
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
	WindowHelper.h
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
	WindowHelper.cpp
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
	)
if(WIN32)
	set(app_PLATFORM_SOURCES 
		platform-window.cpp
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

if(WIN32)
	set(app_UI_PLATFORM_SOURCES App.rc)	
elseif(APPLE)
	set(ICON_NAME "logo.icns")
	set(app_UI_PLATFORM_SOURCES "${CMAKE_MODULE_PATH}/${ICON_NAME}")
	set(MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
	set_property(SOURCE "${app_UI_PLATFORM_SOURCES}" PROPERTY
		MACOSX_PACKAGE_LOCATION Resources)
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
if(APPLE)
	set_property(TARGET ${PROJECT_NAME} PROPERTY
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_MODULE_PATH}/Info.plist")
endif()

source_group("ui" FILES ${app_ui_FILES} ${app_qrc_FILES} ${app_UI_PLATFORM_SOURCES})
source_group("data" FILES ${app_data_FILES})

set_source_files_properties(${CAINFO_FILE} PROPERTIES
    VS_COPY_TO_OUT_DIR "Always")

set(RESOURCE_FILES
    ${CAINFO_FILE})
set_target_properties(${PROJECT_NAME}
    PROPERTIES RESOURCE "${RESOURCE_FILES}")
set_target_properties(${PROJECT_NAME}
	PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")

foreach(qt_module ${QT_DEPENDENCY})
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::${qt_module})
    target_compile_definitions(${PROJECT_NAME} PUBLIC ${Qt5${qt_module}_COMPILE_DEFINITIONS})
    # print_list(Qt5${qt_module}_DEFINITIONS "Qt5${qt_module}_DEFINITIONS:")
    # print_list(Qt5${qt_module}_COMPILE_DEFINITIONS "Qt5${qt_module}_COMPILE_DEFINITIONS:")
    # print_list(Qt5${qt_module}_EXECUTABLE_COMPILE_FLAGS "Qt5${qt_module}_EXECUTABLE_COMPILE_FLAGS:")
    # print_list(Qt5${qt_module}_PLUGINS "Qt5${qt_module}_PLUGINS:")
endforeach()

target_link_libraries(${PROJECT_NAME}
PRIVATE
	log
	http
	)
if(WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)

	set(WLIB_TARGET "x86")
	if (CMAKE_CL_64)
		set(WLIB_TARGET "x64")
	endif()
	set(PLAYERSDK_INCLUDE_DIRS ${PLAYER_SDK_PATH}/windows/include)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${PLAYER_SDK_PATH}/windows/lib/${WLIB_TARGET}/plv-player-sdk.lib)
endif()

if(APPLE)
    find_library(CORE_FOUNDATION CoreFoundation)
    find_library(CORE_SERVICES CoreServices)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CORE_FOUNDATION})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CORE_SERVICES})

	set(PLAYERSDK_INCLUDE_DIRS ${PLAYER_SDK_PATH}/mac/include)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${PLAYER_SDK_PATH}/mac/lib/libplv-player-sdk.dylib)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${PLAYERSDK_INCLUDE_DIRS})

mark_as_advanced(BUILD_WINDEPLOY)
option(BUILD_WINDEPLOY "Build win deploy" ON)
if (MSVC)
	if(BUILD_WINDEPLOY)
		set(SDK_PATH "${PLAYER_SDK_PATH}/windows/lib/${WLIB_TARGET}")
		set(LIBSDK_DLL "${SDK_PATH}/*.*")
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
elseif(APPLE)
	set(prefix "${PROJECT_NAME}.app/Contents")
	set(INSTALL_RUNTIME_DIR "${prefix}/MacOS")
	set(INSTALL_CMAKE_DIR "${prefix}/Resources")

	# based on code from CMake's QtDialog/CMakeLists.txt
	macro(install_qt5_plugin _qt_plugin_name _qt_plugins_var _prefix)
		get_target_property(_qt_plugin_path "${_qt_plugin_name}" LOCATION)
		if(EXISTS "${_qt_plugin_path}")
			get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
			get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
			get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
			set(_qt_plugin_dest "${_prefix}/PlugIns/${_qt_plugin_type}")
			install(FILES "${_qt_plugin_path}"
				DESTINATION "${_qt_plugin_dest}")
			set(${_qt_plugins_var}
				"${${_qt_plugins_var}};\$ENV{DEST_DIR}\${CMAKE_INSTALL_PREFIX}/${_qt_plugin_dest}/${_qt_plugin_file}")
		else()
			message(FATAL_ERROR "QT plugin ${_qt_plugin_name} not found")
		endif()
	endmacro()
	install_qt5_plugin("Qt5::QCocoaIntegrationPlugin" QT_PLUGINS ${prefix})
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
		"[Paths]\nPlugins = ${_qt_plugin_dir}\n")
	install(FILES "${CMAKE_CURRENT_BINARY_DIR}/qt.conf"
		DESTINATION "${INSTALL_CMAKE_DIR}")
	# Destination paths below are relative to ${CMAKE_INSTALL_PREFIX}
	install(TARGETS ${PROJECT_NAME}
	BUNDLE DESTINATION . COMPONENT Runtime
	RUNTIME DESTINATION ${INSTALL_RUNTIME_DIR} COMPONENT Runtime
	)

	# Note Mac specific extension .app
	set(APPS "\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app")

	# Directories to look for dependencies
	set(DIRS "${CMAKE_BINARY_DIR}")

	# Path used for searching by FIND_XXX(), with appropriate suffixes added
	if(CMAKE_PREFIX_PATH)
		foreach(dir ${CMAKE_PREFIX_PATH})
			list(APPEND DIRS "${dir}/bin" "${dir}/lib")
		endforeach()
	endif()
	list(APPEND DIRS "${Qt5Widgets_DIR}/../..")

	include(InstallRequiredSystemLibraries)

	message(STATUS "APPS: ${APPS}")
	message(STATUS "QT_PLUGINS: ${QT_PLUGINS}")
	message(STATUS "DIRS: ${DIRS}")

	install(CODE "include(BundleUtilities)
		fixup_bundle(\"${APPS}\" \"${QT_PLUGINS}\" \"${DIRS}\")")

	set(CPACK_GENERATOR "DRAGNDROP")
	include(CPack)
endif()


