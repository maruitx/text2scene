INCLUDEPATH *= $$PWD/scene_lab
Debug: LIBS += -L$$PWD/scene_lab/lib/debug/ -lscene_lab
Release: LIBS += -L$$PWD/scene_lab/lib/release/ -lscene_lab
