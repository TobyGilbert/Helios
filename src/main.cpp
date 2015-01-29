#include <QApplication>
#include <QFile>
#include "mainwindow.h"

#define FULLSCREEN

int main(int argc, char **argv)
{
    QApplication app(argc,argv);
    MainWindow w;
    QFile file("styleSheet/darkOrange");
    file.open(QFile::ReadOnly);
    QString stylesheet = QLatin1String(file.readAll());
    w.setStyleSheet(stylesheet);
    w.setWindowTitle(QString("Helios"));
#ifdef FULLSCREEN
    w.showMaximized();
#endif
    w.show();
    app.exec();
}
