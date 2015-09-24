project( osgTreeWidget )
cmake_minimum_required(VERSION 3.2)

# Allow all sub CMakeLists to use our custom distributed CMake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_SOURCE_DIR}/misc/CMake"
  "${CMAKE_SOURCE_DIR}/misc/CMake/compiler"
)

include(findAndInstallDependencies)
include(setBuildMachineUser) #create definitions for whence the executable was built
include(compiler_setup)
include(InstallRequiredSystemLibraries)

include_directories(${CMAKE_SOURCE_DIR}/include)

# target build locations
set(RUNTIME_DIR bin)
set(PLUGIN_DIR ${RUNTIME_DIR}/plugins)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/lib)

#set(OSG_DIR "C:/Users/Lee/Documents/src/osg/install340")
if (WIN32)
set(OSG_DIR "$ENV{USERPROFILE}/sw") 
endif (WIN32)
find_package(OpenSceneGraph 3.0.1 COMPONENTS osg osgDB osgViewer osgGA osgUtil OpenThreads )
include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})

set(ENV{GDAL_DIR} "$ENV{USERPROFILE}/sw")
find_package(GDAL REQUIRED)
INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR})

# Get the Qt stuff set up.
# if cmake cannot find Qt you can set Qt5_DIR or uncomment next line
if (WIN32)
set(Qt5_DIR "C:/Qt/5.5/msvc2013_64/lib/cmake/Qt5" CACHE PATH "Qt cmake dir")
endif (WIN32)
find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)

add_subdirectory(OsgForm)
add_subdirectory(mdiapp)
add_subdirectory(widgetapp)


## The following is from http://www.vtk.org/Wiki/CMake/CPack/BundleUtilities/Examples/Linux/Automatic
#SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/osgTreeWidget${CMAKE_EXECUTABLE_SUFFIX}")
#set(YOUR_PLUGINS "") #empty for now
##get_filename_component(GDAL_LIBDIR ${GDAL_LIBRARY} DIRECTORY)
##message(STATUS "gdal_libdir>${GDAL_LIBDIR}/../bin<")
#set(YOUR_LIBRARY_DIRS
#  "${CMAKE_INSTALL_PREFIX}/bin"
#  "${CMAKE_INSTALL_PREFIX}/lib"
#  "${CMAKE_BINARY_DIR}/bin"
#  "${CMAKE_BINARY_DIR}/lib"
#    "${OSG_DIR}/bin"
#    "${OSG_DIR}/lib"
##    "${GDAL_LIBDIR}/../bin"
#  "${Qt5_DIR}/../../"
#  "${Qt5_DIR}/../../bin"
#  "${Qt5_DIR}/../../../bin"
#  "${Qt5_DIR}/../../../lib")

#file(GLOB OSG_PLUGINS_DIR ${OSG_DIR}/*/osgPlugins-${OPENSCENEGRAPH_VERSION})
#message(STATUS ${OSG_PLUGINS_DIR})

#file(GLOB OSG_PLUGINS "${OSG_PLUGINS_DIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}")
#foreach (OSG_PLUGIN ${OSG_PLUGINS})
#file(COPY "${OSG_PLUGIN}"
#  DESTINATION bin/osgPlugins-${OPENSCENEGRAPH_VERSION})
#endforeach()

#find_and_install_dependencies(osgTreeWidget)

#install(CODE "
#  include(BundleUtilities)
#  fixup_bundle(\"${APPS}\" \"\${YOUR_PLUGINS}\" \"${YOUR_LIBRARY_DIRS}\")
#  " COMPONENT Runtime)

# include(setupCpack)

