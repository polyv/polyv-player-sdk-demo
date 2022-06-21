if (MSVC)
    if(NOT DEFINED THIRD_PARTY_PATH)
        message(FATAL_ERROR "Not defined THIRD_PARTY_PATH")
    endif()
    set(WLIB_TARGET "x86")
	if (CMAKE_CL_64)
	set(WLIB_TARGET "x64")
	endif() 
    set(CRYPTO_LIBS
        ${THIRD_PARTY_PATH}/lib/${WLIB_TARGET}/libcrypto.lib
        ${THIRD_PARTY_PATH}/lib/${WLIB_TARGET}/libssl.lib
        CACHE PATH "openssl libraries"
        )
    set(CRYPTO_LIBS_DEPENDS
        ${THIRD_PARTY_PATH}/lib/${WLIB_TARGET}/libcrypto-1_1-x64.dll
        ${THIRD_PARTY_PATH}/lib/${WLIB_TARGET}/libssl-1_1-x64.dll
        CACHE PATH "openssl libraries depends"
        )
    set(CRYPTO_INCLUDE_DIRS
        ${THIRD_PARTY_PATH}/include
        CACHE PATH "openssl include directories"
        )
    set(CLIENT_LIBS
        ${THIRD_PARTY_PATH}/lib/${WLIB_TARGET}/libcurl.lib
        CACHE PATH "libcurl libraries"
        )
    set(CLIENT_LIBS_DEPENDS
        ${THIRD_PARTY_PATH}/lib/${WLIB_TARGET}/libcurl.dll
        ${THIRD_PARTY_PATH}/lib/${WLIB_TARGET}/zlib1.dll
        CACHE PATH "libcurl libraries depends"
        )
    set(CLIENT_INCLUDE_DIRS
        ${THIRD_PARTY_PATH}/include
        CACHE PATH "libcurl include directories"
        )
elseif (APPLE)
    find_package(CURL)
    find_package(OpenSSL)

    if(NOT CURL_FOUND)
        message(FATAL_ERROR "Could not find curl")
    endif()

    if(NOT OPENSSL_FOUND)
        message(FATAL_ERROR "Could not find openssl")
    endif()

    set(CRYPTO_LIBS ${OPENSSL_LIBRARIES}
        CACHE PATH "openssl libraries"
    )
    set(CRYPTO_LIBS_DEPENDS ${CRYPTO_LIBS}
        CACHE PATH "openssl libraries depends"
    )
    set(CRYPTO_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR}
        CACHE PATH "openssl include directories"
    )
    set(CLIENT_LIBS ${CURL_LIBRARIES}
        CACHE PATH "libcurl libraries"
    )
    set(CLIENT_LIBS_DEPENDS ${CLIENT_LIBS}
        CACHE PATH "libcurl libraries depends"
    )
    set(CLIENT_INCLUDE_DIRS ${CURL_INCLUDE_DIRS}
        CACHE PATH "libcurl include directories"
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CurlAndSsl DEFAULT_MSG
    CRYPTO_LIBS
    CRYPTO_LIBS_DEPENDS
    CRYPTO_INCLUDE_DIRS
    CLIENT_LIBS
    CLIENT_LIBS_DEPENDS
    CLIENT_INCLUDE_DIRS
)
mark_as_advanced(
    CRYPTO_LIBS
    CRYPTO_LIBS_DEPENDS
    CRYPTO_INCLUDE_DIRS
    CLIENT_LIBS
    CLIENT_LIBS_DEPENDS
    CLIENT_INCLUDE_DIRS
)