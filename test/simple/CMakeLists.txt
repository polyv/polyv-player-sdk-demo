project(plv-player-sdk-simple-demo VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(${PROJECT_NAME})
if(PLV_PLAYER_SDK_INSIDE)
	add_dependencies(${PROJECT_NAME} plv-player-node)
else()
	include(Depend)
	add_dependencies(${PROJECT_NAME} Depend)
	include(PlvPlayerSDK)
	add_dependencies(${PROJECT_NAME} PlvPlayerSDK)
endif()
list(APPEND CMAKE_PREFIX_PATH "${QTDIR}")
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
message(STATUS "Qt Core Version: ${Qt${QT_VERSION_MAJOR}Core_VERSION}")
find_package(CurlAndSsl REQUIRED)
message(STATUS "CRYPTO_INCLUDE_DIRS:${CRYPTO_INCLUDE_DIRS},CLIENT_INCLUDE_DIRS:${CLIENT_INCLUDE_DIRS}")
message(STATUS "CRYPTO_LIBS:${CRYPTO_LIBS},CLIENT_LIBS:${CLIENT_LIBS},ZLIB_LIBS:${ZLIB_LIBS}")
if(WIN32)
	set(PLV_PLAYER_SDK_INCLUDE "${CMAKE_BINARY_DIR}/plv-player-sdk/${SDK_RELATIVE_PATH}/include")
	set(PLV_PLAYER_SDK_LIB "${CMAKE_BINARY_DIR}/plv-player-sdk/${SDK_RELATIVE_PATH}/lib/plv-player-sdk.lib")
elseif(APPLE)
	set(PLV_PLAYER_SDK_INCLUDE "${CMAKE_BINARY_DIR}/plv-player-sdk/${SDK_RELATIVE_PATH}/include")
	set(PLV_PLAYER_SDK_LIB "${CMAKE_BINARY_DIR}/plv-player-sdk/${SDK_RELATIVE_PATH}/lib/libplv-player-sdk.dylib")
endif()

set(PROJECT_SOURCES
	main.cpp
	mainwindow.cpp
	mainwindow.h
	util.h
	util.cpp
	playimage.h
	playimage.cpp
	mainwindow.ui
	simple.qrc)
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE
	${CRYPTO_INCLUDE_DIRS}
	${CLIENT_INCLUDE_DIRS}
	${PLV_PLAYER_SDK_INCLUDE}
	${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(${PROJECT_NAME} PRIVATE
	Qt${QT_VERSION_MAJOR}::Widgets
	${CRYPTO_LIBS}
	${CLIENT_LIBS}
	${ZLIB_LIBS}
	${PLV_PLAYER_SDK_LIB}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE
	USE_OPENSSL
	CURL_STATICLIB
	NOMINMAX
)
set_target_properties(${PROJECT_NAME} PROPERTIES
	WIN32_EXECUTABLE TRUE
	MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
	MACOSX_BUNDLE TRUE
	MACOSX_BUNDLE_GUI_IDENTIFIER "net.polyv.${PROJECT_NAME}"
	MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
	MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
	XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
)
if(WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 crypt32 winmm wldap32)
elseif(APPLE)
	find_library(COREFOUNDATION CoreFoundation)
	find_library(SYSTEMCONFIGURATION SystemConfiguration)
	find_library(SECURITY Security)
	target_link_libraries(${PROJECT_NAME} PRIVATE
		${COREFOUNDATION}
		${SYSTEMCONFIGURATION}
		${SECURITY}
	)
endif()

#install rundir
if(WIN32)
	#msvc
	string(FIND ${CMAKE_AR} "/VC/Tools/MSVC" FIND_MSVC)
	string(SUBSTRING ${CMAKE_AR} 0 ${FIND_MSVC} MSCV_PATH)
	file(READ "${MSCV_PATH}/VC/Auxiliary/Build/Microsoft.VCRedistVersion.default.txt" MSVC_REDIST_VER)
	string(STRIP ${MSVC_REDIST_VER} MSVC_REDIST_VER)
	set(MSCV_REDIST_PATH "${MSCV_PATH}/VC/Redist/MSVC/${MSVC_REDIST_VER}/${WLIB_TARGET}/Microsoft.VC${MSVC_TOOLSET_VERSION}.CRT")
	message("MSCV_REDIST_PATH: ${MSCV_REDIST_PATH}")
	file(GLOB MSCV_REDIST_FILES "${MSCV_REDIST_PATH}/*.dll")
	foreach(item IN LISTS MSCV_REDIST_FILES)
		install(FILES ${item} DESTINATION "." COMPONENT rundir EXCLUDE_FROM_ALL)
	endforeach()
	#ucrt
	set(UCRT_PATH "C:/Program Files (x86)/Windows Kits/10/Redist/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/ucrt/DLLs/${WLIB_TARGET}")
	message("UCRT_PATH: ${UCRT_PATH}")
	file(GLOB UCRT_FILES "${UCRT_PATH}/*.dll")
	foreach(item IN LISTS UCRT_FILES)
		install(FILES ${item} DESTINATION "." COMPONENT rundir EXCLUDE_FROM_ALL)
	endforeach()
	#install
	install(FILES "$<TARGET_FILE:${PROJECT_NAME}>" DESTINATION "." COMPONENT rundir EXCLUDE_FROM_ALL)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR} --config $<CONFIG> --prefix "${CMAKE_BINARY_DIR}/rundir/$<CONFIG>" --component rundir
		COMMAND "${QTDIR}/bin/windeployqt" "${CMAKE_BINARY_DIR}/rundir/$<CONFIG>/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
		COMMENT "installing rundir and windeployqt"
	VERBATIM)
else()
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources"
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${CMAKE_BINARY_DIR}/plv-player-sdk/${SDK_RELATIVE_PATH}/lib/cacert.pem"
			"$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>/Contents/Resources"
	VERBATIM)
endif()
