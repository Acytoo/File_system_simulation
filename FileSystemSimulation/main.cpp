#include "mainwindow.h"
#include "content.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    extern content fileContentWindow;

    return a.exec();
}
