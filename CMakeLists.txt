cmake_minimum_required(VERSION 3.14)
project(WasmProfilerAgent CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_WASM "Build for WebAssembly using Emscripten" OFF)

include(FetchContent)
FetchContent_Declare(
  flatbuffers
  GIT_REPOSITORY https://github.com/google/flatbuffers.git
  GIT_TAG v23.5.26
)
set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(flatbuffers)

set(FBS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/schema/profile.fbs)
set(FBS_GEN_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${FBS_GEN_DIR})

add_custom_command(
  OUTPUT ${FBS_GEN_DIR}/profile_generated.h
  COMMAND flatc -c -o ${FBS_GEN_DIR} ${FBS_FILE}
  DEPENDS ${FBS_FILE} flatc
  COMMENT "Generating C++ headers for profile.fbs"
)

add_custom_target(GenerateProfileHeaders DEPENDS ${FBS_GEN_DIR}/profile_generated.h)

set(COMMON_SOURCES
    src/memory_tracker.cpp
    src/stack_unwinder.cpp
    src/transport.cpp
    src/profile_serializer.cpp
    src/profiler_agent.cpp
    src/memory_pool.cpp
)

include_directories(include ${FBS_GEN_DIR} ${flatbuffers_SOURCE_DIR}/include)

if(BUILD_WASM)
    list(APPEND COMMON_SOURCES src/stack_unwinder_wasm.cpp)
    add_executable(agent ${COMMON_SOURCES})
    add_dependencies(agent GenerateProfileHeaders)
    target_link_libraries(agent flatbuffers::flatbuffers)
    
    set_target_properties(agent PROPERTIES SUFFIX ".js")
    target_link_options(agent PRIVATE 
        "-s" "ALLOW_MEMORY_GROWTH=1"
        "-s" "TOTAL_STACK=1MB"
        "-s" "TOTAL_MEMORY=16MB"
        "-s" "ASYNCIFY"
        "-s" "EXPORTED_FUNCTIONS=[\"_start_agent\",\"_stop_agent\",\"_is_agent_running\",\"_malloc\",\"_free\"]"
        "-s" "EXPORTED_RUNTIME_METHODS=[\"ccall\",\"cwrap\"]"
        "-Wl,--wrap=malloc"
        "-Wl,--wrap=free"
    )
else()
    add_executable(cli_decoder src/cli_decoder.cpp)
    add_dependencies(cli_decoder GenerateProfileHeaders)
    target_link_libraries(cli_decoder flatbuffers::flatbuffers)
    
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.14.0
    )
    FetchContent_MakeAvailable(googletest)
    
    add_executable(run_tests tests/main_test.cpp ${COMMON_SOURCES})
    add_dependencies(run_tests GenerateProfileHeaders)
    target_link_libraries(run_tests GTest::gtest flatbuffers::flatbuffers)
    
    target_compile_options(run_tests PRIVATE -fno-omit-frame-pointer)
endif()
