QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SudokuDLX
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += c++11

SOURCES += \
    dlx.cpp \
    exactcoverbuilder.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    dlx.h \
    exactcoverbuilder.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
