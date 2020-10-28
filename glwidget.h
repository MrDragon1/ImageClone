#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QtGui/QOpenGLShaderProgram>
#include <iostream>
#include <QMouseEvent>
#include <QtGui>
#include <QtOpenGL>
#include <math.h>
#include <QCoreApplication>
#include <GL/glu.h>
#include "mesh.h"
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLExtraFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLTexture>
#include <opencv2/opencv.hpp>
#include "vector3d.h"

typedef struct  {
    int numPoints;
    int numTriangles;
    int boundarySize;

    Point *  vertices;//store all the points
    Triangles * triangles;
    Point * boundaryCoords;//store the boundary points
    float * boundaryTex;//
    float * weightsTex;//
    QVector<QVector<float>> diff;//

    //store the shift between source picture and target picture
    float tx, ty;
    float dx, dy;

} Selection;


class GLWidget:public QGLWidget
{
    Q_OBJECT
public:
    GLWidget(QWidget *parent = nullptr);
    void updateSelection(std::vector<CDTPoint> & boundaryVector);

    void setTargetImage(const QImage & image);
    void setSourcePatch(const QImage & image);
    void bindTarget();
    void bindSource();
    void paintSelection();
    void paintMesh();
    double CalcWeight(Point x,Point v1,Point v2,Point v3);
    void notInit() { init = false; }

protected:
    void initializeGL();
    void loadShaders();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);

private:
    Selection selection;
    bool init;
    Mesh* mesh;

    // image size
    int target_w, target_h;
    int source_w, source_h;
    QImage * sourceImage;
    QImage * targetImage;

    GLenum target;
    GLuint tex_tar_id, tex_src_id, tex_weight_id,tex_boundarycoords_id;

    QOpenGLShaderProgram * program;
    QOpenGLShaderProgram * bgprogram;//draw the target picture in the background
    QOpenGLBuffer* VBO;
    QOpenGLVertexArrayObject* VAO;

    std::vector<CDTPoint> boundaryVector;//store the points on the boundary
    int click_x, click_y;//the start point of the boundary

};
#endif // GLWIDGET_H
