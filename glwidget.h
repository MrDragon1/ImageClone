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
// Data structures that hold the information on the selection
typedef struct  {
    int numPoints;
    int numTriangles;
    int boundarySize;

    Point *  vertices;   // 2D coordinates of the mesh
    Triangles * triangles;  // Indices into vertices
    Point * boundaryCoords;
    float * boundaryTex;
    float * weightsTex;
    QVector<QVector<float>> diff;
    // Trnsformation vars
    float tx, ty; // Accum. translation coords
    float dx, dy; // Current translation coords

} Selection;


class GLWidget:public QGLWidget
{
    Q_OBJECT
public:
    GLWidget(QWidget *parent = 0);
    void updateSelection(std::vector<CDTPoint> & boundaryVector, QPolygonF & selectionPoly, bool reset = true);

    void setTargetImage(const QImage & image);
    void setSourcePatch(const QImage & image);
    void bindTarget();
    void bindSource();
    void paintSelection();
    void paintMesh();
    void paintDot();
    double CalcWeight(Point x,Point v1,Point v2,Point v3);
    void notInit() { init = false; }
    bool PointinTriangle(Point A,Point B,Point C,Point P);
    void OriginalBlend();
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
    int target_w, target_h;
    int source_w, source_h;
    int selection_w, selection_h;

    GLenum target; // as in texture target (not source/target)
    GLuint tex_tar_id, tex_src_id, tex_weight_id,tex_boundarycoords_id;

    QOpenGLShaderProgram * program;
    QOpenGLShaderProgram * bgprogram;
    QOpenGLBuffer* VBO;
    QOpenGLVertexArrayObject* VAO;
    QOpenGLTexture tex_tar,tex_src,tex_weight,tex_boundarycoords;
    std::vector<CDTPoint> boundaryVector;
    int click_x, click_y; // Coordinates of the mouse click which started the current event

    QPolygonF selectionPoly;

    QImage * sourceImage;
    QImage * targetImage;

};
#endif // GLWIDGET_H
