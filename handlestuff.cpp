/*
    This file is part of the SaveState2snes software
    Copyright (C) 2017  Sylvain "Skarsnik" Colinet <scolinet@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <QDebug>
#include "handlestuff.h"

Q_LOGGING_CATEGORY(log_handleStuff, "HandleStuff")

#define sDebug() qCDebug(log_handleStuff)

#define ORDERCATFILE  "svt2snesordercat.txt"
#define ORDERSAVEFILE "svt2snesordersave.txt"
#define GAMEINFOS     "gameinfos.txt"

HandleStuff::HandleStuff()
{

}

QStringList HandleStuff::loadGames()
{
    games.clear();
    gameLoaded.clear();
    categories.clear();
    saveStates.clear();
    categoriesByPath.clear();

    QFileInfoList listDir = saveDirectory.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    sDebug() << "Loading games" << listDir.size();
    foreach(QFileInfo fi, listDir)
    {
        games << fi.baseName();
        sDebug() << fi.baseName();
    }
    return games;
}

QVector<Category *> HandleStuff::loadCategories(QString game)
{
    sDebug() << "Loading categories for " << game;
    if (!categories.contains(game))
    {
        Category* root = new Category();
        saveDirectory.cd(game);
        root->parent = NULL;
        categoriesByPath[saveDirectory.absolutePath()] = root;
        findCategory(root, saveDirectory);
        saveDirectory.cdUp();
        categories[game] = root;
    }
    gameLoaded = game;
    m_gameInfo.loadShortcut = 0;
    m_gameInfo.saveShortcut = 0;
    m_gameInfo.name = game;
    if (QFileInfo::exists(saveDirectory.absolutePath() + "/" + game + "/" + GAMEINFOS))
    {
        sDebug() << "Game has file info";
        QSettings file(saveDirectory.absolutePath() + "/" + game + "/" + GAMEINFOS, QSettings::IniFormat);
        m_gameInfo.loadShortcut = file.value("_/loadShorcut").toString().toUInt(NULL, 16);
        m_gameInfo.saveShortcut = file.value("_/saveShorcut").toString().toUInt(NULL, 16);
        m_gameInfo.name = file.value("_/game").toString();

    }
    return categories[game]->children;
}

void HandleStuff::setUsb2snes(USB2snes *usbsnes)
{
    usb2snes = usbsnes;
}

//$FC2000 db saveState  (Make sure both load and save are zero before setting this to nonzero)
//$FC2001 db loadState  (Make sure both load and save are zero before setting this to nonzero)
//$FC2002 dw saveButton (Set to $FFFF first and then set to correct value)
//$FC2004 dw loadButton (Set to $FFFF first and then set to correct value)

QByteArray HandleStuff::UsbSNESSaveState(bool trigger)
{
    QByteArray data;
    if (trigger)
    {
        checkForSafeState();
        data.resize(2);
        data[0] = 0;
        /*data[1] = 0;
        usb2snes->setAddress(0xFC2000, data);*/
        data[0] = 1;
        usb2snes->setAddress(0xFC2000, data);
    }
    checkForSafeState();
    QByteArray saveData = usb2snes->getAddress(0xF00000, 320 * 1024);
    return saveData;
}

void    HandleStuff::checkForSafeState()
{
    QByteArray data = usb2snes->getAddress(0xFC2000, 2);
    while (!(data.at(0) == 0 && data.at(1) == 0))
    {
            QThread::usleep(100);
            data = usb2snes->getAddress(0xFC2000, 2);
    }
}

bool    HandleStuff::loadSaveState(QString name)
{
    QFile saveFile(catLoaded->path + "/" + name + ".svt");
    if (saveFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = saveFile.readAll();
        checkForSafeState();
        usb2snes->setAddress(0xF00000, data);
        data.resize(2);
        data[0] = 0;
        /*data[1] = 0;
        usb2snes->setAddress(0xFC2000, data);*/
        data[1] = 1;
        usb2snes->setAddress(0xFC2000, data);
        saveFile.close();
        return true;
    }
    qCWarning(log_handleStuff()) << "Can't open savestate file : " << saveFile.errorString();
    return false;
}

