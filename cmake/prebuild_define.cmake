execute_process(
    COMMAND git rev-parse --short=8 HEAD
    OUTPUT_VARIABLE PROJECT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(GIT_BRANCH OR DEFINED ENV{GIT_BRANCH})
    if(NOT GIT_BRANCH AND DEFINED ENV{GIT_BRANCH})
        set(GIT_BRANCH $ENV{GIT_BRANCH})
    endif()
else()
    execute_process(
        COMMAND git branch --show-current
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

if(GIT_BRANCH MATCHES "release.*")
    set(DEV_SUFFIX "")
elseif(GIT_BRANCH MATCHES "hotfix.*")
    set(DEV_SUFFIX "")
else()
    set(DEV_SUFFIX "-dev")
endif()

message(STATUS "prebuild_define.cmake: PROJECT_COMMIT_HASH=${PROJECT_COMMIT_HASH}")
message(STATUS "prebuild_define.cmake: GIT_BRANCH=${GIT_BRANCH}")
message(STATUS "prebuild_define.cmake: DEV_SUFFIX=${DEV_SUFFIX}")

string(LENGTH "${GIT_BRANCH}" LENGTH_OF_GIT_BRANCH)
if(NOT LENGTH_OF_GIT_BRANCH)
    message(WARNING "GIT_BRANCH is empty")
    return()
endif()

set(TMP_FILE "${OUTPUT_DIR}/prebuild_defined.h.tmp")
configure_file("${CMAKE_CURRENT_LIST_DIR}/prebuild_defined.h.in" "${TMP_FILE}")

# cmake 3.21+
# file(COPY_FILE "${TMP_FILE}"
#     "${OUTPUT_DIR}/prebuild_defined.h"
#     RESULT copy_result
#     ONLY_IF_DIFFERENT)
# message(${copy_result})
# if (copy_result equal 0)
#     message(STATUS "prebuild_define.cmake: generate ${OUTPUT_DIR}/prebuild_defined.h")
# endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different "${TMP_FILE}" "${OUTPUT_DIR}/prebuild_defined.h")
message(STATUS "prebuild_define.cmake: generate ${OUTPUT_DIR}/prebuild_defined.h")
file(REMOVE "${TMP_FILE}")
