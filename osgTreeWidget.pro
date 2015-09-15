#-------------------------------------------------
#
# Project created by QtCreator 2015-09-14T10:39:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = osgTreeWidget
TEMPLATE = app
LIBS += -losg -losgDB -losgViewer

SOURCES += main.cpp MainWindow.cpp RecentFiles.cpp VSLapp.cpp \
    OsgTreeWidget.cpp \
    OsgPropertyTable.cpp

HEADERS  += MainWindow.h RecentFiles.h VSLapp.h \
    OsgTreeWidget.h VariantPtr.h \
    OsgPropertyTable.h

FORMS    += MainWindow.ui

