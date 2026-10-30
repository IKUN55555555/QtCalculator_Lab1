QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TextEditor
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        codeeditor.cpp \
        linenumberarea.cpp \
        finddialog.cpp \
        replacedialog.cpp \
        aboutdialog.cpp

HEADERS += \
        mainwindow.h \
        codeeditor.h \
        linenumberarea.h \
        finddialog.h \
        replacedialog.h \
        aboutdialog.h

FORMS += \
        mainwindow.ui \
        finddialog.ui \
        replacedialog.ui \
        aboutdialog.ui

RESOURCES += \
        resource.qrc
