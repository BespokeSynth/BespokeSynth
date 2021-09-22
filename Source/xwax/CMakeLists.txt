project(xwax VERSION 0.0.0 LANGUAGES CXX)

add_library(${PROJECT_NAME}
        lut.c
        timecoder.c
        )

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE BESPOKE_WINDOWS=1)
endif()

add_library(bespoke::${PROJECT_NAME} ALIAS ${PROJECT_NAME})