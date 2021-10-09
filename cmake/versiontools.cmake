
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

if (WIN32)
    set(BESPOKE_BUILD_ARCH "x86")
else ()
    execute_process(
            COMMAND uname -m
            OUTPUT_VARIABLE BESPOKE_BUILD_ARCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif ()

cmake_host_system_information(RESULT BESPOKE_BUILD_FQDN QUERY FQDN)

message(STATUS "Setting up BESPOKE version")
message(STATUS "  git hash is ${GIT_COMMIT_HASH} and branch is ${GIT_BRANCH}")
message(STATUS "  buildhost is ${BESPOKE_BUILD_FQDN}")
message(STATUS "  buildarch is ${BESPOKE_BUILD_ARCH}")

string(TIMESTAMP BESPOKE_BUILD_DATE "%Y-%m-%d")
string(TIMESTAMP BESPOKE_BUILD_TIME "%H:%M:%S")


message(STATUS "Configuring ${BESPOKEBLD}/geninclude/VersionInfoBld.cpp")
configure_file(${BESPOKESRC}/Source/VersionInfoBld.cpp.in
        ${BESPOKEBLD}/geninclude/VersionInfoBld.cpp)

