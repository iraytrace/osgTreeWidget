######################################################################
# rpath start
######################################################################
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

# set the RPATH 
# SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib64")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# look for "${CMAKE_INSTALL_PREFIX}/lib" in the list of implicit (system)
# link directories.  If it isn't in the list, then set the CMAKE_INSTALL_RPATH
# to what we want the rpath to be.
# the RPATH to be used when installing, but only if it's not a system directory

MESSAGE(STATUS "---rpath--- ${CMAKE_INSTALL_RPATH} -----")

LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:${CMAKE_INSTALL_PREFIX}/lib64:/usr/brlcad/stable/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")
######################################################################
# rpath end
######################################################################

