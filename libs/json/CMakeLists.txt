project(json VERSION 0.0.0 LANGUAGES CXX)

add_library(${PROJECT_NAME}
        jsoncpp/src/lib_json/json_reader.cpp
        jsoncpp/src/lib_json/json_value.cpp
        jsoncpp/src/lib_json/json_writer.cpp
        )

target_include_directories(${PROJECT_NAME} PUBLIC jsoncpp/include)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<CXX_COMPILER_ID:GNU>:-Wno-deprecated-declarations>)
add_library(bespoke::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
