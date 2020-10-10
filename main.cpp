






#include <QApplication>

#include "mainwindow.h"
#include "Utils/ylogger.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    initLoggerModule();

    MainWindow w;
    w.show();
    return a.exec();
}
