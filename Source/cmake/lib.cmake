# Install resource directory to the output directory or bundle
function(bespoke_copy_resource_dir TARGET)
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_LIST_DIR}/cmake/install-quiet.cmake"
        "${CMAKE_SOURCE_DIR}/resource"
        "$<TARGET_FILE_DIR:${TARGET}>$<$<BOOL:${APPLE}>:/../Resources>"
        QUIET
        VERBATIM
        )
endfunction()

# Install dependencies to the output directory and fix up binaries
function(bespoke_make_portable TARGET)
    if(NOT BESPOKE_PORTABLE)
        return()
    endif()
    get_target_property(pyDirDst ${TARGET} RUNTIME_OUTPUT_DIRECTORY)
    set(pyMajDotMin "${Python_VERSION_MAJOR}.${Python_VERSION_MINOR}")
    if(APPLE)
        string(REGEX REPLACE "\.framework/Versions/${pyMajDotMin}/.*" ".framework/Versions/${pyMajDotMin}" pyFWDirSrc "${Python_STDLIB}")
        string(REGEX REPLACE "/Versions/${pyMajDotMin}" "" pyFWName "${pyFWDirSrc}")
        string(REGEX REPLACE ".*/" "" pyFWName "${pyFWName}")
        set(bundleContents "${pyDirDst}/${TARGET}.app/Contents")
        set(pyDirDst "${bundleContents}/Frameworks/${pyFWName}/Versions/${pyMajDotMin}")
    else()
        set(pyDirDst "${pyDirDst}/python")
    endif()

    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
        -D APPLE=${APPLE} -D MINGW=${MINGW} -D WIN32=${WIN32} -D UNIX=${UNIX}
        -D CMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}"
        -D CMAKE_INSTALL_NAME_TOOL="${CMAKE_INSTALL_NAME_TOOL}"
        -D targetBinary="$<TARGET_FILE:${TARGET}>"
        -D bundleContents="${bundleContents}"
        -D pyBulkInstallFrom="$<IF:$<BOOL:${APPLE}>,${pyFWDirSrc},${Python_STDLIB}>"
        -D pyBulkInstallTo="$<IF:$<BOOL:${APPLE}>,"${bundleContents}/Frameworks/${pyFWName}/Versions",${pyDirDst}$<$<NOT:$<BOOL:${WIN32}>>:/lib>>"
        -D pyDirDst="${pyDirDst}"
        -D pyFWDirSrc="${pyFWDirSrc}"
        -D pyFWName="${pyFWName}"
        -D pyMajDotMin="${pyMajDotMin}"
        -D Python_EXECUTABLE="${Python_EXECUTABLE}"
        -D Python_LIBRARIES="${Python_LIBRARIES}"
        -D Python_VERSION_MAJOR="${Python_VERSION_MAJOR}"
        -D Python_VERSION_MINOR="${Python_VERSION_MINOR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/bundle-python.cmake"
        )

    if(UNIX)
        set_target_properties(${TARGET} PROPERTIES
            BUILD_RPATH $<IF:$<BOOL:${APPLE}>,@loader_path/../Frameworks,$ORIGIN/python/bin>
            )
    elseif(WIN32)
        if (${CMAKE_SIZEOF_VOID_P} EQUAL 4)
            set(cpuArch "x86")
        else()
            set(cpuArch "amd64")
        endif()
        get_filename_component(pyDLL "${Python_LIBRARIES}" NAME)
        string(REGEX REPLACE "\.[Ll][Ii][Bb]$" ".dll" pyDLL "${pyDLL}")
        # Reject alien implibs such as MinGW .dll.a for now. MinGW Python has a
        # UNIX-y layout and needs different treatment.
        if("${CMAKE_MATCH_0}" STREQUAL "")
            message(FATAL_ERROR "The Python library \"${pyDLL}\" does not look \
            like an import library (.lib extension). Portable builds require \
            the official Python for Windows from python.org.")
        endif()
        configure_file(cmake/python.manifest.in python.manifest @ONLY)
        set(manifestPath ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}.manifest)
        configure_file(cmake/${TARGET}.manifest.in ${manifestPath} @ONLY)
        if(MSVC)
            target_sources(${TARGET} PRIVATE ${manifestPath})
        else()
            configure_file(cmake/${TARGET}.manifest.rc.in ${TARGET}.manifest.rc @ONLY)
            target_sources(${TARGET} PRIVATE ${TARGET}.manifest.rc)
        endif()
    endif()
endfunction()
