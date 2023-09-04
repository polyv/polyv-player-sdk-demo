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
find_package(QT NAMES Qt6 Qt5 COMPONENTS ${QT_DEPENDENCY} REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS ${QT_DEPENDENCY} REQUIRED)
message(STATUS "Found Qt5 Version:" ${Qt5Core_VERSION})

set(CAINFO_FILE ${CMAKE_MODULE_PATH}/cacert.pem)

set(APP_GENERATE_DIR ${CMAKE_CURRENT_BINARY_DIR}/generate)
set(APP_GENERATE_HEADER_DIR ${APP_GENERATE_DIR}/Headers)
configure_file(${CMAKE_MODULE_PATH}/version.h.in
    ${APP_GENERATE_HEADER_DIR}/version.h)
list(APPEND APP_GENERATED_HEAD_FILES ${APP_GENERATE_HEADER_DIR}/version.h)

file(GLOB app_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h" 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" CACHE INTERNAL "source files" FORCE
    )
list(APPEND app_SOURCES ${APP_GENERATED_HEAD_FILES})
source_group(
    "Source Files"
    FILES ${app_SOURCES}
    )
if (WIN32)
    file(GLOB app_win_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/win/*.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/win/*.cpp" CACHE INTERNAL "source files" FORCE
        )
	source_group(
        TREE "${CMAKE_CURRENT_SOURCE_DIR}/win"
        PREFIX "Win Files"
        FILES ${app_win_SOURCES}
        )
	list(APPEND app_SOURCES ${app_win_SOURCES})
elseif(APPLE)
    file(GLOB app_mac_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/mac/*.h"
		"${CMAKE_CURRENT_SOURCE_DIR}/mac/*.c"
		"${CMAKE_CURRENT_SOURCE_DIR}/mac/*.cc"
		"${CMAKE_CURRENT_SOURCE_DIR}/mac/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/mac/*.mm" CACHE INTERNAL "source files" FORCE
        )
	source_group(
        TREE "${CMAKE_CURRENT_SOURCE_DIR}/mac"
        PREFIX "Mac Files"
        FILES ${app_mac_SOURCES}
        )
	list(APPEND app_SOURCES ${app_mac_SOURCES})
endif()

file(GLOB app_UI_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/forms/*.ui" CACHE INTERNAL "qt form ui files" FORCE
    )
file(GLOB app_QRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.qrc" CACHE INTERNAL "qt qrc files" FORCE
    )
file(GLOB_RECURSE app_DATA_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/data/*.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/locale/*.ini"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/*.qss"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/themes/*.qss" CACHE INTERNAL "data files" FORCE
    )
list(APPEND app_DATA_FILES ${CAINFO_FILE})

if (NOT Qt6_FOUND)
    qt5_wrap_ui(qt_UI_FILES ${app_UI_FILES})
    qt5_add_resources(qt_QRC_FILES ${app_QRC_FILES})
else()
    qt6_wrap_ui(qt_UI_FILES ${app_UI_FILES})
    qt6_add_resources(qt_QRC_FILES ${app_QRC_FILES})
endif()

source_group("Generated Files" FILES ${qt_UI_FILES} ${qt_QRC_FILES})

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
 
    list(APPEND app_UI_FILES ${RESOURCE_FILE})
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
	list(APPEND app_UI_FILES ${RESOURCE_FILE})

	set_source_files_properties(
        ${RESOURCE_FILE} PROPERTIES 
		MACOSX_BUNDLE_INFO_PLIST "Resources")
	set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Manual")
endif()

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE
	${app_SOURCES}
	${qt_UI_FILES}
	${qt_QRC_FILES}	
	${app_UI_FILES}
	${app_QRC_FILES}
	${app_DATA_FILES}
 )
source_group("Ui Files" FILES ${app_UI_FILES} ${app_QRC_FILES})
source_group("Data Files" FILES ${app_DATA_FILES})

set(RESOURCE_FILES
    ${CAINFO_FILE})
set_target_properties(${PROJECT_NAME}
    PROPERTIES RESOURCE "${RESOURCE_FILES}")

set_source_files_properties(${CAINFO_FILE} PROPERTIES
    VS_COPY_TO_OUT_DIR "Always")

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
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
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
endforeach()

include(Helpers)
list_dir_recursively(SUB_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE log http)

if(WIN32)
	set(SDK_PATH ${PLAYER_SDK_PATH}/windows/${WLIB_TARGET}/lib)
	target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${SDK_PATH}/plv-player-sdk.lib)
	file(GLOB SDK_FILES "${SDK_PATH}/*.dll")
	set(SDK_LIB_FILE ${SDK_PATH}/plv-player-sdk.dll)
elseif(APPLE)
    find_library(CORE_FOUNDATION CoreFoundation)
    find_library(CORE_SERVICES CoreServices)
	find_library(APPLICATION_SERVICES ApplicationServices)
	find_library(COCOA Cocoa)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
		${CORE_FOUNDATION}
		${CORE_SERVICES}
		${APPLICATION_SERVICES}
		${COCOA}
		)
	set(SDK_PATH ${PLAYER_SDK_PATH}/mac/lib)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${SDK_PATH}/libplv-player-sdk.dylib)
	file(GLOB SDK_FILES "${SDK_PATH}/*.dylib")
	set(SDK_LIB_FILE ${SDK_PATH}/libplv-player-sdk.dylib)
	#set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,${PLAYER_SDK_PATH}/mac/lib")
endif()
set(PLAYERSDK_INCLUDE_DIRS ${SDK_PATH}/../include)
target_include_directories(${PROJECT_NAME} PRIVATE ${PLAYERSDK_INCLUDE_DIRS})


if(PLV_PLAYER_SDK_INSIDE)
	add_dependencies(${PROJECT_NAME} BUILD_SDK)
else()
	include(Depend)
	add_dependencies(${PROJECT_NAME} Depend)

	include(PlvPlayerSDK)
	add_dependencies(${PROJECT_NAME} PlvPlayerSDK)
endif()

include(CMakePrintHelpers)
get_all_targets(ALL_TARGETS)
cmake_print_variables(ALL_TARGETS)

print_target_property(${PROJECT_NAME} INCLUDE_DIRECTORIES)
print_target_property(${PROJECT_NAME} LINK_LIBRARIES)

if (MSVC)
	# output to bin
	file(GLOB SDK_CAINFO_FILES ${SDK_PATH}/*.pem)
	list(APPEND SDK_FILES ${SDK_CAINFO_FILES})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${SDK_FILES}
			${SDK_LIB_FILE}
			$<TARGET_FILE_DIR:${PROJECT_NAME}>
		VERBATIM)

	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND "${QTDIR}/bin/windeployqt" $<TARGET_FILE:${PROJECT_NAME}> # --verbose=2
		VERBATIM)
endif()


