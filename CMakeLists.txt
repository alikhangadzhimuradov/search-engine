cmake_minimum_required(VERSION 3.28)
project(search_engine)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(search_engine
        src/main.cpp
        src/ConverterJSON.cpp
        src/InvertedIndex.cpp
        src/SearchServer.cpp
)

target_include_directories(search_engine PUBLIC include)
target_link_libraries(search_engine PRIVATE nlohmann_json::nlohmann_json)

file(COPY
        ${CMAKE_CURRENT_SOURCE_DIR}/config.json
        ${CMAKE_CURRENT_SOURCE_DIR}/requests.json
        ${CMAKE_CURRENT_SOURCE_DIR}/documents
        DESTINATION ${CMAKE_BINARY_DIR}
)

add_executable(tests
        test/test.cpp
        src/ConverterJSON.cpp
        src/InvertedIndex.cpp
        src/SearchServer.cpp
)

target_include_directories(tests PRIVATE include)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

target_link_libraries(tests PRIVATE
        nlohmann_json::nlohmann_json
        GTest::gtest_main
        GTest::gmock
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(tests)