#-------------------------------------------------
#
# Project created by QtCreator 2013-06-05T23:40:35
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VolumeRenderer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    widget.cpp \
    screenshotwidget.cpp

HEADERS  += mainwindow.h \
    widget.h \
    screenshotwidget.h

FORMS    += mainwindow.ui \
    widget.ui \
    screenshotwidget.ui
