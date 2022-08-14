#ifndef HANDLESTUFFSNESCLASSIC_H
#define HANDLESTUFFSNESCLASSIC_H

#include <QSignalSpy>
#include "handlestuff.h"
#include "snesclassicstuff/stuffclient/stuffclient.h"

class HandleStuffSnesClassic : public HandleStuff
{
public:
    HandleStuffSnesClassic();
    void            setControlCo(StuffClient* client);
    void            setShortcutLoad(quint16 shortcut);
    void            setShortcutSave(quint16 shortcut);
    QByteArray      getScreenshotData();
    quint16         shortcutLoad();
    quint16         shortcutSave();
    bool            hasScreenshots();
    bool            hasShortcutsEdit();
    bool            hasMemoryWatch();
    void            startMemoryWatch();
    void            stopMemoryWatch();
    void            controllerSaveState();
    void            controllerLoadState();

protected:
    bool            saveState(bool trigger);
    void            loadState(QByteArray data);
    bool            needByteData();
    bool            saveState(QString path);
    bool            loadState(QString path);
    bool            hasPostSaveScreenshot();
    bool            doScreenshot();


private:
    StuffClient*            controlCo;
    QByteArray              lastLoadMD5;
    QByteArray              screenshotData;
    quint16                 saveShortcut;
    quint16                 loadShortcut;
    uint                    fileReceivedSize;
    bool                    expectingSaveFile;

    QSignalSpy*              commandSpy;
    void        runCanoe(QStringList canoeArgs);
    QStringList getCanoeExecution();
    void        killCanoe(int signal);
    void        removeCanoeUnnecessaryArg(QStringList &canoeRun);
    bool        mySaveState(bool trigger, bool noGet);
    void        myLoadState(QByteArray data, bool noPut);


    bool fakeWaitForCommand(QByteArray cmd, unsigned int timeout = 200);

};

#endif // HANDLESTUFFSNESCLASSIC_H
