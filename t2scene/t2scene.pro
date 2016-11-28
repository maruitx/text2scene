include($$[STARLAB])
include( ../common.pri )
include( ../scene_lab.pri )

StarlabTemplate(plugin)


FORMS += \
	text_scene_widget.ui 

HEADERS += \
	text2scene_mode.h \
	text_scene_widget.h \
	ScenePatch.h \
	SynScene.h

	
SOURCES += \
	text2scene_mode.cpp \
	text_scene_widget.cpp \
	ScenePatch.cpp \
	SynScene.cpp
	
RESOURCES += \
	text2scene.qrc
	

