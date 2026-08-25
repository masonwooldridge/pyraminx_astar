cmake_minimum_required(VERSION 3.20)

project(pyraminx_solver VERSION 1.0.0 LANGUAGES CXX)

option(PYRAMINX_BUILD_TESTS "Build the test suite" ON)
option(PYRAMINX_BUILD_BENCHMARKS "Build the benchmark runner" ON)
option(PYRAMINX_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(PYRAMINX_USE_GTEST "Fetch GoogleTest and build its test suite" ON)

add_library(pyraminx
    src/state.cpp
    src/heuristic.cpp
    src/solver.cpp
)
add_library(pyraminx::pyraminx ALIAS pyraminx)

target_include_directories(pyraminx PUBLIC include)
target_compile_features(pyraminx PUBLIC cxx_std_20)
target_compile_options(pyraminx PRIVATE
    $<$<CXX_COMPILER_ID:Clang,GNU>:-Wall;-Wextra;-Wpedantic;-Wconversion>
    $<$<CXX_COMPILER_ID:MSVC>:/W4;/permissive->
)

if(PYRAMINX_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(pyraminx PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(pyraminx PUBLIC -fsanitize=address,undefined)
endif()

add_executable(pyraminx-cli src/cli.cpp)
target_link_libraries(pyraminx-cli PRIVATE pyraminx::pyraminx)

if(EMSCRIPTEN)
    add_executable(pyraminx-web web/bindings.cpp)
    target_link_libraries(pyraminx-web PRIVATE pyraminx::pyraminx)
    set_target_properties(pyraminx-web PROPERTIES
        OUTPUT_NAME pyraminx
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/web/dist"
        SUFFIX ".js"
    )
    target_link_options(pyraminx-web PRIVATE
        "-sMODULARIZE=1"
        "-sEXPORT_NAME=createPyraminxModule"
        "-sEXPORTED_RUNTIME_METHODS=['ccall']"
        "-sALLOW_MEMORY_GROWTH=1"
    )
endif()

if(PYRAMINX_BUILD_TESTS)
    enable_testing()
    add_executable(pyraminx-tests tests/pyraminx_test.cpp)
    target_link_libraries(pyraminx-tests PRIVATE pyraminx::pyraminx)
    add_test(NAME pyraminx-tests COMMAND pyraminx-tests)

    if(PYRAMINX_USE_GTEST)
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
        add_executable(pyraminx-gtests tests/google_test.cpp)
        target_link_libraries(pyraminx-gtests PRIVATE pyraminx::pyraminx GTest::gtest_main)
        include(GoogleTest)
        gtest_discover_tests(pyraminx-gtests)
    endif()
endif()

if(PYRAMINX_BUILD_BENCHMARKS)
    add_executable(pyraminx-benchmark benchmarks/benchmark.cpp)
    target_link_libraries(pyraminx-benchmark PRIVATE pyraminx::pyraminx)
endif()

install(TARGETS pyraminx pyraminx-cli)
