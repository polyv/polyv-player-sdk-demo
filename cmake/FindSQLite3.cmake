if(NOT DEFINED THIRD_PARTY_PATH)
    message(FATAL_ERROR "Not defined THIRD_PARTY_PATH")
endif()
if (MSVC)
    set(WLIB_TARGET "x86")
    if (CMAKE_CL_64)
        set(WLIB_TARGET "x64")
    endif() 
    set(SQLITE3_LIBS
        ${THIRD_PARTY_PATH}/${WLIB_TARGET}-windows/lib/sqlite3.lib
        CACHE PATH "sqlite3 libraries"
    )
    set(SQLITE3_INCLUDE_DIRS
        ${THIRD_PARTY_PATH}/${WLIB_TARGET}-windows/include
        CACHE PATH "sqlite3 include directories"
    )
elseif (APPLE)
    if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
        set(ARCH_PATH "arm64-osx")
    else()
        set(ARCH_PATH "x64-osx")
    endif()
    set(SQLITE3_LIBS
        ${THIRD_PARTY_PATH}/${ARCH_PATH}/lib/libsqlite3.a
        CACHE PATH "sqlite3 libraries"
        )
    set(SQLITE3_INCLUDE_DIRS
        ${THIRD_PARTY_PATH}/${ARCH_PATH}/include
        CACHE PATH "sqlite3 include directories"
        )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SQLite3 DEFAULT_MSG
    SQLITE3_LIBS
    SQLITE3_INCLUDE_DIRS
)
mark_as_advanced(
    SQLITE3_LIBS
    SQLITE3_INCLUDE_DIRS
)