void    HandleStuff::findCategory(Category* parent, QDir dir)
{
    QFileInfoList listDir = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(QFileInfo fi, listDir)
    {
        Category*   newCat = new Category();
        newCat->name = fi.baseName();
        newCat->path = fi.absoluteFilePath();
        newCat->parent = parent;
        newCat->parent->children.append(newCat);
        categoriesByPath[newCat->path] = newCat;
        dir.cd(fi.baseName());
        findCategory(newCat, dir);
        dir.cdUp();
    }
}

QStringList HandleStuff::getCacheOrderList(QString file, QString dirPath)
{
    QStringList toRet;
    QFile cache(dirPath + "/" + file);
    if (cache.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        while (!cache.atEnd())
        {
            QString line = cache.readLine();
            QFileInfo fi(dirPath + "/" + line);
            if (fi.exists())
                toRet << fi.completeBaseName();
        }
        cache.close();
    }
    return toRet;
}

void HandleStuff::writeCacheOrderFile(QString file, QString dirPath)
{
    if (file == ORDERSAVEFILE)
    {
        QFile   cache(dirPath + "/" + file);
        if (cache.open(QIODevice::WriteOnly))
        {
           foreach(QString save, saveStates[dirPath])
           {
               cache.write(QByteArray(save.toUtf8() + ".svt\n"));
           }
           cache.close();
        } else {
            qCWarning(log_handleStuff()) << "Can't write to cache file : " << cache.errorString();
        }
    }
}


bool HandleStuff::addGame(QString newGame)
{
    sDebug() << "Add game " << newGame;
    if (saveDirectory.mkdir(newGame))
    {
        games.append(newGame);
        Category*   newCat = new Category;
        newCat->parent = NULL;
        categories[newGame] = newCat;
        return true;
    } else {
        qCCritical(log_handleStuff()) << "Can't create the directory for the game " << newGame;
    }
    return false;
}

void HandleStuff::setSaveStateDir(QString dir)
{
    saveDirectory.setPath(dir);
}

Category* HandleStuff::addCategory(QString newCategory, QString parentPath)
{
    if (parentPath.isEmpty())
        parentPath = saveDirectory.absolutePath() + "/" + gameLoaded;
    QFileInfo fi(parentPath + "/" + newCategory);
    QDir di(parentPath);
    if (!di.mkdir(newCategory))
    {
        qCCritical(log_handleStuff()) << "Can't create a new category directory " << newCategory;
        return NULL;
    }
    Category* newCat = new Category();
    newCat->name = newCategory;
    newCat->path = fi.absoluteFilePath();
    if (categoriesByPath.contains(parentPath))
        newCat->parent = categoriesByPath[parentPath];
    else
        newCat->parent = categories[gameLoaded];
    categoriesByPath[newCat->path] = newCat;
    newCat->parent->children.append(newCat);
    return newCat;
}

QStringList HandleStuff::loadSaveStates(QString categoryPath)
{
    if (!saveStates.contains(categoryPath))
    {
        QStringList cachedList = getCacheOrderList(ORDERSAVEFILE, categoryPath);
        if (cachedList.isEmpty())
        {

            QDir dir(categoryPath);
            QFileInfoList fil = dir.entryInfoList(QDir::Files);
            foreach(QFileInfo fi, fil)
            {
                if (fi.fileName() == ORDERSAVEFILE)
                    continue;
                saveStates[categoryPath] << fi.baseName();
            }
        } else {
            foreach(QString s, cachedList)
            {
                saveStates[categoryPath] << QFileInfo(s).baseName();
            }
        }
    }
    catLoaded = categoriesByPath[categoryPath];
    return saveStates[categoryPath];
}

