#include <QApplication>
#include <QFile>
#include "Core/mainwindow.h"
#include <QSplashScreen>
//#define FULLSCREEN

int main(int argc, char **argv)
{
    QApplication app(argc,argv);
    app.processEvents(QEventLoop::AllEvents);

    //create our application
    MainWindow w;
    QFile file("styleSheet/darkOrange");
    file.open(QFile::ReadOnly);
    QString stylesheet = QLatin1String(file.readAll());
    w.setStyleSheet(stylesheet);
    w.setWindowTitle(QString("Helios"));

    //create our loading screen
    QPixmap loadingPicture("./icons/splashscreen.png");
    QSplashScreen loadingScreen(loadingPicture.scaled(675,460),Qt::WindowStaysOnTopHint);
    loadingScreen.show();
    loadingScreen.setMaximumSize(QSize(675,460));
    app.processEvents();


#ifdef FULLSCREEN
    w.showMaximized();
#endif
    w.show();
    loadingScreen.hide();
    app.exec();

}
