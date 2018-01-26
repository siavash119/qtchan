QT      += core gui network
DEFINES *= QT_USE_QSTRINGBUILDER

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qtchan
TEMPLATE = app

QMAKE_CXXFLAGS_RELEASE += -O3

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
    threadtabhelper.cpp \
    treemodel.cpp \
    treeitem.cpp \
    settings.cpp \
    boardtabhelper.cpp \
    captcha.cpp \
    threadinfo.cpp \
    chans.cpp \
    you.cpp

HEADERS += mainwindow.h \
    threadform.h \
    clickablelabel.h \
    boardtab.h \
    netcontroller.h \
    threadtab.h \
    post.h \
    postform.h \
    filter.h \
    overlay.h \
    threadtabhelper.h \
    treeitem.h \
    treemodel.h \
    settings.h \
    boardtabhelper.h \
    captcha.h \
    threadinfo.h \
    chans/chan.h \
    chans.h \
    chans/fourchan.h \
    chans/eightchan.h \
    you.h

FORMS   += mainwindow.ui \
    threadform.ui \
    boardtab.ui \
    threadtab.ui \
    postform.ui \
    settings.ui \
    threadinfo.ui

RC_ICONS = icon.ico
