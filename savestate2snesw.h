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


#ifndef SAVESTATE2SNESW_H
#define SAVESTATE2SNESW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "handlestuff.h"
#include "usb2snes.h"
#include "consoleswitcher.h"

namespace Ui {
class Savestate2snesw;
}

class Savestate2snesw : public QMainWindow
{
    Q_OBJECT

public:
    explicit Savestate2snesw(QWidget *parent = 0);
    ~Savestate2snesw();

private slots:
    void    onModeChanged(ConsoleSwitcher::Mode mode);

    void    saveListShowContextMenu(QPoint point);
    void    categoryListShowContextMenu(QPoint point);

    void on_actionRemoveCategory_triggered();

    void on_actionAddCategory_triggered();

    void on_actionAddSubCategory_triggered();

    void on_newGamePushButton_clicked();

    void on_gameComboBox_currentIndexChanged(const QString &arg1);

    void on_addSaveStatePushButton_clicked();

    void    saveStateItemChanged(QStandardItem* item);

    void on_categoryTreeView_clicked(const QModelIndex &index);
    //void    onSaveStateModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int> ());
    void    onSaveStateDelegateDataCommited(QWidget *e);
    void    saveStateRowRemoved(QModelIndex p, int begin, int last);
    void    saveStateRowInserted(QModelIndex p, int begin, int last);
    void    saveStateModelReset();

    void    onReadyForSaveState();
    void    onUnReadyForSaveState();

    void on_upSavePushButton_clicked();

    void on_downSavePushButton_clicked();

    void on_loadStatePushButton_clicked();

    void on_deleteSavePushButton_clicked();

    void on_renameSavePushButton_clicked();

    void on_savestateListView_doubleClicked(const QModelIndex &index);

    void on_saveSaveStatePushButton_clicked();

    void on_pathPushButton_clicked();

    void on_editShortcutButton_clicked();

    void on_actionReload_last_savestate_triggered();

    void on_actionMake_a_savestate_triggered();

private:
    Ui::Savestate2snesw *ui;
    QStandardItemModel* saveStateModel;
    QStandardItemModel* repStateModel;
    QStandardItem*      currentRep;
    QMenu*              saveStateMenu;
    QMenu*              categoryMenu;
    QAction*            customAddCatAction;
    QModelIndex         indexCatUnderMenu;
    HandleStuff*        handleStuff;
    QStandardItem*      newSaveInserted;
    USB2snes*           usb2snes;
    QString             gamesFolder;
    QSettings*           m_settings;

    void    createMenus();
    void    loadGames();
    void    addSubName(QString name, QString parentPath);
    void    addState(QString name);
    void    turnSaveStateAction(bool on);
    void    setStateTitle(QStandardItem *cat);
    void    newSaveState(bool triggerSave);
    void    closeEvent(QCloseEvent *event);
};

#endif // SAVESTATE2SNESW_H
