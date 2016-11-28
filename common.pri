INCLUDEPATH *= $$PWD/common
Debug: LIBS += -L$$PWD/common/lib/debug/ -lcommon
Release: LIBS += -L$$PWD/common/lib/release/ -lcommon

# Opcode lib
INCLUDEPATH *= $$PWD/common/third_party/opcode/include
Debug: LIBS += -L$$PWD/common/third_party/opcode/lib/win32 -lOpcode_D
Release: LIBS += -L$$PWD/common/third_party/opcode/lib/win32 -lOpcode

win32 {# Prevent rebuild and Enable debuging in release mode
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG
}
	