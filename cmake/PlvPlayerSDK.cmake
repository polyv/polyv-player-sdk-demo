
include(ExternalProject)
set(PLAYER_SDK_PATH ${CMAKE_BINARY_DIR}/plv-player-sdk)

if(MSVC)
    if (CMAKE_CL_64)
        set(DOWNLOAD_SDK_URL https://soft.polyv.net/soft/VodPCPlayer/PCPlayerSDK/${PLV_PLAYER_SDK_VERSION}/Win/PCPlayerSDK-Win-x64.7z )
		set(PLAYER_SDK_PATH ${CMAKE_BINARY_DIR}/plv-player-sdk/windows/x64)
    else()
        set(DOWNLOAD_SDK_URL https://soft.polyv.net/soft/VodPCPlayer/PCPlayerSDK/${PLV_PLAYER_SDK_VERSION}/Win/PCPlayerSDK-Win-x86.7z )
		set(PLAYER_SDK_PATH ${CMAKE_BINARY_DIR}/plv-player-sdk/windows/x86)
    endif()
elseif(APPLE)
    set(DOWNLOAD_SDK_URL https://soft.polyv.net/soft/VodPCPlayer/PCPlayerSDK/${PLV_PLAYER_SDK_VERSION}/Mac/PCPlayerSDK-Mac-x64.zip )
	set(PLAYER_SDK_PATH ${CMAKE_BINARY_DIR}/plv-player-sdk/mac)
endif()

ExternalProject_Add(
    PlvPlayerSDK
    URL ${DOWNLOAD_SDK_URL}
    #DOWNLOAD_NAME PCPlayerSDK-Win-x86.7z
    DOWNLOAD_DIR ${SOURCE_LOCATION}
    SOURCE_DIR ${PLAYER_SDK_PATH}
    #--Configure step-------------
    CONFIGURE_COMMAND ""
    #--Build step-----------------
    BUILD_COMMAND ""
    #--Install step---------------
    UPDATE_COMMAND "" # Skip annoying updates for every build
    INSTALL_COMMAND ""
    LOG_DOWNLOAD 1 LOG_UPDATE 1 LOG_CONFIGURE 1 LOG_BUILD 1 LOG_INSTALL 1
    )

if(WIN32)
    set(SDK_RUNTIME_FILES
        "${PLAYER_SDK_PATH}/lib/mpv-1.dll"
        "${PLAYER_SDK_PATH}/lib/plv-player-sdk.dll"
        "${PLAYER_SDK_PATH}/lib/cacert.pem")
    foreach(item IN LISTS SDK_RUNTIME_FILES)
        install(FILES ${item} DESTINATION "." COMPONENT rundir EXCLUDE_FROM_ALL)
    endforeach()
endif()
