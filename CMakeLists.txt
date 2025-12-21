cmake_minimum_required(VERSION 3.5)

project(EASYTC VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SPDLOG_LOG_DIR  ${PROJECT_SOURCE_DIR}/3rdparty/spdlog/include)
set(ASIO_DIR  ${PROJECT_SOURCE_DIR}/3rdparty/asio/include)
set(CROW_DIR ${PROJECT_SOURCE_DIR}/3rdparty/Crow/include)

set(INCLUDE_DIR ${SPDLOG_LOG_DIR} ${ASIO_DIR} ${CROW_DIR})
include_directories(${INCLUDE_DIR})

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/install)

if(WIN32)
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_definitions(_WIN32_WINNT=0x0A00)
endif()

message(${CROW_DIR})
add_subdirectory(src)
add_subdirectory(example)
