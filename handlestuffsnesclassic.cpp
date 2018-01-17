#include "handlestuffsnesclassic.h"

#define CLOVERSAVESTATEPATH "/tmp/savestate2snes.svt"
#define CLOVERROLLBACKPATH "/tmp/rollback/"
#define SCREENSHOTPATH "/tmp/savestate2snes.png"
#define KILLCANOECMD "kill -2 `pidof canoe-shvc`"

HandleStuffSnesClassic::HandleStuffSnesClassic()
{

}

QStringList HandleStuffSnesClassic::getCanoeExecution()
{
    QByteArray ba = telCo->syncExecuteCommand("ps | grep canoe | grep -v grep");
    QString result = ba.trimmed();
    QString canoeStr = result.mid(result.indexOf("canoe"));
    QStringList canoeArgs = canoeStr.split(" ");
    return canoeArgs;
}

void    HandleStuffSnesClassic::killCanoe()
{
    telCo->syncExecuteCommand(KILLCANOECMD);
    telCo->syncExecuteCommand("test -e /tmp/plop || (kill -2 `pidof ReedPlayer-Clover` && touch /tmp/plop)");
}

void    HandleStuffSnesClassic::runCanoe(QStringList canoeArgs, QString stateFile)
{
    int saveLPos = canoeArgs.indexOf("--load-time-path");
    int rollPos = canoeArgs.indexOf("-rollback-input-dir");
    int saveSPos = canoeArgs.indexOf("--save-time-path");
    if (saveLPos != -1)
    {
        canoeArgs.removeAt(saveLPos);
        canoeArgs.removeAt(saveLPos);
    }
    canoeArgs.removeAt(saveSPos);
    canoeArgs.removeAt(saveSPos);
    canoeArgs.removeAt(rollPos);
    canoeArgs.removeAt(rollPos);
    /*canoeArgs.append("-rollback-input-dir");
    canoeArgs.append(CLOVERROLLBACKPATH); // FIXME*/
    canoeArgs.append("--load-time-path");
    canoeArgs.append(stateFile);
    canoeArgs.append("--save-time-path");
    canoeArgs.append(CLOVERSAVESTATEPATH);
    telCo->syncExecuteCommand(canoeArgs.join(" "));
}

QByteArray HandleStuffSnesClassic::saveState(bool trigger)
{
    Q_UNUSED(trigger)
    QStringList canoeRun = getCanoeExecution();
    QString fileSavePath = canoeRun.at(canoeRun.indexOf("--save-time-path") + 1);
    killCanoe();
    runCanoe(canoeRun, fileSavePath);
    return ftpCo->get(fileSavePath);
}

void HandleStuffSnesClassic::loadState(QByteArray data)
{
    QStringList  ce = getCanoeExecution();
    telCo->syncExecuteCommand(KILLCANOECMD);
    ftpCo->put(CLOVERSAVESTATEPATH, data);
    runCanoe(ce, CLOVERSAVESTATEPATH);
}
