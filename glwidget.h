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


//保存每一次绘制所需的信息
typedef struct  {
    int numPoints;
    int numTriangles;
    int boundarySize;

    Point *  vertices;//保存所有顶点
    Triangles * triangles;//保存mesh中的三角形
    Point * boundaryCoords;//保存边界的坐标
    float * boundaryTex;//纹理的原始数据
    float * weightsTex;//

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

    QOpenGLShaderProgram * program;//绘制融合后的图片
    QOpenGLShaderProgram * bgprogram;//绘制背景图片
    QOpenGLBuffer* VBO;
    QOpenGLVertexArrayObject* VAO;

    std::vector<CDTPoint> boundaryVector;//保存从sourcewidget接收的边界坐标
    int click_x, click_y;

};
#endif // GLWIDGET_H
