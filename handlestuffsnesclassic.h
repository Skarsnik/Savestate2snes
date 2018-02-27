#ifndef HANDLESTUFFSNESCLASSIC_H
#define HANDLESTUFFSNESCLASSIC_H

#include "handlestuff.h"

#include <snesclassicstuff/desktopclient/miniftp.h>
#include <snesclassicstuff/desktopclient/telnetconnection.h>

class HandleStuffSnesClassic : public HandleStuff
{
public:
    HandleStuffSnesClassic();
    void            setCommandCo(TelnetConnection* co, TelnetConnection *canoe);
    void            setShortcutLoad(quint16 shortcut);
    void            setShortcutSave(quint16 shortcut);
    QByteArray      getScreenshotData();
    quint16         shortcutLoad();
    quint16         shortcutSave();
    bool            hasScreenshots();
    bool            hasShortcutsEdit();

protected:
    QByteArray      saveState(bool trigger);
    void            loadState(QByteArray data);


private:
    TelnetConnection*        telCo;
    TelnetConnection*        canoeCo;
    MiniFtp*                 ftpCo;
    QByteArray               lastLoadMD5;

    void        runCanoe(QStringList canoeArgs);
    QStringList getCanoeExecution();
    void        killCanoe(int signal);
    void removeCanoeUnnecessaryArg(QStringList &canoeRun);
};

#endif // HANDLESTUFFSNESCLASSIC_H
