add_subdirectory(libacm)
add_subdirectory(md5)
add_subdirectory(stb)

# Create licenses directory
add_custom_target(LicenseDir
  COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Descent3>/licenses"
  COMMENT "Create licenses/ directory"
)

# List of third-party directories
set(third_party_dirs "libacm" "md5" "stb")

# Loop through each directory and copy the matching license file if it exists
foreach(dir ${third_party_dirs})
    set(license_file "${CMAKE_SOURCE_DIR}/third_party/LICENSE_${dir}.txt")
    if(EXISTS ${license_file})
        add_custom_command(
            TARGET LicenseDir
            COMMAND ${CMAKE_COMMAND} -E copy "${license_file}" "$<TARGET_FILE_DIR:Descent3>/licenses/LICENSE_${dir}.txt"
            COMMENT "Copying LICENSE_${dir}.txt to licenses directory"
        )
    endif()
endforeach()