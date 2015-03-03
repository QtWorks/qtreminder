#include "taskview.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TaskView w;
    w.show();
//    qDebug()<<sizeof(TaskView);
    return a.exec();
}
