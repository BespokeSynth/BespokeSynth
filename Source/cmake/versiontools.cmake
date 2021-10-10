# This is a multi-role CMake file. The IF branch runs on include, the ELSE
# branch runs in script mode or when re-included after setting BESPOKESRC.
if(CMAKE_PARENT_LIST_FILE AND NOT BESPOKESRC)
    option(BESPOKE_RELIABLE_VERSION_INFO "Update version info on every build (off: generate only at configuration time)" OFF)
    function(bespoke_buildtime_version_info TARGET)
        configure_file(VersionInfo.cpp.in geninclude/VersionInfo.cpp)
        target_sources(${TARGET} PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/geninclude/VersionInfo.cpp
            ${CMAKE_CURRENT_BINARY_DIR}/geninclude/VersionInfoBld.cpp
            )
        if(BESPOKE_RELIABLE_VERSION_INFO)
            add_custom_target(version-info BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/geninclude/VersionInfoBld.cpp
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMAND ${CMAKE_COMMAND} -D CMAKE_PROJECT_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR}
                -D CMAKE_PROJECT_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR}
                -D CMAKE_SYSTEM_PROCESSOR="${CMAKE_SYSTEM_PROCESSOR}"
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
    message(STATUS "  buildarch is ${CMAKE_SYSTEM_PROCESSOR}")

    string(TIMESTAMP BESPOKE_BUILD_DATE "%Y-%m-%d")
    string(TIMESTAMP BESPOKE_BUILD_TIME "%H:%M:%S")
    set(BESPOKE_BUILD_ARCH "${CMAKE_SYSTEM_PROCESSOR}")

    message(STATUS "Configuring ${BESPOKEBLD}/geninclude/VersionInfoBld.cpp")
    configure_file(${BESPOKESRC}/Source/VersionInfoBld.cpp.in ${BESPOKEBLD}/geninclude/VersionInfoBld.cpp)
endif()
