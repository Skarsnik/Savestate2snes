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
#include <QLoggingCategory>
#include <QObject>
#include <QSettings>
#include <QCryptographicHash>
#include <QDir>
#include "handlestuff.h"

Q_LOGGING_CATEGORY(log_handleStuff, "HandleStuff")

#define sDebug() qCDebug(log_handleStuff)

#define ORDERCATFILE  "svt2snesordercat.txt"
#define ORDERSAVEFILE "svt2snesordersave.txt"
#define GAMEINFOS     "gameinfos.txt"

HandleStuff::HandleStuff() : QObject()
{
    connect(this, &HandleStuff::saveStateFinished, this, &HandleStuff::onSaveStateFinished);
    connect(this, &HandleStuff::loadStateFinished, this, &HandleStuff::loadSaveStateFinished);
    connect(this, &HandleStuff::screenshotDone, this, &HandleStuff::onScreenshotDone);
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
        if (fi.baseName() != "SNESClassic" && fi.baseName() != "USB2Snes" && fi.baseName() != "NWAccess")
        {
            games << fi.baseName();
            sDebug() << fi.baseName();
        }
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
        root->parent = nullptr;
        categoriesByPath[saveDirectory.absolutePath()] = root;
        findCategory(root, saveDirectory);
        saveDirectory.cdUp();
        categories[game] = root;
    }
    gameLoaded = game;
    m_gameInfo.loadShortcut = 0;
    m_gameInfo.saveShortcut = 0;
    m_gameInfo.memoryPreset.size = 0;
    m_gameInfo.memoryPreset.address = 0;
    m_gameInfo.memoryPreset.domain = "";
    m_gameInfo.name = game;
    m_gameInfo.checkMemory = false;
    QDir gDir(saveDirectory.absolutePath() + "/" + game);
    if (QFileInfo::exists(saveDirectory.absolutePath() + "/" + game + "/" + GAMEINFOS))
    {
        sDebug() << "Game has file info";
        QSettings file(saveDirectory.absolutePath() + "/" + game + "/" + GAMEINFOS, QSettings::IniFormat);
        bool ok;
        if (file.contains("_/loadShortcut"))
            m_gameInfo.loadShortcut = file.value("_/loadShortcut").toString().toUInt(&ok, 16);
        if (file.contains("_/saveShortcut"))
            m_gameInfo.saveShortcut = file.value("_/saveShortcut").toString().toUInt(&ok, 16);
        m_gameInfo.name = file.value("_/game").toString();
        sDebug() << "Shortcuts are : (s/l)" << QString::number(m_gameInfo.saveShortcut, 16) << QString::number(m_gameInfo.loadShortcut, 16);
        if (file.contains("_/memoryAddress"))
            m_gameInfo.memoryPreset.address = file.value("_/memoryAddress").toString().toULong(&ok, 16);
        if (file.contains("_/memorySize"))
        {
            m_gameInfo.memoryPreset.size = file.value("_/memorySize").toString().toUInt(&ok);
        }
        if (file.contains("_/memoryDomain"))
            m_gameInfo.memoryPreset.domain = file.value("_/memoryDomain").toString();
        if (file.contains("_/checkMemory"))
            m_gameInfo.checkMemory = file.value("_/checkMemory").toBool();
    }
    emit newGameLoaded();
    return categories[game]->children;
}

QIcon HandleStuff::getGameIcon(QString game)
{
    QDir gDir(saveDirectory.absolutePath() + "/" + game);
    foreach(QFileInfo fi, gDir.entryInfoList())
    {
        if (fi.baseName() == "icon")
            return QIcon(fi.absoluteFilePath());
    }
    return QIcon();
}

