message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})

# make all sources part of the build
file(GLOB widgetapp_SRCS
  "${CMAKE_CURRENT_LIST_DIR}/*cpp"
  "${CMAKE_CURRENT_LIST_DIR}/*h"
  "${CMAKE_SOURCE_DIR}/common/*cpp"
  "${CMAKE_SOURCE_DIR}/common/*h"
  "${CMAKE_CURRENT_LIST_DIR}/*qrc")

message(STATUS "inc >${CMAKE_SOURCE_DIR}/common<")
include_directories("${CMAKE_SOURCE_DIR}/common")
include_directories("${CMAKE_SOURCE_DIR}/OsgForm")

file(GLOB widgetapp_UIS "${CMAKE_CURRENT_LIST_DIR}/*ui") # Make all *.ui files part of the build

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
qt5_wrap_ui(UIS ${widgetapp_UIS})

if (WIN32)
    add_executable(widgetapp WIN32 ${widgetapp_SRCS} ${UIS} )
else (WIN32)
    add_executable(widgetapp       ${widgetapp_SRCS} ${UIS} )
endif (WIN32)

target_link_LIBRARIES(widgetapp
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
   ${OPENSCENEGRAPH_LIBRARIES}
${GDAL_LIBRARY}
osgform
  )
set_target_properties(widgetapp
    PROPERTIES
    ENABLE_EXPORTS ON)
# if cmake version is before 2.8.12 you need to add
# COMPILE_FLAGS "${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}"


install(TARGETS widgetapp
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime )

