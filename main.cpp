#include "savestate2snesw.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Savestate2snesw w;
    w.show();

    return a.exec();
}
