#include "mafenetre.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MaFenetre w;
    w.show();
    return a.exec();
}