void    HandleStuff::findCategory(Category* parent, QDir dir)
{
    QFileInfoList listDir = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(QFileInfo fi, listDir)
    {
        if (fi.fileName() == "ScreenShots")
            continue;
        Category*   newCat = new Category();
        QDir catDir(fi.absoluteFilePath());
        foreach (QFileInfo catFi, catDir.entryInfoList(QDir::Files))
        {
            if (catFi.baseName() == "icon")
            {
                sDebug() << "Category" << fi.baseName() << "Has icon";
                newCat->icon = QIcon(catFi.absoluteFilePath());
            }
        }
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
            QFileInfo fi(dirPath + "/" + line.trimmed());
            if (fi.exists())
                toRet << fi.completeBaseName();
            else
                sDebug() << "File in cache : " << fi.absoluteFilePath() + " Does not exists";
        }
        cache.close();
    } else {
        sDebug() << "Can't open the cache file" << cache.errorString();
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
    sDebug() << "Adding game " << newGame;
    if (saveDirectory.mkdir(newGame))
    {
        games.append(newGame);
        Category*   newCat = new Category;
        newCat->parent = nullptr;
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
        return nullptr;
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
    if (hasScreenshots())
    {
        di.cd(newCategory);
        di.mkdir("ScreenShots");
    }
    return newCat;
}

QStringList HandleStuff::loadSaveStates(QString categoryPath)
{
    sDebug() << "Loading savestate for " << categoryPath;
    if (!saveStates.contains(categoryPath))
    {
        QStringList cachedList = getCacheOrderList(ORDERSAVEFILE, categoryPath);
        if (cachedList.isEmpty())
        {

            QDir dir(categoryPath);
            QFileInfoList fil = dir.entryInfoList(QDir::Files);
            foreach(QFileInfo fi, fil)
            {
                if (fi.fileName() == ORDERSAVEFILE || fi.fileName() == "ScreenShots")
                    continue;
                saveStates[categoryPath] << fi.baseName();
            }
        } else {
            sDebug() << "Category has cache file";
            foreach(QString s, cachedList)
            {
                saveStates[categoryPath] << QFileInfo(s).baseName();
            }
        }
    }
    catLoaded = categoriesByPath[categoryPath];
    return saveStates[categoryPath];
}


// Load & save
bool    HandleStuff::loadSaveState(QString name)
{
    if (!needByteData())
    {
        bool result = loadState(catLoaded->path + "/" + name + ".svt");
        if (result)
            return true;
        qCWarning(log_handleStuff()) << "Can't open savestate, refused by emulator";
        return false;
    }
    QFile saveFile(catLoaded->path + "/" + name + ".svt");
    if (saveFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = saveFile.readAll();
        loadState(data);
        saveFile.close();
        return true;
    }
    qCWarning(log_handleStuff()) << "Can't open savestate file : " << saveFile.errorString();
    return false;
}


bool HandleStuff::addSaveState(QString name, bool trigger)
{
    sDebug() << "Add Savestate state";
    QFileInfo fi(catLoaded->path + "/" + name + ".svt");
    saveStateFileInfo = fi;
    saveStates[catLoaded->path] << fi.baseName();
    //QFile f(fi.absoluteFilePath()); f.open(QIODevice::WriteOnly); f.write("Hello"); f.close();
    if (!needByteData())
    {
        bool result = saveState(fi.absoluteFilePath());
        if (result)
            return true;
        qCCritical(log_handleStuff()) << "Can't create file for savestate, refused by emulator";
        return false;
    }
    return saveState(trigger);
}

void    HandleStuff::onSaveStateFinished(bool success)
{
    sDebug() << "Savestate finished";
    if (!success)
    {
        emit addSaveStateFinished(success);
        return;
    }
    if (hasScreenshots() && hasPostSaveScreenshot())
    {
        doScreenshot();
    }
    /* TODO, preSaveScreenshot */
    if (needByteData())
    {
        QFile saveFile(saveStateFileInfo.absoluteFilePath());
        if (saveFile.open(QIODevice::WriteOnly))
        {
            saveFile.write(saveStateData);
            saveFile.close();
        } else {
            emit addSaveStateFinished(false);
            qCCritical(log_handleStuff()) << "Can't create file for savestate : " << saveFile.errorString();
        }
    }
    writeCacheOrderFile(ORDERSAVEFILE, catLoaded->path);
    emit addSaveStateFinished(true);
    return ;
}

void    HandleStuff::onScreenshotDone()
{
    if (hasPostSaveScreenshot())
        saveScreenshot();
}

void    HandleStuff::saveScreenshot()
{
    QByteArray scData = getScreenshotData();
    QFile scFile(catLoaded->path + "/ScreenShots/" + QCryptographicHash::hash(saveStateData, QCryptographicHash::Md5).toHex() + ".png");
    if (scFile.open(QIODevice::WriteOnly))
    {
        scFile.write(scData);
        scFile.close();
    }
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
            catLoaded = nullptr;
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

void HandleStuff::setCategoryIcon(QString categoryPath, QString iconPath)
{
    categoriesByPath[categoryPath]->icon = QIcon(iconPath);
}

QPixmap HandleStuff::getScreenshot(QString name)
{
    QFile saveFile(catLoaded->path + "/" + name + ".svt");
    if (saveFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = saveFile.readAll();
        QString screenShotFile = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex() + ".png";
        return QPixmap(catLoaded->path+ "/ScreenShots/" + screenShotFile);
    }
    return QPixmap();
}

QString HandleStuff::getScreenshotPath(QString name)
{
    QFile saveFile(catLoaded->path + "/" + name + ".svt");
    if (saveFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = saveFile.readAll();
        QString screenShotFile = QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex() + ".png";
        if (QFileInfo::exists(catLoaded->path + "/ScreenShots/" + screenShotFile))
            return catLoaded->path + "/ScreenShots/" + screenShotFile;
        return QString();
    }
    return QString();
}

QString HandleStuff::getSavestatePath(QString name)
{
    return catLoaded->path + "/" + name + ".svt";
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


void HandleStuff::setMemoryToWatch(MemoryPreset preset)
{
    m_gameInfo.memoryPreset = preset;
    QSettings file(saveDirectory.absolutePath() + "/" + gameLoaded + "/" + GAMEINFOS, QSettings::IniFormat);
    file.setValue("_/memoryAddress", QString::number(preset.address, 16));
    file.setValue("_/memorySize", QString::number(preset.size));
    file.setValue("_/memoryDomain", preset.domain);
}

void HandleStuff::saveMemoryCheck(bool value)
{
    QSettings file(saveDirectory.absolutePath() + "/" + gameLoaded + "/" + GAMEINFOS, QSettings::IniFormat);
    file.setValue("_/checkMemory", value);
}

void HandleStuff::savestateReady()
{
    sDebug() <<  "Nothing to do here";
}

void HandleStuff::savestateUnready()
{
    sDebug() <<  "Nothing to do here";
}

