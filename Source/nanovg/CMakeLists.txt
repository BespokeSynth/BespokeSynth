project(nanovg VERSION 0.0.0 LANGUAGES CXX)

add_library(${PROJECT_NAME}
        nanovg.c
        )

target_include_directories(${PROJECT_NAME} PUBLIC .)

add_library(bespoke::${PROJECT_NAME} ALIAS ${PROJECT_NAME})