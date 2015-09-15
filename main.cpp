#include "MainWindow.h"
#include <QApplication>
#include "VSLapp.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    VSLapp::applicationSetup("US Army Research Laboratory");
    MainWindow w;
    w.show();

    return a.exec();
}


