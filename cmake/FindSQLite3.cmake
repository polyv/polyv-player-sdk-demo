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
    set(SQLITE3_LIBS
        ${THIRD_PARTY_PATH}/x64-osx/lib/libsqlite3.a
        CACHE PATH "sqlite3 libraries"
        )
    set(SQLITE3_INCLUDE_DIRS
        ${THIRD_PARTY_PATH}/x64-osx/include
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