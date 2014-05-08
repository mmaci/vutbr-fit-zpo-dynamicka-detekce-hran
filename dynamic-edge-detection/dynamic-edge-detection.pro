#-------------------------------------------------
#
# Project created by QtCreator 2014-05-02T12:24:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dynamic-edge-detection
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        mainwindow.cpp \
dynamicedgedetector2.cpp \
    dynamicedgedetector.cpp


HEADERS  += mainwindow.h \
    edgedetector.h \
dynamicedgedetector2.h \
    dynamicedgedetector.h \
    constants.h


FORMS    += mainwindow.ui
