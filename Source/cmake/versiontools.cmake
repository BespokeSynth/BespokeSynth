# This is a multi-role CMake file. The IF branch runs on include, the ELSE
# branch runs in script mode or when re-included after setting BESPOKESRC.
if(CMAKE_PARENT_LIST_FILE AND NOT BESPOKESRC)
    # Toggle this to on by default. here's my reasoning
    # 1. Users who know what they are doing and dont want this can turn it off
    # 2. Self build users who are struggling to build won't know to turn it off but need reliable build inf
    # so having it 'on' means a few power users add an option but the bug reports on discord get more
    # version accurate
    option(BESPOKE_RELIABLE_VERSION_INFO "Update version info on every build (off: generate only at configuration time)" ON)
    function(bespoke_buildtime_version_info TARGET)
        configure_file(VersionInfo.cpp.in geninclude/VersionInfo.cpp)
        target_sources(${TARGET} PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/geninclude/VersionInfo.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/geninclude/VersionInfoBld.cpp
            )
        set(BESPOKE_BUILD_ARCH "${MSVC_CXX_ARCHITECTURE_ID}")
        if("${BESPOKE_BUILD_ARCH}" STREQUAL "")
            set(BESPOKE_BUILD_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
        endif()
        if(BESPOKE_RELIABLE_VERSION_INFO)
            add_custom_target(version-info BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/geninclude/VersionInfoBld.cpp
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMAND ${CMAKE_COMMAND} -D CMAKE_PROJECT_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
                -D CMAKE_PROJECT_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
                -D BESPOKE_BUILD_ARCH="${BESPOKE_BUILD_ARCH}"
                -D BESPOKESRC=${CMAKE_SOURCE_DIR} -D BESPOKEBLD=${CMAKE_CURRENT_BINARY_DIR}
                -D WIN32=${WIN32}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/versiontools.cmake
                )
            add_dependencies(${TARGET} version-info)
        else()
            set(BESPOKESRC ${CMAKE_SOURCE_DIR})
            set(BESPOKEBLD ${CMAKE_CURRENT_BINARY_DIR})
            include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/versiontools.cmake)
        endif()
    endfunction()
else()
    find_package(Git)

    if (NOT EXISTS ${BESPOKESRC}/.git)
        message(STATUS "This is not a .git checkout; Defaulting versions")
        set(GIT_BRANCH "unknown-branch")
        set(GIT_COMMIT_HASH "deadbeef")
    elseif (Git_FOUND)
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY ${BESPOKESRC}
            OUTPUT_VARIABLE GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )

        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${BESPOKESRC}
            OUTPUT_VARIABLE GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )
    else()
        message(STATUS "GIT EXE not found Defaulting versions")
        set(GIT_BRANCH "built-without-git")
        set(GIT_COMMIT_HASH "deadbeef")
    endif ()

    cmake_host_system_information(RESULT BESPOKE_BUILD_FQDN QUERY FQDN)

    message(STATUS "Setting up BESPOKE version")
    message(STATUS "  git hash is ${GIT_COMMIT_HASH} and branch is ${GIT_BRANCH}")
    message(STATUS "  buildhost is ${BESPOKE_BUILD_FQDN}")
    message(STATUS "  buildarch is ${BESPOKE_BUILD_ARCH}")

    string(TIMESTAMP BESPOKE_BUILD_DATE "%Y-%m-%d")
    string(TIMESTAMP BESPOKE_BUILD_TIME "%H:%M:%S")

    message(STATUS "Configuring ${BESPOKEBLD}/geninclude/VersionInfoBld.cpp")
    configure_file(${BESPOKESRC}/Source/VersionInfoBld.cpp.in ${BESPOKEBLD}/geninclude/VersionInfoBld.cpp)
endif()
