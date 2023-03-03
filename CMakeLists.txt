cmake_minimum_required(VERSION 3.24)
project(webp_encoder)

set(CMAKE_CXX_STANDARD 17)

find_package(WebP CONFIG REQUIRED)

file(GLOB_RECURSE LIB_SOURCES "src/*.h" "src/*.cpp")
file(GLOB_RECURSE CLI_SOURCES "cli/*.h" "cli/*.cpp")
file(GLOB_RECURSE WASM_SOURCES "wasm/*.h" "wasm/*.cpp")

add_library(libwebp_encoder STATIC ${LIB_SOURCES})
target_include_directories(libwebp_encoder PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(libwebp_encoder PRIVATE WebP::webp WebP::libwebpmux)

if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    add_executable(${PROJECT_NAME} ${WASM_SOURCES})
    target_link_libraries(${PROJECT_NAME} PRIVATE embind)
    target_compile_options(${PROJECT_NAME} PRIVATE)
    target_link_options(${PROJECT_NAME} PRIVATE -sASSERTIONS -sEXPORT_NAME="WebpEncoder" -sSINGLE_FILE=1 -sMODULARIZE=1)
    add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.js" "${CMAKE_CURRENT_SOURCE_DIR}/docs/js/"
            COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.js" "${CMAKE_CURRENT_SOURCE_DIR}/dist/"
    )
else ()
    find_package(Stb REQUIRED)
    add_executable(${PROJECT_NAME} ${CLI_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE "${Stb_INCLUDE_DIR}")
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE libwebp_encoder)
