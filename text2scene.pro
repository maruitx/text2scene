TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS += common
SUBDIRS += scene_lab
SUBDIRS += t2scene

scene_lab.depends = common
t2scene.depends = common scene_lab