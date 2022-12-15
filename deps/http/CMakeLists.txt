project(http)

find_package(CurlAndSsl REQUIRED)
add_definitions(-DUSE_OPENSSL)
add_definitions(-DCURL_STATICLIB)

set(http_HEADERS
	${http_HEADERS}
	http/http-code.h
	http/http-param.h
	http/http-client.h
	http/http-manager.h
	)
set(http_SOURCES
	${http_SOURCES}
	src/http-code.cpp
	src/http-param.cpp
	src/http-client.cpp
	src/http-manager.cpp
	)

add_library(${PROJECT_NAME} STATIC
	${http_HEADERS}
	${http_SOURCES}
	)

target_include_directories(${PROJECT_NAME} PRIVATE ${CRYPTO_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${CLIENT_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}
PUBLIC ${CMAKE_CURRENT_LIST_DIR}
	#PUBLIC ${http_HEADERS}
	)

if(WIN32)
	target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 crypt32)
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
target_link_libraries(${PROJECT_NAME} PRIVATE ${ZLIB_LIBS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${CRYPTO_LIBS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${CLIENT_LIBS})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
mark_as_advanced(BUILD_HTTP_TESTS)
option(BUILD_HTTP_TESTS "Build test directory" OFF)
if(BUILD_HTTP_TESTS)
    add_subdirectory(test)
    set_target_properties(http-test PROPERTIES FOLDER "deps")
endif()
set_target_properties(http PROPERTIES FOLDER "deps")
