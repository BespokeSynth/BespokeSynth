project(push2)

set(DOC_STRING "Enable Ableton Push 2 support. When off, the push2control module is present but non-functional.")
if(BESPOKE_DESKTOP OR BESPOKE_ANDROID)
    set(BESPOKE_PUSH2_SUPPORT_DEFAULT ON)
else()
    set(BESPOKE_PUSH2_SUPPORT_DEFAULT OFF)
endif()
option(BESPOKE_PUSH2_SUPPORT "${DOC_STRING}" ${BESPOKE_PUSH2_SUPPORT_DEFAULT})

add_library(${PROJECT_NAME}
    JuceToPush2DisplayBridge.cpp
    Push2-Display.cpp
    Push2-Display.h
    Push2-UsbCommunicator.h
    Result.cpp
    include/push2/JuceToPush2DisplayBridge.h
    include/push2/Macros.h
    include/push2/Push2-Bitmap.h
    include/push2/Result.h
    )

# libusb comes from libs/libusb, which uses the system's libusb on linux
if(BESPOKE_PUSH2_SUPPORT AND NOT BESPOKE_LIBUSB_FOUND)
    message(WARNING "libusb not found, disabling Ableton Push 2 support")
    set(BESPOKE_PUSH2_SUPPORT OFF CACHE BOOL "${DOC_STRING}" FORCE)
endif()

if(BESPOKE_PUSH2_SUPPORT)
    target_sources(${PROJECT_NAME} PRIVATE Push2-Usb-Communicator.cpp)
    target_link_libraries(${PROJECT_NAME} PRIVATE bespoke::libusb)
    message(STATUS "Ableton Push 2 support enabled")
else()
    message(STATUS "Ableton Push 2 support disabled")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE BESPOKE_PUSH2_SUPPORT=$<BOOL:${BESPOKE_PUSH2_SUPPORT}>)
target_include_directories(${PROJECT_NAME} PUBLIC include)
add_library(bespoke::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
