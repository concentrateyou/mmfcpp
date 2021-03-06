#ifndef OFFLINE_WIDGET_H
#define OFFLINE_WIDGET_H

#include <GL/glew.h>

#include <QFile>
#include <QTimer>
#include <QGLWidget>
#include <QGLShader>
#include <QGLShaderProgram>

#include <QBool>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

#include <gl_thread.h>

class Offline_widget: public QGLWidget
{
    Q_OBJECT
public:
    friend class Gl_thread;

    Offline_widget(int width, int height, const QString& path, QGLWidget* shared, QWidget *parent = 0);
    ~Offline_widget();

public:
    GLuint renderBuffer();
    bool isRenderBufferEmpty();
    void addToFreeQueue(GLuint id);

protected:
    void run();
    void paintGL();
    void initializeGL();
    void resizeGL(int width, int height);

private:
    QFile file;
    QMutex mutex;
    QMutex wc_mutex;
    QWaitCondition wc;
    QQueue<GLuint> free;
    QQueue<GLuint> render;

    int video_width;
    int video_height;

    GLuint buffer[10];

    Gl_thread thread;
};

#endif

