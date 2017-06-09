#-------------------------------------------------
#
# Project created by QtCreator 2016-12-11T10:33:38
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtchan
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    threadform.cpp \
    clickablelabel.cpp \
    boardtab.cpp \
    netcontroller.cpp \
    threadtab.cpp \
    post.cpp \
    postform.cpp \
    filter.cpp \
    overlay.cpp \
    threadtabhelper.cpp

HEADERS  += mainwindow.h \
    threadform.h \
    clickablelabel.h \
    boardtab.h \
    netcontroller.h \
    threadtab.h \
    post.h \
    postform.h \
    filter.h \
    overlay.h \
    threadtabhelper.h

FORMS    += mainwindow.ui \
    threadform.ui \
    boardtab.ui \
    threadtab.ui \
    postform.ui

RESOURCES +=
