message(STATUS ${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB osgform_SRCS
  "${CMAKE_CURRENT_LIST_DIR}/*cpp"
  "${CMAKE_CURRENT_LIST_DIR}/*h"
  "${CMAKE_CURRENT_LIST_DIR}/*qrc")

file(GLOB osgform_UIS "${CMAKE_CURRENT_LIST_DIR}/*ui") # Make all *.ui files part of the build

include_directories(${PROJECT_BINARY_DIR}) # necessary to pick up uic headers
include_directories(${CMAKE_BINARY_DIR}) # necessary to pick up uic headers
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
qt5_wrap_ui(UIS ${osgform_UIS})


add_library(osgform STATIC ${osgform_SRCS} ${UIS})

target_link_libraries(osgform
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::OpenGL
   ${OPENSCENEGRAPH_LIBRARIES}
)
