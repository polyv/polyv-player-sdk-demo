if(NOT DEFINED THIRD_PARTY_PATH)
    message(FATAL_ERROR "Not defined THIRD_PARTY_PATH")
endif()
if (MSVC)
    set(WLIB_TARGET "x86")
	if (CMAKE_CL_64)
	    set(WLIB_TARGET "x64")
	endif() 
    set(CRYPTO_LIBS
        ${THIRD_PARTY_PATH}/${WLIB_TARGET}-windows/lib/libcrypto.lib
        ${THIRD_PARTY_PATH}/${WLIB_TARGET}-windows/lib/libssl.lib
        CACHE PATH "openssl libraries"
        )
    set(CRYPTO_INCLUDE_DIRS
        ${THIRD_PARTY_PATH}/${WLIB_TARGET}-windows/include
        CACHE PATH "openssl include directories"
        )
    set(CLIENT_LIBS
        ${THIRD_PARTY_PATH}/${WLIB_TARGET}-windows/lib/libcurl.lib
        CACHE PATH "libcurl libraries"
        )
    set(CLIENT_INCLUDE_DIRS
        ${THIRD_PARTY_PATH}/${WLIB_TARGET}-windows/include
        CACHE PATH "libcurl include directories"
        )
	set(ZLIB_LIBS
        ${THIRD_PARTY_PATH}/${WLIB_TARGET}-windows/lib/zlib.lib
        CACHE PATH "libcurl libraries"
        )
elseif (APPLE)
    set(CRYPTO_LIBS
        ${THIRD_PARTY_PATH}/x64-osx/lib/libcrypto.a
        ${THIRD_PARTY_PATH}/x64-osx/lib/libssl.a
        CACHE PATH "openssl libraries"
        )
    set(CRYPTO_INCLUDE_DIRS
        ${THIRD_PARTY_PATH}/x64-osx/include
        CACHE PATH "openssl include directories"
        )
    set(CLIENT_LIBS
        ${THIRD_PARTY_PATH}/x64-osx/lib/libcurl.a
        CACHE PATH "libcurl libraries"
        )
    set(CLIENT_INCLUDE_DIRS
        ${THIRD_PARTY_PATH}/x64-osx/include
        CACHE PATH "libcurl include directories"
        )
    set(ZLIB_LIBS
        ${THIRD_PARTY_PATH}/x64-osx/lib/libz.a
        CACHE PATH "libcurl libraries"
        )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CurlAndSsl DEFAULT_MSG
    CRYPTO_LIBS
    #CRYPTO_LIBS_DEPENDS
    CRYPTO_INCLUDE_DIRS
    CLIENT_LIBS
    #CLIENT_LIBS_DEPENDS
    CLIENT_INCLUDE_DIRS
)
mark_as_advanced(
    CRYPTO_LIBS
    #CRYPTO_LIBS_DEPENDS
    CRYPTO_INCLUDE_DIRS
    CLIENT_LIBS
    #CLIENT_LIBS_DEPENDS
    CLIENT_INCLUDE_DIRS
)