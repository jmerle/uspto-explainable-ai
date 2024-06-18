cmake_minimum_required(VERSION 3.16)
project(uspto-explainable-ai)

set(CMAKE_CXX_STANDARD 17)
add_definitions(-DANTLR4_USE_THREAD_LOCAL_CACHE=1)

find_package(antlr4-runtime CONFIG REQUIRED)
find_package(bshoshany-thread-pool CONFIG REQUIRED)
find_package(duckdb CONFIG REQUIRED)
find_package(fast-cpp-csv-parser CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(indicators CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(roaring CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(unordered_dense CONFIG REQUIRED)

file(GLOB_RECURSE common_sources src/*.cpp)
list(FILTER common_sources EXCLUDE REGEX "\/uspto\/tools\/")

set(common_includes src)
set(common_libraries
        antlr4_static
        bshoshany-thread-pool::bshoshany-thread-pool
        fast-cpp-csv-parser::fast-cpp-csv-parser
        fmt::fmt
        indicators::indicators
        roaring::roaring
        spdlog::spdlog
        unordered_dense::unordered_dense)

file(GLOB target_files src/uspto/tools/*.cpp)
foreach (target_file ${target_files})
    get_filename_component(target_name ${target_file} NAME_WE)

    add_executable(${target_name} ${target_file} ${common_sources})
    target_include_directories(${target_name} PRIVATE ${common_includes})
    target_link_libraries(${target_name} PRIVATE ${common_libraries})
endforeach ()

target_link_libraries(reformat-patent-data PRIVATE duckdb::duckdb)
target_link_libraries(test-searcher PRIVATE nlohmann_json::nlohmann_json)

file(GLOB_RECURSE test_sources tests/*.cpp)
add_executable(tests ${common_sources} ${test_sources})
target_include_directories(tests PRIVATE ${common_includes} tests)
target_link_libraries(tests PRIVATE ${common_libraries} GTest::gtest GTest::gtest_main)
