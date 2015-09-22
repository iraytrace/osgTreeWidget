if (WIN32)
set(CPACK_GENERATOR ZIP)
endif (WIN32)

set(CPACK_PACKAGE_NAME osgTreeWidget)

if (EXISTS "${CMAKE_SOURCE_DIR}/misc/license.txt")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/misc/license.txt")
endif ()

set (CPACK_PACKAGE_VERSION_MAJOR 1)
set (CPACK_PACKAGE_VERSION_MINOR 0)
set (CPACK_PACKAGE_VERSION_PATCH 0)
include (CPack)
