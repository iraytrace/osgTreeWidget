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

    if(WIN32)
      if (EXISTS "${CMAKE_BINARY_DIR}/${RUNTIME_DIR}/${EXEC_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
        set(APP "${CMAKE_BINARY_DIR}/${RUNTIME_DIR}/${EXEC_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
        set(GLOB_DIR "${CMAKE_BINARY_DIR}/${RUNTIME_DIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}*")
      else()
        if (EXISTS "${CMAKE_BINARY_DIR}/${RUNTIME_DIR}/${CMAKE_BUILD_TYPE}/${EXEC_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
          set(APP "${CMAKE_BINARY_DIR}/${RUNTIME_DIR}/${CMAKE_BUILD_TYPE}/${EXEC_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
          set(GLOB_DIR "${CMAKE_BINARY_DIR}/${RUNTIME_DIR}/${CMAKE_BUILD_TYPE}/*${CMAKE_SHARED_LIBRARY_SUFFIX}*")
        else()
          message(FATAL_ERROR "Could not find app ${EXEC_NAME}")
        endif()
      endif()
    else(WIN32)
      set(APP "${CMAKE_BINARY_DIR}/${RUNTIME_DIR}/${EXEC_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
      set(GLOB_DIR "${CMAKE_BINARY_DIR}/${RUNTIME_DIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}*")
    endif(WIN32)

    add_custom_command(TARGET
                       ${EXEC_NAME}
                       POST_BUILD
                       COMMAND "${CMAKE_COMMAND}"
                       -D APP="${APP}"
                       -D CMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}"
                       -D BIN_DIR="${CMAKE_BINARY_DIR}"
                       -D CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
                       -D Qt5Core_DIR="${Qt5Core_DIR}"
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

