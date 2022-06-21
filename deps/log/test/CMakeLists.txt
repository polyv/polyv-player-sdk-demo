project(log-test)

add_definitions(-DLOG_STATIC)

set(log-test_HEADERS
    )
set(log-test_SOURCES
    main.cpp
    )

add_executable(${PROJECT_NAME}
    ${log-test_HEADERS}
    ${log-test_SOURCES}
    )

target_link_libraries(${PROJECT_NAME}
    log
    )

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
    )
