TEMPLATE = lib
CONFIG += staticlib
TARGET = QPropertyEditor

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES =   ColorCombo.cpp \
            Property.cpp \
            QPropertyEditorWidget.cpp \
            QPropertyModel.cpp \
            QVariantDelegate.cpp \
            EnumProperty.cpp

HEADERS=    ColorCombo.h \
	    	Property.h \
            QPropertyEditorWidget.h \
            QPropertyModel.h \
            QVariantDelegate.h \
            EnumProperty.h

DESTDIR = ../../lib