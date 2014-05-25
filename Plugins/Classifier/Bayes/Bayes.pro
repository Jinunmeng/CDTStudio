QT       += core gui

TARGET = Bayes
TEMPLATE = lib
CONFIG += plugin
QMAKE_CXXFLAGS += -std=c++0x


DESTDIR = ../../../bin/Plugins


INCLUDEPATH += ../../../Interfaces

SOURCES += bayesinterface.cpp

HEADERS += bayesinterface.h \
    ../../../Interfaces/cdtclassifierinterface.h

OTHER_FILES += Bayes.json

unix{
target.path = /usr/lib
INSTALLS += target
LIBS += -lopencv_core -lopencv_ml
}
!unix{
include(../../../Tools/Config/win.pri)
}
