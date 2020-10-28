#include <QtGui>
#include <QMessageBox>
#include <iostream>
#include "sourcewidget.h"
#include "glwidget.h"

using std::cout;
using std::endl;

SourceWidget::SourceWidget(QWidget *parent, GLWidget *gl)
        : QDialog(parent){
    setAttribute(Qt::WA_StaticContents);
    myPenWidth = 3;
    myPenColor = Qt::blue;
    setWindowTitle(tr("Source Image"));
    glWidget = gl;

}

void SourceWidget::setImage(const QImage &img){
    path = QPainterPath();
    image = img;
    update();
}

void SourceWidget::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton) {
        lastPoint = event->pos();
        path = QPainterPath(event->pos());
        update();
    }
}

void SourceWidget::mouseMoveEvent(QMouseEvent *event){

    if ((event->buttons() & Qt::LeftButton)) {
        path.lineTo(event->pos());
        update();
    }
}

// Find next boundary pixel starting from position (x,y) which is guaranteed to be
// on the boundary, while previous direction was dir
QPoint SourceWidget::findStartingFromDir(QPolygonF & selectionPoly, int x, int y, int & dir){

    bool N,W,S,E;
    N = selectionPoly.containsPoint(QPointF(x,y+1),Qt::WindingFill);
    W = selectionPoly.containsPoint(QPointF(x-1,y),Qt::WindingFill);
    S = selectionPoly.containsPoint(QPointF(x,y-1),Qt::WindingFill);
    E = selectionPoly.containsPoint(QPointF(x+1,y),Qt::WindingFill);

    dir = (dir+3)%4;
    if (dir==0){
        if (E) {
            dir = 0;
            return QPoint(x+1,y);
        }
        if (N){
            dir = 1;
            return QPoint(x,y+1);
        }
        if (W){
            dir = 2;
            return QPoint(x-1,y);
        }
        if (S){
            dir = 3;
            return QPoint(x,y-1);
        }
    }
    if (dir==1){
        if (N){
            dir = 1;
            return QPoint(x,y+1);
        }
        if (W){
            dir = 2;
            return QPoint(x-1,y);
        }
        if (S){
            dir = 3;
            return QPoint(x,y-1);
        }
        if (E){
            dir = 0;
            return QPoint(x+1,y);
        }
    }
    if (dir==2){
        if (W){
            dir = 2;
            return QPoint(x-1,y);
        }
        if (S){
            dir = 3;
            return QPoint(x,y-1);
        }
        if (E){
            dir = 0;
            return QPoint(x+1,y);
        }
        if (N){
            dir = 1;
            return QPoint(x,y+1);
        }
    }
    if (dir==3){
        if (S){
            dir = 3;
            return QPoint(x,y-1);
        }
        if (E){
            dir = 0;
            return QPoint(x+1,y);
        }
        if (N){
            dir = 1;
            return QPoint(x,y+1);
        }
        if (W){
            dir = 2;
            return QPoint(x-1,y);
        }
    }

    //throw std::exception();
    assert("Wrong path in findStartingFromDir");
    return QPoint(-1,-1);
}

void SourceWidget::mouseReleaseEvent(QMouseEvent *event)
{
    // Once the user releases the mouse, the selection process is finished. We need to
    // extract the selection and update the glWidget
    if (event->button() == Qt::LeftButton) {

        // Draw the last line
        path.lineTo(event->pos());
        path.connectPath(path);
        update();

        if (path.length()<20)
            return;

        QPolygonF selectionPoly = path.toFillPolygon();
        QRectF boundingRect = selectionPoly.boundingRect();


        // Don't pass bad selections on
        if (!(boundingRect.width()>20 && boundingRect.height()>20)) return;

        // adjusted - boundary value will be cut without it
        QImage sourcePatch = image.copy(boundingRect.toRect().adjusted(-1,-1,1,1));

        // Pass source patch pixels to glWidet
        glWidget->setSourcePatch(sourcePatch);

        qreal x0,x1,y0,y1;
        boundingRect.getCoords(&x0,&y0,&x1,&y1);

        // Find a point on the boundary of the selection
        QPoint cPoint(-1,-1);
        for (int x=x0; x<=x1; x++) {
            for (int y=y0; y<=y1; y++) {
                if (selectionPoly.containsPoint(QPointF(x,y),Qt::WindingFill))
                {
                    cPoint = QPoint(x,y);
                }

            }
        }

        //assert(cPoint != QPoint(-1,-1));
        if (cPoint == QPoint(-1,-1)){
            qDebug() << "assert(cPoint != QPoint(-1,-1)) fails";
            return;
        }
        // Track the boundary of the selection
        std::vector<CDTPoint> boundaryVector;

        int dir = 1;
        int x,y;
        int c=0;
        bool cont = true;
        do {
            x = cPoint.x();
            y = cPoint.y();
            cPoint = findStartingFromDir(selectionPoly ,x, y, dir);
            boundaryVector.push_back(CDTPoint((cPoint.x()-x0),((y1-y0) - (cPoint.y()-y0))));
            if (boundaryVector.size()>3){
                if(boundaryVector[0]==boundaryVector[c-1] && boundaryVector[1]==boundaryVector[c]){
                    boundaryVector.pop_back();
                    boundaryVector.pop_back();
                    cont = false;
                }
            }
            c++;
        } while (cont);
        if (boundaryVector.size()>8192){
            path = QPainterPath();
            update();
            QMessageBox::warning(this, tr("Boundary Size"),
                               tr("Largest supported boundary size is 8192"));
            return;
        }
        glWidget->updateSelection(boundaryVector);
        glWidget->update();
    }
}

void SourceWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);
    painter.setPen(QPen(myPenColor, myPenWidth, Qt::SolidLine, Qt::RoundCap,
                        Qt::RoundJoin));

    painter.drawImage(QPoint(0, 0), image);

    painter.drawPath(path);
    update();
}


