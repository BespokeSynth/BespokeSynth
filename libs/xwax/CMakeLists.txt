project(xwax VERSION 0.0.0 LANGUAGES CXX)

add_library(${PROJECT_NAME}
        src/lut.c
        src/timecoder.c
        )

# This means clients have to say "include xwax/foo" but internally we say "include foo"
target_include_directories(${PROJECT_NAME} PUBLIC .)
target_include_directories(${PROJECT_NAME} PRIVATE xwax)

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE BESPOKE_WINDOWS=1)
endif()

add_library(bespoke::${PROJECT_NAME} ALIAS ${PROJECT_NAME})