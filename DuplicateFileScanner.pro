#-------------------------------------------------
#
# Project created by QtCreator 2018-12-07T00:06:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DuplicateFileScanner
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        src/main.cpp \
        src/interface/mainwindow.cpp \
        src/interface/Deleter.cpp \
        src/interface/TreeWidgetItem.cpp \
        src/library/DirectoryScanner.cpp \
        src/library/FileComparator.cpp \
        src/library/retry.cpp \
        src/library/Worker.cpp

HEADERS += \
        src/interface/mainwindow.h \
        src/interface/Deleter.h \
        src/interface/TreeWidgetItem.h \
        src/library/DirectoryScanner.h \
        src/library/FileComparator.h \
        src/library/retry.h \
        src/library/util.h \
        src/library/Worker.h

FORMS += \
        src/interface/mainwindow.ui

CONFIG (debug, debug|release) {
    CONFIG += \
        console
}
