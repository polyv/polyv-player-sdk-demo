project(http-test)


set(http-test_HEADERS
	)	
set(http-test_SOURCES
	main.cpp
	)

add_executable(${PROJECT_NAME}
	${http-test_HEADERS}
	${http-test_SOURCES}
	)
	
target_link_libraries(${PROJECT_NAME}
	http
	)

set_target_properties(${PROJECT_NAME}
	PROPERTIES
	OUTPUT_NAME "${PROJECT_NAME}"
	)