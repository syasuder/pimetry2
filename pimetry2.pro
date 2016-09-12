#-------------------------------------------------
#
# Project created by QtCreator 2016-08-31T21:59:55
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = pimetry2
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    tachowidget.cpp

HEADERS  += mainwindow.h \
    tachowidget.h

FORMS    += mainwindow.ui \
    tachowidget.ui

#DEFINES += FRAMELESS

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/qjson/build/src -lqjson
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/qjson/build/src -lqjson

INCLUDEPATH += $$PWD/qjson/include/QJson/
