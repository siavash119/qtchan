QT      += core gui network widgets
DEFINES *= QT_USE_QSTRINGBUILDER

TARGET = qtchan
TEMPLATE = app

QMAKE_CXXFLAGS_RELEASE += -O3

SOURCES += main.cpp \
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
	you.cpp \
	notificationtray.cpp \
	treeview.cpp \
	notificationview.cpp \
	archivetab.cpp \
	threadformcontext.cpp \
	threadformstrings.cpp

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
	you.h \
	notificationtray.h \
	treeview.h \
	notificationview.h \
	archivetab.h \
	threadformcontext.h \
	threadformstrings.h

FORMS   += mainwindow.ui \
	threadform.ui \
	boardtab.ui \
	threadtab.ui \
	postform.ui \
	settings.ui \
	threadinfo.ui \
	notificationview.ui \
	archivetab.ui

RC_ICONS = icons/icon.ico

RESOURCES += \
	files.qrc
