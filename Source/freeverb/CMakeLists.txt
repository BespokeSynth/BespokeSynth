project(freeverb VERSION 0.0.0 LANGUAGES CXX)

add_library(${PROJECT_NAME}
        allpass.cpp
        comb.cpp
        revmodel.cpp
        )

add_library(bespoke::${PROJECT_NAME} ALIAS ${PROJECT_NAME})