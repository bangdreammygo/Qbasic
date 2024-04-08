#include "mainbasic.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    mainBasic w;
    w.show();
    return a.exec();
}
