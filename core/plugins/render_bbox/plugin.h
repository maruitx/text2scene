#pragma once
#include "RenderPlugin.h"

class plugin : public RenderPlugin{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "render_bbox.plugin.starlab")
    Q_INTERFACES(RenderPlugin)
public: 
    QString name() { return "Bounding Box"; }
    QIcon icon(){ return QIcon(":/icons/bbox.png"); }
    bool isApplicable(Starlab::Model* model){ return !(model->bbox().isNull()); }
    Renderer* instance();    
};
