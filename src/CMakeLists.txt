set(
    SourceFiles
    "args.cpp"
    "camera.cpp"
    "mesh.cpp"
    "screen.cpp"
    "utils.cpp"
    "obj_parser.cpp"
    "rasterizer.cpp"
    "transform.cpp"
)

if(WIN32)
    list(APPEND SourceFiles "terminal_windows.cpp")
elseif(UNIX)
    list(APPEND SourceFiles "terminal_unix.cpp")
endif()

add_library(textrend_lib STATIC ${SourceFiles})
target_include_directories(textrend_lib PRIVATE "${CMAKE_SOURCE_DIR}/src")

add_executable(textrend_app ${SourceFiles} "main.cpp")
target_include_directories(textrend_app PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_link_libraries(textrend_app PUBLIC textrend_lib)

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(textrend_lib PUBLIC /fp:fast)
else()
    target_compile_options(textrend_lib PRIVATE -ffast-math)
    target_link_options(textrend_lib PUBLIC -ffast-math) 
endif()

set_target_properties(textrend_lib PROPERTIES LINKER_TYPE LLD)
set_target_properties(textrend_app PROPERTIES LINKER_TYPE LLD)

if(CMAKE_BUILD_TYPE MATCHES "Rel")
    set_target_properties(textrend_app PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
