#-------------------------------------------------
#
# Project created by QtCreator 2017-01-19T15:02:21
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = VolumeRenderer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    screenshotwidget.cpp

HEADERS  += mainwindow.h \
    screenshotwidget.h

FORMS    += mainwindow.ui \
    screenshotwidget.ui
