cmake_minimum_required(VERSION 3.19)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(THIRD_PARTY_PATH ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)


cmake_policy(SET CMP0048 NEW)

project(polyv-player-sdk-demo)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/MP)
    add_compile_definitions(UNICODE _UNICODE _CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /D_DEBUG=1")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG=1")
elseif (APPLE)
    # list(APPEND COMPILER_FLAGS "-fno-exceptions" "-fPIC" "-fno-rtti") # todo: confirm
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG=1")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG=1")
endif()

if(MSVC)
    # TODO: check
    # generate pdb
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Zi")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    # /DEBUG:FULL make the release execution can be attached by debugger
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG")
    set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "${CMAKE_MODULE_LINKER_FLAGS_RELEASE} /DEBUG")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /DEBUG")
endif()

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
endif()

configure_file("${CMAKE_SOURCE_DIR}/cmake/ALL_BUILD.vcxproj.user.in"
 "${CMAKE_BINARY_DIR}/ALL_BUILD.vcxproj.user")

if (WIN32)
elseif(APPLE)
    include(Helpers)
    append_brew_pkg_path_to_env(curl)
    append_brew_pkg_path_to_env(openssl@1.1)
endif()
add_subdirectory(deps)

set(PLAYER_SDK_PATH ${CMAKE_CURRENT_SOURCE_DIR}/plv-player-sdk)
add_subdirectory(test)

