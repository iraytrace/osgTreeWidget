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

# target build locations
set(RUNTIME_DIR bin)
set(PLUGIN_DIR ${RUNTIME_DIR}/plugins)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH    ${CMAKE_BINARY_DIR}/lib)

#set(OSG_DIR "C:/Users/Lee/Documents/src/osg/install340")
set(OSG_DIR "C:/Users/Lee/Desktop/StingrayLeeBuild/Installed")
find_package(OpenSceneGraph 3.0.1 COMPONENTS osg osgDB osgViewer osgGA osgUtil OpenThreads REQUIRED)
include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})

set(GDAL_DIR "C:/Users/Lee/Desktop/StingrayLeeBuild/Installed")
set(GDAL_INCLUDE_DIR "C:/Users/Lee/Desktop/StingrayLeeBuild/Installed/include")
set(GDAL_LIBRARY "C:/Users/Lee/Desktop/StingrayLeeBuild/Installed/lib/gdal_i.lib")
find_package(GDAL REQUIRED)
INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR})




# Get the Qt stuff set up.
# if cmake cannot find Qt you can set Qt5_DIR or uncomment next line
set(Qt5_DIR "C:/Qt/5.5/msvc2013_64/lib/cmake/Qt5" CACHE PATH "Qt cmake dir")
find_package(Qt5 COMPONENTS Core Gui Widgets OpenGL REQUIRED)

# make all sources part of the build
file(GLOB product_SRCS
  "${CMAKE_CURRENT_LIST_DIR}/*cpp"
  "${CMAKE_CURRENT_LIST_DIR}/*h"
  "${CMAKE_CURRENT_LIST_DIR}/*qrc")

file(GLOB product_UIS "${CMAKE_CURRENT_LIST_DIR}/*ui") # Make all *.ui files part of the build

include_directories(${PROJECT_BINARY_DIR}) # necessary to pick up uic headers
include_directories(
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5Gui_INCLUDE_DIRS}
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5OpenGL_INCLUDE_DIRS}
    )

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)
# this makes sure UI files are shown in the IDE
qt5_wrap_ui(UIS ${product_UIS})

if (WIN32)
    add_executable(osgTreeWidget WIN32 ${product_SRCS} ${UIS} )
else (WIN32)
    add_executable(osgTreeWidget       ${product_SRCS} ${UIS} )
endif (WIN32)

target_link_LIBRARIES(osgTreeWidget
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
   ${OPENSCENEGRAPH_LIBRARIES}
${GDAL_LIBRARY}
  )
set_target_properties(osgTreeWidget
    PROPERTIES
    ENABLE_EXPORTS ON)
# if cmake version is before 2.8.12 you need to add 
# COMPILE_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}"

	
install(TARGETS osgTreeWidget
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime )


# The following is from http://www.vtk.org/Wiki/CMake/CPack/BundleUtilities/Examples/Linux/Automatic
SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/osgTreeWidget${CMAKE_EXECUTABLE_SUFFIX}")
set(YOUR_PLUGINS "") #empty for now
get_filename_component(GDAL_LIBDIR ${GDAL_LIBRARY} DIRECTORY)
message(STATUS "gdal_libdir>${GDAL_LIBDIR}/../bin<")
set(YOUR_LIBRARY_DIRS
  "${CMAKE_INSTALL_PREFIX}/bin"
  "${CMAKE_INSTALL_PREFIX}/lib"
  "${CMAKE_BINARY_DIR}/bin"
  "${CMAKE_BINARY_DIR}/lib"
    "${OSG_DIR}/bin"
    "${OSG_DIR}/lib"
    "${GDAL_LIBDIR}/../bin"
  "${Qt5_DIR}/../../"
  "${Qt5_DIR}/../../bin"
  "${Qt5_DIR}/../../../bin"
  "${Qt5_DIR}/../../../lib")

file(GLOB OSG_PLUGINS_DIR ${OSG_DIR}/*/osgPlugins-${OPENSCENEGRAPH_VERSION})
message(STATUS ${OSG_PLUGINS_DIR})

file(GLOB OSG_PLUGINS "${OSG_PLUGINS_DIR}/*${CMAKE_SHARED_LIBRARY_SUFFIX}")
foreach (OSG_PLUGIN ${OSG_PLUGINS})
file(COPY "${OSG_PLUGIN}"
  DESTINATION bin/osgPlugins-${OPENSCENEGRAPH_VERSION})
endforeach()

find_and_install_dependencies(osgTreeWidget)

install(CODE "
  include(BundleUtilities)
  fixup_bundle(\"${APPS}\" \"\${YOUR_PLUGINS}\" \"${YOUR_LIBRARY_DIRS}\")
  " COMPONENT Runtime)

 include(setupCpack)
