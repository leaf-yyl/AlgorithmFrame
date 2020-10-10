QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    $$files(Utils/*.cpp) \
    $$files(VideoIo/*.cpp)

HEADERS += \
    mainwindow.h \
    $$files(Utils/*.h) \
    $$files(VideoIo/*.h)

FORMS += \
    mainwindow.ui

FFMPEG_PATH=/opt/ffmpeg3.4.5
INCLUDEPATH += \
#FFMPEG
    $$FFMPEG_PATH/include

LIBS += \
#ffmpeg
    -L$$FFMPEG_PATH/lib \
    -Wl,-rpath,$$FFMPEG_PATH/lib \
    -lavformat -lavcodec -lavutil -lswscale -lswresample \
#libs installed on system
    -L/usr/lib/x86_64-linux-gnu \
    -Wl,-rpath,/usr/lib/x86_64-linux-gnu \
    -lglog

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
