# This abomination is the only way I know how to install quietly at build time.
# Arguments: <source> <destination> [QUIET]
if("${CMAKE_ARGV5}" STREQUAL "QUIET")
    execute_process(COMMAND ${CMAKE_COMMAND} -P "${CMAKE_SCRIPT_MODE_FILE}"
        "${CMAKE_ARGV3}"
        "${CMAKE_ARGV4}"
        OUTPUT_VARIABLE output
        RESULT_VARIABLE result
        )
    if(result AND NOT result EQUAL 0)
        message(SEND_ERROR "Failed to install \"${CMAKE_ARGV3}\" to \"${CMAKE_ARGV4}\"")
    endif()
else()
    file(INSTALL "${CMAKE_ARGV3}" DESTINATION "${CMAKE_ARGV4}" USE_SOURCE_PERMISSIONS)
endif()
