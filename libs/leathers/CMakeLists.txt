project(leathers VERSION 0.0.0 LANGUAGES CXX)

add_library(${PROJECT_NAME} INTERFACE)

# This means external code says include leathers/blah
target_include_directories(${PROJECT_NAME} INTERFACE .)

add_library(bespoke::${PROJECT_NAME} ALIAS ${PROJECT_NAME})