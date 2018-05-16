QT      += core gui network widgets
DEFINES *= QT_USE_QSTRINGBUILDER

TARGET = qtchan
TEMPLATE = app

QMAKE_CXXFLAGS_RELEASE += -O3

SOURCES += main.cpp \
	mainwindow.cpp \
	threadformcontext.cpp \
	threadformstrings.cpp \
	threadform.cpp \
	clickablelabel.cpp \
	filter.cpp \
	netcontroller.cpp \
	chans.cpp \
	overlay.cpp \
	captcha.cpp \
	post.cpp \
	postform.cpp \
	threadtab.cpp \
	threadtabhelper.cpp \
	boardtab.cpp \
	boardtabhelper.cpp \
	treemodel.cpp \
	treeitem.cpp \
	treeview.cpp \
	settings.cpp \
	threadinfo.cpp \
	you.cpp \
	notificationtray.cpp \
	notificationview.cpp \
	archivetab.cpp

HEADERS += mainwindow.h \
	threadformcontext.h \
	threadformstrings.h \
	threadform.h \
	clickablelabel.h \
	filter.h \
	netcontroller.h \
	chans.h \
	chans/chan.h \
	chans/fourchan.h \
	chans/eightchan.h \
	chans/twochhk.h \
	captcha.h \
	overlay.h \
	post.h \
	postform.h \
	threadtab.h \
	threadtabhelper.h \
	boardtab.h \
	boardtabhelper.h \
	treemodel.h \
	treeitem.h \
	treeview.h \
	settings.h \
	threadinfo.h \
	you.h \
	notificationtray.h \
	notificationview.h \
	archivetab.h

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
