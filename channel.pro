#-------------------------------------------------
#
# Project created by QtCreator 2016-12-11T10:33:38
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = channel
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    threadform.cpp \
    clickablelabel.cpp \
    boardtab.cpp \
    treekeypress.cpp \
    netcontroller.cpp \
    threadtab.cpp

HEADERS  += mainwindow.h \
    threadform.h \
    clickablelabel.h \
    boardtab.h \
    treekeypress.h \
    netcontroller.h \
    threadtab.h

FORMS    += mainwindow.ui \
    threadform.ui \
    boardtab.ui \
    threadtab.ui

RESOURCES += \
    textfinder.qrc
