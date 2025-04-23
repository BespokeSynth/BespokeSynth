project(scalable-font2 VERSION 0.0.0 LANGUAGES CXX)

add_library(scalable-font2
        INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/ssfn.h
        )

add_library(bespoke::scalable-font2 ALIAS scalable-font2)

target_include_directories(
        scalable-font2
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(WIN32)
  target_compile_definitions(${PROJECT_NAME} PRIVATE BESPOKE_WINDOWS=1)
endif()

add_library(bespoke::${PROJECT_NAME} ALIAS ${PROJECT_NAME})