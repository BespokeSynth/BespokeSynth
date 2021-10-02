project(push2)

set(DOC_STRING "Enable Ableton Push 2 support. When off, the push2control module is present but non-functional.")
option(BESPOKE_PUSH2_SUPPORT "${DOC_STRING}" ON)

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

if(BESPOKE_PUSH2_SUPPORT)
    # FIXME: The code currently assumes that WIN32 == MSVC and will fail to build
    # on MinGW, so we use the system libusb on there. This is the best option for
    # native builds anyway, but for cross we should build libusb for convenience.
    if(APPLE OR MSVC)
        target_sources(${PROJECT_NAME} PRIVATE
            modules/libusb/core.c
            modules/libusb/descriptor.c
            modules/libusb/hotplug.c
            modules/libusb/io.c
            modules/libusb/strerror.c
            modules/libusb/sync.c
            )
        target_include_directories(${PROJECT_NAME} PRIVATE
            libusb
            modules/libusb
            )
    endif()

    if(APPLE)
        target_sources(${PROJECT_NAME} PRIVATE
            modules/libusb/os/darwin_usb.c
            modules/libusb/os/poll_posix.c
            modules/libusb/os/threads_posix.c
            )
        target_compile_definitions(${PROJECT_NAME} PRIVATE OBJC_SILENCE_GC_DEPRECATIONS=1)
    elseif(MSVC)
        target_sources(${PROJECT_NAME} PRIVATE
            modules/libusb/os/poll_windows.c
            modules/libusb/os/threads_windows.c
            modules/libusb/os/windows_winusb.c
            modules/libusb/os/windows_nt_common.c
            )
        target_compile_definitions(${PROJECT_NAME} PRIVATE
            HAVE_STRUCT_TIMESPEC
            THREADS_WINDOWS
            )
    else()
        find_package(PkgConfig)
        pkg_check_modules(LIBUSB libusb-1.0)
        if(NOT LIBUSB_FOUND)
            message(WARNING "libusb not found, disabling Ableton Push 2 support")
            set(BESPOKE_PUSH2_SUPPORT OFF CACHE BOOL "${DOC_STRING}" FORCE)
        endif()
        target_include_directories(${PROJECT_NAME} PRIVATE ${LIBUSB_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBUSB_LIBRARIES})
    endif()
endif()

# Gotta recheck in case it was vetoed above
if(BESPOKE_PUSH2_SUPPORT)
    target_sources(${PROJECT_NAME} PRIVATE Push2-Usb-Communicator.cpp)
    message(STATUS "Ableton Push 2 support enabled")
else()
    message(STATUS "Ableton Push 2 support disabled")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE BESPOKE_PUSH2_SUPPORT=$<BOOL:${BESPOKE_PUSH2_SUPPORT}>)
target_include_directories(${PROJECT_NAME} PUBLIC include)
add_library(bespoke::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
