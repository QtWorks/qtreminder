#-------------------------------------------------
#
# Project created by QtCreator 2014-07-28T06:33:08
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ToDoReminder
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp\
    sidewidget.cpp \
    tasknotifier.cpp \
    taskview.cpp \
    taskdialog.cpp

HEADERS  += \
    sidewidget.h \
    tasknotifier.h \
    taskview.h \
    taskdialog.h

FORMS    += \
    sidewidget.ui \
    tasknotifier.ui \
    taskdialog.ui \
    taskview.ui

RESOURCES += \
    Resources.qrc

VERSION = 1.0.0.0

RC_ICONS = Icon2.ico
