#-------------------------------------------------
#
# Project created by QtCreator 2013-03-27T08:32:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VS-Projekt
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    Generator.cpp \
    Simulator.cpp \
    Event.cpp

HEADERS  += MainWindow.h \
    Generator.h \
    Simulator.h \
    Event.h

FORMS    += MainWindow.ui
