#ifndef SOURCEWIDGET_H
#define SOURCEWIDGET_H


#include <QColor>
#include <QImage>
#include <QPoint>
#include <QWidget>
#include <QDialog>
#include "mesh.h"
#include "glwidget.h"

class SourceWidget : public QDialog
{
    Q_OBJECT
public:
    SourceWidget(QWidget * parent, GLWidget* gl);
    void setImage(const QImage & img);
protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);
    void paintEvent(QPaintEvent * event);
private:
    QPoint findStartingFromDir(QPolygonF & selectionPoly, int x, int y, int & dir);

    GLWidget * glWidget;

    QPainterPath path;

    QPoint lastPoint;
    int myPenWidth;
    QColor myPenColor;
    QImage image;

};

#endif
