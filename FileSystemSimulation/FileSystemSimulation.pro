#-------------------------------------------------
#
# Project created by QtCreator 2018-07-05T10:43:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FileSystemSimulation
TEMPLATE = app
RC_ICONS = yt.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    functions.cpp \
    md5.cpp \
    content.cpp \
    login.cpp \
    zip.cpp \
    block.cpp \
    inode.cpp \
    fs.cpp \
    path.cpp \
    dir.cpp \
    file.cpp

HEADERS += \
        mainwindow.h \
    md5.h \
    functions.h \
    content.h \
    login.h \
    const.h \
    user.h \
    fs.h \
    dir.h \
    block.h \
    file.h \
    inode.h \
    path.h \
    superblk.h \
    zip.h

FORMS += \
    mainwindow.ui \
    content.ui

DISTFILES += \
    ../../../yt.png
