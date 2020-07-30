#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "HumanFactors" for configuration "Release"
set_property(TARGET HumanFactors APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(HumanFactors PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/HumanFactors.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/HumanFactors.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS HumanFactors )
list(APPEND _IMPORT_CHECK_FILES_FOR_HumanFactors "${_IMPORT_PREFIX}/lib/HumanFactors.lib" "${_IMPORT_PREFIX}/bin/HumanFactors.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
