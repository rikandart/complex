QT += core gui
QT += network
QT += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

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
    chartview.cpp \
    cuza.cpp \
    dataprocessor.cpp \
    deprecated-code.cpp \
    error.cpp \
    graphview.cpp \
    iniprocessor.cpp \
    jsonprocessor.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    CppFFTW/include/fftw3.h \
    chartview.h \
    cuza.h \
    dataprocessor.h \
    error.h \
    graphview.h \
    iniprocessor.h \
    jsonprocessor.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    complex_ru_RU.ts

INCLUDEPATH += \
    ./FFTW/ \

LIBS += -L"C:/git/repositories/complex/FFTW/" -llibfftw3-3
#QMAKE_CFLAGS += C:/repos/complex/FFTW/lib/libfftw3-3.def

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
