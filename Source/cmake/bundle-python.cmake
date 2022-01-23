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
        execute_process(COMMAND codesign --remove-signature "${binary}")
        execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change "${pySO}" "${newPySO}" "${binary}")
    endfunction()
    file(CREATE_LINK "${pyMajDotMin}" "${pyBulkInstallTo}/Current" SYMBOLIC)  # Make codesign happy
    file(CREATE_LINK "../Frameworks/${pyFWName}/Versions/${pyMajDotMin}" "${bundleContents}/Resources/python" SYMBOLIC)
    file(CREATE_LINK "python${pyMajDotMin}" "${pyDirDst}/bin/python" SYMBOLIC)
    get_filename_component(pySO "${Python_LIBRARIES}" REALPATH)
    get_filename_component(pySOName "${pySO}" NAME)
    string(REGEX REPLACE ".*/${pyFWName}" "${pyFWName}" pyFWRelative "${pySO}")
    set(pyExecutable "${pyDirDst}/bin/python${pyMajDotMin}")
    # python.org Python links and is linked without @rpath, the next 3 lines fix this.
    execute_process(COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change "${pySO}" "@loader_path/../Frameworks/${pyFWRelative}" "${targetBinary}")
    relinkPySigned("${pyExecutable}" "@executable_path/../${pySOName}")
    relinkPySigned("${pyDirDst}/Resources/Python.app/Contents/MacOS/Python" "@executable_path/../../../../${pySOName}")
    execute_process(COMMAND chmod -R u+w "${bundleContents}/Frameworks/${pyFWName}")
    execute_process(COMMAND codesign --remove-signature --deep "${bundleContents}/Frameworks/${pyFWName}")
    execute_process(COMMAND codesign -s - --deep "${bundleContents}/Frameworks/${pyFWName}")
    execute_process(COMMAND "${pyExecutable}" -m ensurepip)
    execute_process(COMMAND "${pyExecutable}" -m pip install ${BESPOKE_PIP_PACKAGES})
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


