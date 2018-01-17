#ifndef HANDLESTUFFSNESCLASSIC_H
#define HANDLESTUFFSNESCLASSIC_H

#include "handlestuff.h"

#include <snesclassicstuff/desktopclient/miniftp.h>
#include <snesclassicstuff/desktopclient/telnetconnection.h>

class HandleStuffSnesClassic : public HandleStuff
{
public:
    HandleStuffSnesClassic();

protected:
    QByteArray    saveState(bool trigger);
    void          loadState(QByteArray data);

private:
    TelnetConnection*        telCo;
    MiniFtp*                 ftpCo;

    void        runCanoe(QStringList canoeArgs, QString stateFile);
    QStringList getCanoeExecution();
};

#endif // HANDLESTUFFSNESCLASSIC_H
