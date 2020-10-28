#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
MainWindow::MainWindow()
{
    glWidget = new GLWidget(this);
    setCentralWidget(glWidget);
    glWidget->setFixedSize(400, 300);

    setWindowTitle(tr("Target Image"));
    layout()->setSizeConstraint(QLayout::SetFixedSize);

    sourceWindow = nullptr;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::quickOpen()
{
    QString source(":/image/source.jpg");
    QString target(":/image/target.jpg");
    setImages(source, target);
}

void MainWindow::setImages(QString & source, QString & target){

    if (!source.isEmpty()) {
        QImage image(source);
        if (image.isNull()) {
            QMessageBox::information(this, tr("MVCloner"),
                                     tr("Cannot load %1.").arg(source));
            return;
        }
        if (!sourceWindow)
            sourceWindow = new SourceWidget(this, glWidget);
        sourceWindow->setImage(image);
        sourceWindow->setFixedSize(image.width(), image.height());
        sourceWindow->show();
     }
    if (!target.isEmpty()) {
        QImage image(target);
        if (image.isNull()) {
            QMessageBox::information(this, tr("MVCloner"),
                                     tr("Cannot load %1.").arg(target));
            return;
        }
        glWidget->setTargetImage(image);
        glWidget->resize(image.width(), image.height());
        glWidget->setFixedSize(image.width(), image.height());
        glWidget->show();
    }

    glWidget->notInit();
    glWidget->update();
}



