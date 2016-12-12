include($$[STARLAB])
include( ../common.pri )

QT*=xml opengl widgets
win32:LIBS += -lopengl32 -lglu32

# LOADS EIGEN
INCLUDEPATH *= $$[EIGENPATH]
DEFINES *= EIGEN

TEMPLATE = lib
CONFIG += staticlib

# Build flag
CONFIG(debug, debug|release) {CFG = debug} else {CFG = release}

# Library name and destination
TARGET = scene_lab
DESTDIR = $$PWD/lib/$$CFG

FORMS += \
	scene_lab_widget.ui \
	modeldbviewer_widget.ui
	
HEADERS += \
	scene_lab.h \
	scene_lab_widget.h \
	modeldbviewer_widget.h \
	ModelDBViewer.h \
	modelDatabase.h \ 
	category.h
	
SOURCES += \
	scene_lab.cpp \
	scene_lab_widget.cpp \
	modeldbviewer_widget.cpp \
	ModelDBViewer.cpp \
	modelDatabase.cpp \
	category.cpp