bool HandleStuff::addSaveState(QString name, bool trigger)
{
    QFileInfo fi(catLoaded->path + "/" + name + ".svt");
    saveStates[catLoaded->path] << fi.baseName();
    writeCacheOrderFile(ORDERSAVEFILE, catLoaded->path);
    //QFile f(fi.absoluteFilePath()); f.open(QIODevice::WriteOnly); f.write("Hello"); f.close();
    QByteArray data = UsbSNESSaveState(trigger);
    QFile saveFile(fi.absoluteFilePath());
    if (saveFile.open(QIODevice::WriteOnly))
    {
        saveFile.write(data);
        saveFile.close();
        return true;
    } else {
        qCCritical(log_handleStuff()) << "Can't create file for savestate : " << saveFile.errorString();
    }
    return false;
}

bool HandleStuff::removeCategory(QString categoryPath)
{
    sDebug() << "remove category" << categoryPath;
    QDir dir = QDir::root();
    dir.cd(categoryPath);
    if (dir.removeRecursively())
    {
        Category* cat = categoriesByPath.take(categoryPath);
        cat->parent->children.remove(cat->parent->children.indexOf(cat));
        if (catLoaded == cat)
            catLoaded = NULL;
        delete cat;
        return true;
    }
    qCWarning(log_handleStuff()) << "Can't remove category directory";
    return false;
}

bool HandleStuff::renameSaveState(int row, QString newName)
{
    QStringList& saveList = saveStates[catLoaded->path];
    QString dirPath = catLoaded->path;
    if (QFile::rename(dirPath + "/" + saveList.at(row) + ".svt", dirPath + "/" + newName + ".svt")) {
        saveList[row] = newName;
        writeCacheOrderFile(ORDERSAVEFILE, catLoaded->path);
        return true;
    }
    qCInfo(log_handleStuff()) << "Can't rename savestate";
    return false;
}

void HandleStuff::changeStateOrder(int from, int to)
{
    QStringList& saveList = saveStates[catLoaded->path];
    saveList.move(from, to);
    writeCacheOrderFile(ORDERSAVEFILE, catLoaded->path);
}

bool HandleStuff::deleteSaveState(int row)
{
    QStringList& saveList = saveStates[catLoaded->path];
    QString dirPath = catLoaded->path;
    QString filePath = dirPath + "/" + saveList.at(row) + ".svt";
    sDebug() << "Removing : " << filePath;
    if (!QFile::remove(filePath))
    {
        qCWarning(log_handleStuff()) << "Can't remove savestate";
        return false;
    }
    saveList.removeAt(row);
    writeCacheOrderFile(ORDERSAVEFILE, catLoaded->path);
    return true;
}

quint16 HandleStuff::shortcutSave()
{
    QByteArray saveButton = usb2snes->getAddress(0xFC2002, 2);
    quint16 toret = (saveButton.at(1) << 8) + saveButton.at(0);
    return toret;
}

quint16 HandleStuff::shortcutLoad()
{
    QByteArray loadButton = usb2snes->getAddress(0xFC2004, 2);
    quint16 toret = (loadButton.at(1) << 8) + loadButton.at(0);
    return toret;
}

void HandleStuff::setShortcutLoad(quint16 shortcut)
{
    QByteArray data;
    data.resize(2);
    data[0] = shortcut & 0x00FF;
    data[1] = shortcut >> 8;
    usb2snes->setAddress(0xFC2004, data);
}

void HandleStuff::setShortcutSave(quint16 shortcut)
{
    QByteArray data;
    data.resize(2);
    data[0] = shortcut & 0x00FF;
    data[1] = shortcut >> 8;
    usb2snes->setAddress(0xFC2002, data);
}

GameInfos HandleStuff::gameInfos()
{
    return m_gameInfo;
}

void HandleStuff::setGameShortCut(quint16 save, quint16 load)
{
    QSettings file(saveDirectory.absolutePath() + "/" + gameLoaded + "/" + GAMEINFOS, QSettings::IniFormat);
    file.setValue("_/saveShortcut", QString::number(save, 16));
    file.setValue("_/loadShortcut", QString::number(load, 16));
}


