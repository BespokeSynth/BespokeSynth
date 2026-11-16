project(abletonmove)

set(DOC_STRING "Enable Ableton Move support. When off, the abletonmovecontrol module is present but non-functional.")
if(BESPOKE_DESKTOP OR BESPOKE_ANDROID)
    set(BESPOKE_ABLETONMOVE_SUPPORT_DEFAULT ON)
else()
    set(BESPOKE_ABLETONMOVE_SUPPORT_DEFAULT OFF)
endif()
option(BESPOKE_ABLETONMOVE_SUPPORT "${DOC_STRING}" ${BESPOKE_ABLETONMOVE_SUPPORT_DEFAULT})

add_library(${PROJECT_NAME}
        JuceToMoveDisplayBridge.cpp
        Move-Display.cpp
        Move-Display.h
        Move-UsbCommunicator.h
        Result.cpp
        include/abletonmove/JuceToMoveDisplayBridge.h
        include/abletonmove/Macros.h
        include/abletonmove/Result.h
)

# libusb comes from libs/libusb, which uses the system's libusb on linux
if(BESPOKE_ABLETONMOVE_SUPPORT AND NOT BESPOKE_LIBUSB_FOUND)
    message(WARNING "libusb not found, disabling Ableton Move support")
    set(BESPOKE_ABLETONMOVE_SUPPORT OFF CACHE BOOL "${DOC_STRING}" FORCE)
endif()

if(BESPOKE_ABLETONMOVE_SUPPORT)
    target_sources(${PROJECT_NAME} PRIVATE Move-Usb-Communicator.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE bespoke::libusb)
    message(STATUS "Ableton Move support enabled")
else()
    message(STATUS "Ableton Move support disabled")
endif()

# public because Source/ includes Move-Display.h, whose layout depends on this
target_compile_definitions(${PROJECT_NAME} PUBLIC BESPOKE_ABLETONMOVE_SUPPORT=$<BOOL:${BESPOKE_ABLETONMOVE_SUPPORT}>)
target_include_directories(${PROJECT_NAME} PUBLIC include)
add_library(bespoke::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
