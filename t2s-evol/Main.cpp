#include <QApplication>
#include "Src/Headers.h"
#include "Src/GLWidget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    glutInit(&argc, argv);

    QGLFormat format;
    format.setSampleBuffers(true);
    format.setDoubleBuffer(true);
    format.setRgba(true);
    format.setDirectRendering(true);
    format.setSamples(16); 

    QGLContext *context = new QGLContext(format);

    GLWidget widget(context, 1280, 720);    
    widget.setWindowTitle("");
    widget.setGeometry(100, 100, 1640, 720);
    widget.show();    
   
    return app.exec();
}
