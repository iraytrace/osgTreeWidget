# Dependency options
if(WIN32)
  set(INSTALL_DEPS_DEFAULT ON)
else(WIN32)
  set(INSTALL_DEPS_DEFAULT OFF)
endif(WIN32)

if(APPLE)
  set(Install_Dependencies OFF)
else(APPLE)
  option(Install_Dependencies
         "Install linked libraries with application"
         ${INSTALL_DEPS_DEFAULT})
endif(APPLE)

macro(find_and_install_dependencies EXEC_NAME)
  if(Install_Dependencies)

    set (APP "$<TARGET_FILE:${EXEC_NAME}>")
    set (GLOB_DIR "$<TARGET_FILE_DIR:${EXEC_NAME}>/*${CMAKE_SHARED_LIBRARY_SUFFIX}")

    add_custom_command(TARGET
                       ${EXEC_NAME}
                       POST_BUILD
                       COMMAND "${CMAKE_COMMAND}"
                       -D APP="${APP}"
                       -D CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
                       -D BIN_DIR="${CMAKE_BINARY_DIR}"
                       -D CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
                       -D GDAL_LIBDIR="${GDAL_LIBDIR}"
                       -D Qt5_DIR="${Qt5_DIR}"
                       -D OSG_DIR="${OSG_DIR}"
                       -P "${CMAKE_SOURCE_DIR}/misc/CMake/runBundleUtil.cmake"
                       )

    # Find files found by BundleUtilities and install them

    file(GLOB BUNDLED_LIBS "${GLOB_DIR}")
    foreach(FILE ${BUNDLED_LIBS})
      message(STATUS "adding ${FILE} to build directory")
      INSTALL(FILES "${FILE}" DESTINATION "${RUNTIME_DIR}")
    endforeach()
  endif(Install_Dependencies)
endmacro(find_and_install_dependencies)

