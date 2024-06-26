cmake_minimum_required(VERSION 3.24)
project(webp_encoder)

set(CMAKE_CXX_STANDARD 17)

find_package(WebP CONFIG REQUIRED)

file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.hpp" "src/*.c" "src/*.cpp" "src/*.cc")
file(GLOB_RECURSE CLI_SOURCES CONFIGURE_DEPENDS "cli/*.h" "cli/*.hpp" "cli/*.c" "cli/*.cpp" "cli/*.cc")
file(GLOB_RECURSE WASM_SOURCES CONFIGURE_DEPENDS "wasm/*.h" "wasm/*.hpp" "wasm/*.c" "wasm/*.cpp" "wasm/*.cc")

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
    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        add_custom_command(
                TARGET ${PROJECT_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.js" "${CMAKE_CURRENT_SOURCE_DIR}/docs/js/"
                COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.js" "${CMAKE_CURRENT_SOURCE_DIR}/dist/"
        )
    endif ()
else ()
    find_package(Stb REQUIRED)
    add_executable(${PROJECT_NAME} ${CLI_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE "${Stb_INCLUDE_DIR}")
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE lib${PROJECT_NAME})

find_program(CLANG_FORMAT clang-format)
if (CLANG_FORMAT)
    list(APPEND ALL_SOURCES ${LIB_SOURCES} ${CLI_SOURCES} ${WASM_SOURCES})
    add_custom_target(
            format
            COMMAND "${CLANG_FORMAT}" -i --verbose ${ALL_SOURCES}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
    add_custom_target(
            check-format
            COMMAND "${CLANG_FORMAT}" --dry-run --Werror --verbose ${ALL_SOURCES}
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif ()
