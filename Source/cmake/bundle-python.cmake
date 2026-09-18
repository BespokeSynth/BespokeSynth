# Much of this exclusion list was lifted from Blender (GPL-2.0-or-later). Thanks!
file(INSTALL
    "${pyBulkInstallFrom}"
    DESTINATION "${pyBulkInstallTo}"
    USE_SOURCE_PERMISSIONS
    PATTERN "*.a" EXCLUDE
    PATTERN "*.exe" EXCLUDE
    PATTERN "*.orig" EXCLUDE
    PATTERN "*.pyc" EXCLUDE
    PATTERN "*.pyo" EXCLUDE
    PATTERN "*.rej" EXCLUDE
    PATTERN ".DS_Store" EXCLUDE
    PATTERN ".git" EXCLUDE
    PATTERN ".svn" EXCLUDE
    PATTERN "__MACOSX" EXCLUDE
    PATTERN "__pycache__" EXCLUDE
    PATTERN "idlelib" EXCLUDE
    PATTERN "lib-dynload/_tkinter.*" EXCLUDE
    PATTERN "lib2to3" EXCLUDE
    PATTERN "site-packages" EXCLUDE
    PATTERN "test" EXCLUDE
    PATTERN "tkinter" EXCLUDE
    PATTERN "turtle.py" EXCLUDE
    PATTERN "turtledemo" EXCLUDE
    )

set(BESPOKE_PIP_PACKAGES jedi)

if(APPLE)
    function(relinkPySigned binary newPySO)
        # execute_process(COMMAND codesign --remove-signature "${binary}")
        execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change "${pySO}" "${newPySO}" "${binary}" COMMAND_ECHO STDOUT RESULT_VARIABLE _relinkResult)
        if(NOT _relinkResult EQUAL 0)
            message(FATAL_ERROR "install_name_tool failed to relink '${binary}' (looking for '${pySO}') - the app would ship with a hardcoded absolute Python path and crash on any other Mac. See comment above for why this can silently mismatch.")
        endif()
    endfunction()
    file(CREATE_LINK "${pyMajDotMin}" "${pyBulkInstallTo}/Current" SYMBOLIC)  # Make codesign happy
    file(CREATE_LINK "../Frameworks/${pyFWName}/Versions/${pyMajDotMin}" "${bundleContents}/Resources/python" SYMBOLIC)
    file(CREATE_LINK "python${pyMajDotMin}" "${pyDirDst}/bin/python" SYMBOLIC)
    # NOTE: this used to be get_filename_component(pySO "${Python_LIBRARIES}" REALPATH), which fully
    # resolves Homebrew's "opt/pythonX.Y" convenience symlink down to its real Cellar path. But the
    # linker embeds whatever path Python_LIBRARIES actually was (the symlinked "opt/" path) as the
    # dylib's LC_LOAD_DYLIB - not its realpath - so install_name_tool -change was searching for a
    # string that didn't exist in the binary, silently doing nothing (execute_process doesn't check
    # the exit code), and leaving the original absolute build-machine path in the shipped binary.
    # This is exactly what caused "Library not loaded: /opt/homebrew/.../Python" crashes on any Mac
    # other than the one that built the app, even with BESPOKE_PORTABLE=ON. Read the path actually
    # embedded in the binary instead of reconstructing/guessing it, so this can never mismatch again.
    execute_process(COMMAND otool -L "${targetBinary}" OUTPUT_VARIABLE _targetBinaryDeps)
    string(REGEX MATCH "[^ \t\n]*${pyFWName}[^ \t\n]*" pySO "${_targetBinaryDeps}")
    if(NOT pySO)
        message(FATAL_ERROR "couldn't find a ${pyFWName} dependency in ${targetBinary} via otool -L - can't relink Python for a portable build")
    endif()
    get_filename_component(pySOName "${pySO}" NAME)
    string(REGEX REPLACE ".*/${pyFWName}" "${pyFWName}" pyFWRelative "${pySO}")
    set(pyExecutable "${pyDirDst}/bin/python${pyMajDotMin}")
    # python.org Python links and is linked without @rpath, the next 3 lines fix this.
    execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change "${pySO}" "@loader_path/../Frameworks/${pyFWRelative}" "${targetBinary}" RESULT_VARIABLE _relinkTargetResult)
    if(NOT _relinkTargetResult EQUAL 0)
        message(FATAL_ERROR "install_name_tool failed to relink '${targetBinary}' (looking for '${pySO}') - the app would ship with a hardcoded absolute Python path and crash on any other Mac.")
    endif()
    relinkPySigned("${pyExecutable}" "@executable_path/../${pySOName}")
    relinkPySigned("${pyDirDst}/Resources/Python.app/Contents/MacOS/Python" "@executable_path/../../../../${pySOName}")
    execute_process(COMMAND chmod -R u+w "${bundleContents}/Frameworks/${pyFWName}")
    execute_process(COMMAND codesign --remove-signature --deep "${bundleContents}/Frameworks/${pyFWName}")
    execute_process(COMMAND codesign -s - --deep "${bundleContents}/Frameworks/${pyFWName}")
    execute_process(COMMAND "${pyExecutable}" -m ensurepip COMMAND_ECHO STDOUT)
    execute_process(COMMAND "${pyExecutable}" -m pip install --break-system-packages ${BESPOKE_PIP_PACKAGES} COMMAND_ECHO STDOUT)
elseif(WIN32)
    get_filename_component(pyDirSrc "${Python_EXECUTABLE}" DIRECTORY)
    file(GLOB pyBinaries LIST_DIRECTORIES false "${pyDirSrc}/*.dll" "${pyDirSrc}/*.exe")
    file(INSTALL ${pyBinaries} DESTINATION "${pyDirDst}")
    file(INSTALL "${pyDirSrc}/DLLs" DESTINATION "${pyDirDst}")
    file(INSTALL "${CMAKE_CURRENT_BINARY_DIR}/python.manifest" DESTINATION "${pyDirDst}")
    set(pyExecutable "${pyDirDst}/python.exe")
    execute_process(COMMAND "${pyExecutable}" -m ensurepip)
    execute_process(COMMAND "${pyExecutable}" -m pip install ${BESPOKE_PIP_PACKAGES})
else()
    get_filename_component(pySO "${Python_LIBRARIES}" REALPATH)
    file(INSTALL "${pySO}" DESTINATION "${pyDirDst}/bin")
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy "${Python_EXECUTABLE}" "${pyDirDst}/bin/python")
    execute_process(COMMAND patchelf --set-rpath "$ORIGIN" "${pyDirDst}/bin/python")
    set(pyExecutable "${pyDirDst}/bin/python")
endif()


