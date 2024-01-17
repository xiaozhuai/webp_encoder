cmake_minimum_required(VERSION 3.24)
project(webp_encoder)

set(CMAKE_CXX_STANDARD 17)

find_package(WebP CONFIG REQUIRED)

file(GLOB_RECURSE LIB_SOURCES "src/*.h" "src/*.hpp" "src/*.c" "src/*.cpp" "src/*.cc")
file(GLOB_RECURSE CLI_SOURCES "cli/*.h" "cli/*.hpp" "cli/*.c" "cli/*.cpp" "cli/*.cc")
file(GLOB_RECURSE WASM_SOURCES "wasm/*.h" "wasm/*.hpp" "wasm/*.c" "wasm/*.cpp" "wasm/*.cc")

add_library(lib${PROJECT_NAME} STATIC ${LIB_SOURCES})
target_include_directories(lib${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(lib${PROJECT_NAME} PRIVATE WebP::webp WebP::libwebpmux)
set_target_properties(lib${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
    add_executable(${PROJECT_NAME} ${WASM_SOURCES})
    target_link_libraries(${PROJECT_NAME} PRIVATE embind)
    target_compile_options(${PROJECT_NAME} PRIVATE)
    target_link_options(${PROJECT_NAME} PRIVATE
            -sEXPORT_NAME="WebpEncoderWasm"
            -sALLOW_MEMORY_GROWTH=1
            -sNO_FILESYSTEM=1
            -sASSERTIONS=1
            -sSINGLE_FILE=1
            -sMODULARIZE=1
            -sASYNCIFY=1)
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

target_link_libraries(${PROJECT_NAME} PRIVATE lib${PROJECT_NAME})

find_program(CLANG_FORMAT clang-format)
if (CLANG_FORMAT)
    add_custom_target(
            format
            COMMAND "${CLANG_FORMAT}" -i -verbose ${LIB_SOURCES} ${CLI_SOURCES} ${WASM_SOURCES}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif ()
