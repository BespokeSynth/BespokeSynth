project(freeverb VERSION 0.0.0 LANGUAGES CXX)

add_library(${PROJECT_NAME}
        src/allpass.cpp
        src/comb.cpp
        src/revmodel.cpp
        )

# This means external code says include freeberb/blah but internal includes blah
target_include_directories(${PROJECT_NAME} PUBLIC .)
target_include_directories(${PROJECT_NAME} PRIVATE freeverb)

add_library(bespoke::${PROJECT_NAME} ALIAS ${PROJECT_NAME})