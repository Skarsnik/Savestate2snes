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
    qDebug() << "Loading games" << listDir.size();
    foreach(QFileInfo fi, listDir)
    {
        games << fi.baseName();
        qDebug() << fi.baseName();
    }
    return games;
}

QStandardItem *HandleStuff::loadCategories(QString game)
{
    qDebug() << "Loading categories for " << game;
    if (!categories.contains(game))
    {
        QStandardItem *root = new QStandardItem();
        saveDirectory.cd(game);
        root->setData(saveDirectory.absolutePath(), MyRolePath);
        categoriesByPath[root->data(MyRolePath).toString()] = root;
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
        qDebug() << "Game has file info";
        QSettings file(saveDirectory.absolutePath() + "/" + game + "/" + GAMEINFOS, QSettings::IniFormat);
        m_gameInfo.loadShortcut = file.value("_/loadShorcut").toString().toUInt(NULL, 16);
        m_gameInfo.saveShortcut = file.value("_/saveShorcut").toString().toUInt(NULL, 16);
        m_gameInfo.name = file.value("_/game").toString();

    }
    return categories[game];
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
    QFile saveFile(catLoaded->data(MyRolePath).toString() + "/" + name + ".svt");
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
    return false;
}

void    HandleStuff::findCategory(QStandardItem* parent, QDir dir)
{
    QFileInfoList listDir = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(QFileInfo fi, listDir)
    {
        QStandardItem* item = new QStandardItem(fi.baseName());
        item->setData(fi.absoluteFilePath(), MyRolePath);
        categoriesByPath[item->data(MyRolePath).toString()] = item;
        dir.cd(fi.baseName());
        findCategory(item, dir);
        dir.cdUp();
        parent->appendRow(item);
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
        }
    }
}


bool HandleStuff::addGame(QString newGame)
{
    qDebug() << "Add game " << newGame;
    if (saveDirectory.mkdir(newGame))
    {
        games.append(newGame);
        QStandardItem *newItem = new QStandardItem();
        categories[newGame] = newItem;
        saveDirectory.cd(newGame);
        newItem->setData(saveDirectory.absolutePath(), MyRolePath);
        saveDirectory.cdUp();
        return true;
    }
    return false;
}

void HandleStuff::setSaveStateDir(QString dir)
{
    saveDirectory.setPath(dir);
}

bool HandleStuff::addCategory(QStandardItem *newCategory, QStandardItem *parent)
{
    QString parentPath;
    if (parent->index().isValid())
        parentPath = parent->data(MyRolePath).toString();
    else
        parentPath = saveDirectory.absolutePath() + "/" + gameLoaded;
    QFileInfo fi(parentPath + "/" + newCategory->text());
    QDir di(parentPath);
    di.mkdir(newCategory->text());
    newCategory->setData(fi.absoluteFilePath(), MyRolePath);
    QStandardItem* cloned = newCategory->clone();
    categoriesByPath[newCategory->data(MyRolePath).toString()] = cloned;
    if (parent->index().isValid())
        categoriesByPath[parentPath]->appendRow(cloned);
    else
        categories[gameLoaded]->appendRow(cloned);
    parent->appendRow(newCategory);
    return true;
}

bool HandleStuff::addSubCategory(QStandardItem *newCategory, QStandardItem *parent)
{
    QString parentPath = parent->data(MyRolePath).toString();
    qDebug() << parent->text() << parentPath;
    QFileInfo fi(parentPath + "/" + newCategory->text());
    QDir di(parentPath);
    di.mkdir(newCategory->text());
    newCategory->setData(fi.absoluteFilePath(), MyRolePath);
    categoriesByPath[parentPath]->appendRow(newCategory->clone());
    parent->appendRow(newCategory);
    QStandardItem* cloned = newCategory->clone();
    categoriesByPath[newCategory->data(MyRolePath).toString()] = cloned;
    categoriesByPath[parentPath]->appendRow(cloned);
    return true;
}

QStringList HandleStuff::loadSaveStates(QStandardItem *category)
{
    QString savePath = category->data(MyRolePath).toString();
    if (!saveStates.contains(savePath))
    {
        QStringList cachedList = getCacheOrderList(ORDERSAVEFILE, savePath);
        if (cachedList.isEmpty())
        {

            QDir dir(savePath);
            QFileInfoList fil = dir.entryInfoList(QDir::Files);
            foreach(QFileInfo fi, fil)
            {
                saveStates[savePath] << fi.baseName();
            }
        } else {
            foreach(QString s, cachedList)
            {
                saveStates[savePath] << QFileInfo(s).baseName();
            }
        }
    }
    catLoaded = category;
    return saveStates[savePath];
}

bool HandleStuff::addSaveState(QString name, bool trigger)
{
    //QStandardItem*  newItem = new QStandardItem(name);
    QFileInfo fi(catLoaded->data(MyRolePath).toString() + "/" + name + ".svt");
    saveStates[catLoaded->data(MyRolePath).toString()] << fi.baseName();
    writeCacheOrderFile(ORDERSAVEFILE, catLoaded->data(MyRolePath).toString());
    //QFile f(fi.absoluteFilePath()); f.open(QIODevice::WriteOnly); f.write("Hello"); f.close();
    QByteArray data = UsbSNESSaveState(trigger);
    QFile saveFile(fi.absoluteFilePath());
    if (saveFile.open(QIODevice::WriteOnly))
    {
        saveFile.write(data);
        saveFile.close();
        return true;
    }
    return false;
}

bool HandleStuff::removeCategory(QStandardItem *category)
{
    QString path = category->data(MyRolePath).toString();
    if (QDir::root().rmpath(path))
    {
        categoriesByPath[path]->parent()->removeRow(category->row());
        return true;
    }
    return false;
}

void HandleStuff::renameSaveState(QStandardItem *item)
{
    QStringList& saveList = saveStates[catLoaded->data(MyRolePath).toString()];
    QString dirPath = catLoaded->data(MyRolePath).toString();
    if (QFile::rename(dirPath + "/" + saveList.at(item->row()) + ".svt", dirPath + "/" + item->text() + ".svt")) {
        saveList[item->row()] = item->text();
        writeCacheOrderFile(ORDERSAVEFILE, catLoaded->data(MyRolePath).toString());
    }
}

void HandleStuff::changeStateOrder(int from, int to)
{
    QStringList& saveList = saveStates[catLoaded->data(MyRolePath).toString()];
    saveList.move(from, to);
    writeCacheOrderFile(ORDERSAVEFILE, catLoaded->data(MyRolePath).toString());
}

void HandleStuff::deleteSaveState(int row)
{
    QStringList& saveList = saveStates[catLoaded->data(MyRolePath).toString()];
    QString dirPath = catLoaded->data(MyRolePath).toString();
    QString filePath = dirPath + "/" + saveList.at(row) + ".svt";
    qDebug() << "Removing : " << filePath;
    QFile::remove(filePath);
    saveList.removeAt(row);
    writeCacheOrderFile(ORDERSAVEFILE, catLoaded->data(MyRolePath).toString());

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


