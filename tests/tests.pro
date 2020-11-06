QT += testlib
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

DEFINES += QT_DEPRECATED_WARNINGS
# DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_DEBUG

TEMPLATE = app

INCLUDEPATH += $$PWD/../src

SOURCES +=  tst_testqtrencode.cpp \
    ../src/qtrencode.cpp

HEADERS += \
    ../src/qtrencode.h
