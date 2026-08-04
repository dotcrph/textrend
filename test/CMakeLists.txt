include(CTest)
enable_testing()

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

# Prevent overriding compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(
    SourceFiles
    "test_math.cpp"
    "test_obj_parser.cpp"
    "test_transform.cpp"
    "test_utils.cpp"
)

add_executable(run_tests ${SourceFiles})
target_link_libraries(run_tests PRIVATE GTest::gtest_main)
target_link_libraries(run_tests PRIVATE textrend_lib)
target_include_directories(run_tests PRIVATE "${CMAKE_SOURCE_DIR}/src")

include(GoogleTest)
gtest_discover_tests(run_tests)
