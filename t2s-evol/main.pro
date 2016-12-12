DEFINES += _USE_MATH_DEFINES

CONFIG += console

QT += widgets core opengl

!win32 {
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS_WARN_ON += -Wall
QMAKE_CXXFLAGS_WARN_ON += -Wall
}

win32 {
QMAKE_CXXFLAGS_RELEASE += /Zi
QMAKE_LFLAGS_RELEASE += /DEBUG
QMAKE_CXXFLAGS_WARN_ON += /W0
QMAKE_CXXFLAGS_WARN_ON += /W0

QMAKE_CXXFLAGS += -MP8
QMAKE_LFLAGS += -LARGEADDRESSAWARE

QMAKE_CFLAGS_RELEASE += /MD
QMAKE_CXXFLAGS_RELEASE += /MD
QMAKE_CFLAGS_DEBUG += /MDd
QMAKE_CXXFLAGS_DEBUG += /MDd
}

INCLUDEPATH += \
./ThirdParty/glew-1.12.0/include \
./ThirdParty/freeglut-MSVC-3.0.0-2.mp/freeglut/include


CONFIG(release, debug | release){
LIBS += \
./ThirdParty/glew-1.12.0/lib/Release/x64/glew32.lib \
./ThirdParty/freeglut-MSVC-3.0.0-2.mp/freeglut/lib/x64/freeglut.lib
}
else {
LIBS += \
./ThirdParty/glew-1.12.0/lib/Release/x64/glew32.lib \
./ThirdParty/freeglut-MSVC-3.0.0-2.mp/freeglut/lib/x64/freeglut.lib
}

TARGET = Main

RESOURCES = main.qrc

HEADERS +=  \
Src/*.h \

SOURCES +=  \
Main.cpp \
Src/*.cpp 

