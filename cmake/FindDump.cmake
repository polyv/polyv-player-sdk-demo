if(NOT DEFINED THIRD_PARTY_PATH)
    message(FATAL_ERROR "Not defined THIRD_PARTY_PATH")
endif()
if (MSVC)
    set(WLIB_TARGET "x86")
	if (CMAKE_CL_64)
	    set(WLIB_TARGET "x64")
	endif() 
    set(DUMP_LIBS
        ${THIRD_PARTY_PATH}/${WLIB_TARGET}-windows/lib/libdump.lib
        CACHE PATH "dump libraries"
        )
	set(DUMP_REPORT_LIBS
        ${THIRD_PARTY_PATH}/${WLIB_TARGET}-windows/lib/bugreport.exe
        CACHE PATH "dump report libraries"
        )
    set(DUMP_INCLUDE_DIRS
        ${THIRD_PARTY_PATH}/${WLIB_TARGET}-windows/include
        CACHE PATH "dump include directories"
        )
elseif (APPLE)
    return()
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Dump DEFAULT_MSG
    DUMP_LIBS
	DUMP_REPORT_LIBS
    DUMP_INCLUDE_DIRS
)
mark_as_advanced(
    DUMP_LIBS
	DUMP_REPORT_LIBS
    DUMP_INCLUDE_DIRS
)