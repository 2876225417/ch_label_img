include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG        v1.17.0
)

set(gtest_force_shared_crt ON  CACHE BOOL "" FORCE)
set(BUILD_GMOCK            OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST            ON  CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)



add_executable(hash_tests
    

)














pretty_message(DEBUG "BUILD_TESTS=ON. Build with tests.")
pretty_message(VINFO_BANNER "Tests Info" "=" ${BANNER_WIDTH})
pretty_message_kv(VINFO "gtest_force_shared_crt" "${gtest_force_shared_crt}")
pretty_message_kv(VINFO "BUILD_GMOCK"            "${BUILD_GMOCK}")
pretty_message_kv(VINFO "BUILD_GTEST"            "${BUILD_GTEST}")
pretty_message(VINFO_LINE "=" ${BANNER_WIDTH})
pretty_message(STATUS "")
