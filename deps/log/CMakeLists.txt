project(log)

set(log_include_HEADERS
    log/log.h
    )
set(log_HEADERS
    ${log_include_HEADERS}
    src/log-def.h
    )
set(log_SOURCES
    ${log_SOURCES}
    src/log.cpp
    )


#add_definitions(-DLOG_STATIC)

add_library(${PROJECT_NAME} STATIC
    ${log_HEADERS}
    ${log_SOURCES}
    )

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
 )
target_link_libraries(${PROJECT_NAME})

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
mark_as_advanced(BUILD_LOG_TESTS)
option(BUILD_LOG_TESTS "Build test directory" OFF)
if(BUILD_LOG_TESTS)
    add_subdirectory(test)
    set_target_properties(log-test PROPERTIES FOLDER "deps")
endif()
set_target_properties(log PROPERTIES FOLDER "deps")

