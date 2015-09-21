###############################################################################
# Compiler configuration
###############################################################################
# This approach to managing compiler settings was adopted from interactions
# between Jeff Amstutz and Intel Corporation.

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  include(gcc)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  include(icc)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  include(clang)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  include(msvc)
else()
  message(FATAL_ERROR "Unknown compiler specified: < ${CMAKE_CXX_COMPILER_ID} >")
endif()

