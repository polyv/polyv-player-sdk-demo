
include(ExternalProject)
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty)

if(MSVC)
    if (CMAKE_CL_64)
        set(DOWNLOAD_DEPEND_URL https://soft.polyv.net/soft/VodPCPlayer/depend/2022-12-15/x64-windows-2022-12-15.7z)
        set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty/x64-windows)
    else()
        set(DOWNLOAD_DEPEND_URL https://soft.polyv.net/soft/VodPCPlayer/depend/2022-12-15/x86-windows-2022-12-15.7z)
        set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty/x86-windows)
    endif()
elseif(APPLE)
    set(DOWNLOAD_DEPEND_URL https://soft.polyv.net/soft/VodPCPlayer/depend/2022-12-15/x64-osx-2022-12-15.7z)
    set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/thirdparty/x64-osx)
endif()

ExternalProject_Add(
    Depend
    URL ${DOWNLOAD_DEPEND_URL}
    #DOWNLOAD_NAME PCPlayerSDK-Win-x86.7z
    DOWNLOAD_DIR ${SOURCE_LOCATION}
    SOURCE_DIR ${THIRD_PARTY_DIR}
    #--Configure step-------------
    CONFIGURE_COMMAND ""
    #--Build step-----------------
    BUILD_COMMAND ""
    #--Install step---------------
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
    LOG_DOWNLOAD 1 LOG_UPDATE 1 LOG_CONFIGURE 1 LOG_BUILD 1 LOG_INSTALL 1
    )