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

#ifndef HANDLESTUFF_H
#define HANDLESTUFF_H

#include "usb2snes.h"

#include <QString>
#include <QStandardItem>
#include <QDir>

#define MyRolePath Qt::UserRole + 1

struct  GameInfos
{
    quint16 saveShortcut;
    quint16 loadShortcut;
    QString name;
};

class HandleStuff
{
public:
    HandleStuff();

    QStringList loadGames();
    void    setUsb2snes(USB2snes* usbsnes);
    QByteArray UsbSNESSaveState(bool trigger);
    void    UsbSNESLoadState();
    QStandardItem*  loadCategories(QString game);
    bool    addGame(QString newGame);
    void    setSaveStateDir(QString dir);
    bool    addCategory(QStandardItem* newCategory, QStandardItem* parent);
    bool    addSubCategory(QStandardItem* newCategory, QStandardItem* parent);
    QStringList    loadSaveStates(QStandardItem* category);
    bool    addSaveState(QString name, bool trigger = true);
    bool    removeCategory(QStandardItem* category);
    bool    renameSaveState(QStandardItem* item);
    void    changeStateOrder(int from, int to);
    bool    loadSaveState(QString name);
    bool    deleteSaveState(int row);
    quint16 shortcutSave();
    quint16 shortcutLoad();
    void    setShortcutLoad(quint16 shortcut);
    void    setShortcutSave(quint16 shortcut);
    GameInfos    gameInfos();
    void    setGameShortCut(quint16 save, quint16 load);

private:
    QDir                                    saveDirectory;
    QStringList                             games;
    QMap<QString, QStandardItem*>           categories;
    QMap<QString, QStandardItem*>           categoriesByPath;
    QMap<QString, QStringList>              saveStates;
    QString                                 gameLoaded;
    QStandardItem*                          catLoaded;
    USB2snes*                               usb2snes;
    GameInfos                               m_gameInfo;

    void findCategory(QStandardItem *parent, QDir dir);
    QStringList getCacheOrderList(QString file, QString dirPath);
    void        writeCacheOrderFile(QString file, QString dirPath);
    void checkForSafeState();
};

#endif // HANDLESTUFF_H
