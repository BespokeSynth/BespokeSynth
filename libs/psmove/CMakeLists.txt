project(psmove)

set(DOC_STRING "Enable PlayStation Move support (currently macOS only)")
option(BESPOKE_PSMOVE_SUPPORT "${DOC_STRING}" ${APPLE})

add_library(${PROJECT_NAME}
    hidapi.h
    include/psmove/psmove.h
    )

if(BESPOKE_PSMOVE_SUPPORT)
    string(TOLOWER "${CMAKE_SYSTEM_NAME}" SYSNAME)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/hid_${SYSNAME}.c")
        target_sources(${PROJECT_NAME} PRIVATE "hid_${SYSNAME}.c")
    else()
        message(WARNING "No HID backend for target OS \"${CMAKE_SYSTEM_NAME}\", disabling PlayStation Move support")
        set(BESPOKE_PSMOVE_SUPPORT OFF CACHE BOOL "${DOC_STRING}" FORCE)
    endif()
endif()

if(BESPOKE_PSMOVE_SUPPORT)
    message(STATUS "PlayStation Move support enabled")
    target_sources(${PROJECT_NAME} PRIVATE psmove.c)
else()
    message(STATUS "PlayStation Move support disabled")
    target_sources(${PROJECT_NAME} PRIVATE psmove_dummy.c)
endif()

target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC include
    )
add_library(bespoke::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
