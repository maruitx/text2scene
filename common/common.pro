include($$[STARLAB])

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
TARGET = common
DESTDIR = $$PWD/lib/$$CFG

DEPENDPATH += geometry 
DEPENDPATH += utilities 
				
INCLUDEPATH += geometry
INCLUDEPATH += utilities

HEADERS += \
	geometry/Scene.h \
	geometry/CModel.h \
	geometry/CMesh.h \
	geometry/AABB.h \
	geometry/OBB.h \
	geometry/OBBEstimator.h \
	geometry/BestFit.h \
	geometry/TriTriIntersect.h \
	geometry/SceneGraph.h \
	geometry/UDGraph.h \
	utilities/utility.h \
	utilities/mathlib.h \
	utilities/Eigen3x3.h 

	
SOURCES += \
	geometry/Scene.cpp \
	geometry/CModel.cpp \
	geometry/CMesh.cpp \
	geometry/AABB.cpp \
	geometry/OBB.cpp \
	geometry/OBBEstimator.cpp \
	geometry/BestFit.cpp \
	geometry/TriTriIntersect.cpp \
	geometry/SceneGraph.cpp \
	geometry/UDGraph.cpp \
	utilities/mathlib.cpp 
	
# Opcode lib
win32{
INCLUDEPATH *= $$PWD/third_party/opcode/include
Debug: LIBS += -L$$PWD/third_party/opcode/lib/win32 -lOpcode_D
Release: LIBS += -L$$PWD/third_party/opcode/lib/win32 -lOpcode

# Copy dll for Opcode
CONFIG(debug, debug|release){
	EXTRA_BINFILES += $$PWD/third_party/opcode/lib/win32/Opcode_D.dll
} else {
	EXTRA_BINFILES += $$PWD/third_party/opcode/lib/win32/Opcode.dll
}		
        EXTRA_BINFILES_WIN = $${EXTRA_BINFILES}
        EXTRA_BINFILES_WIN ~= s,/,\\,g
        DESTDIR_WIN = $${EXECUTABLEPATH}
        DESTDIR_WIN ~= s,/,\\,g
        for(FILE,EXTRA_BINFILES_WIN){
            # message("Will copy file" $$FILE "to" $$DESTDIR_WIN)
            QMAKE_POST_LINK += $$quote(cmd /c copy /y $${FILE} $${DESTDIR_WIN}$$escape_expand(\n\t))
        }
}