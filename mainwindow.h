#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "glwidget.h"
#include "sourcewidget.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    void show() { QMainWindow::show(); quickOpen(); }
    ~MainWindow();
    void quickOpen();
    void keyPressEvent(QKeyEvent *event);
private:
    Ui::MainWindow *ui;
    void setImages(QString & source, QString & target);
    GLWidget * glWidget;
    SourceWidget * sourceWindow;
};

#endif // MAINWINDOW_H
