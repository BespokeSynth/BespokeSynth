# libusb (https://libusb.info), used by the Ableton Push 2 and Move display libraries (libs/push2, libs/abletonmove).
# vendored from the libusb-1.0.30 release: libusb/ is the library's source, with only the backends bespoke builds for,
# and config/ holds the config.h files that the release provides for each platform. LGPL 2.1, see COPYING.
#
# on linux (and mingw), the system's libusb is used instead. sets BESPOKE_LIBUSB_FOUND in the parent scope.

project(bespoke_libusb C)

if(APPLE OR MSVC OR ANDROID)
    add_library(${PROJECT_NAME} STATIC
        libusb/core.c
        libusb/descriptor.c
        libusb/hotplug.c
        libusb/io.c
        libusb/strerror.c
        libusb/sync.c
        )
    target_include_directories(${PROJECT_NAME} PUBLIC libusb PRIVATE libusb/os)

    if(APPLE)
        target_sources(${PROJECT_NAME} PRIVATE
            libusb/os/darwin_usb.c
            libusb/os/events_posix.c
            libusb/os/threads_posix.c
            )
        target_include_directories(${PROJECT_NAME} PRIVATE config/darwin)
        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework IOKit" "-framework CoreFoundation" "-framework Security")
    elseif(MSVC)
        target_sources(${PROJECT_NAME} PRIVATE
            libusb/os/events_windows.c
            libusb/os/threads_windows.c
            libusb/os/windows_common.c
            libusb/os/windows_usbdk.c
            libusb/os/windows_winusb.c
            )
        target_include_directories(${PROJECT_NAME} PRIVATE config/msvc)
    else() # android
        # android apps can't list usb devices, so bespoke opens them through android's usb api and gives libusb the
        # file descriptor (see libusb_wrap_sys_device). linux_netlink.c is needed to build, but isn't used then
        target_sources(${PROJECT_NAME} PRIVATE
            libusb/os/events_posix.c
            libusb/os/linux_netlink.c
            libusb/os/linux_usbfs.c
            libusb/os/threads_posix.c
            )
        target_include_directories(${PROJECT_NAME} PRIVATE config/android)
        target_link_libraries(${PROJECT_NAME} PRIVATE log)
    endif()

    set(BESPOKE_LIBUSB_FOUND TRUE PARENT_SCOPE)
else()
    find_package(PkgConfig)
    pkg_check_modules(LIBUSB libusb-1.0)
    add_library(${PROJECT_NAME} INTERFACE)
    if(LIBUSB_FOUND)
        target_include_directories(${PROJECT_NAME} INTERFACE ${LIBUSB_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} INTERFACE ${LIBUSB_LIBRARIES})
    endif()
    set(BESPOKE_LIBUSB_FOUND ${LIBUSB_FOUND} PARENT_SCOPE)
endif()

add_library(bespoke::libusb ALIAS ${PROJECT_NAME})
