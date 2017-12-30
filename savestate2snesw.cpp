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



#include <QApplication>
#include <QInputDialog>
#include <QDebug>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include "savestate2snesw.h"
#include "shortcuteditdialog.h"
#include "ui_savestate2snesw.h"

Q_LOGGING_CATEGORY(log_MainUI, "MainUI")

#define sDebug() qCDebug(log_MainUI)

// < > : " / \ | ? *

Savestate2snesw::Savestate2snesw(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Savestate2snesw)
{
    ui->setupUi(this);

    m_settings = new QSettings("skarsnik.nyo.fr", "SaveState2SNES");
    if (m_settings->contains("windowGeometry"))
    {
        restoreGeometry(m_settings->value("windowGeometry").toByteArray());
        restoreState(m_settings->value("windowState").toByteArray());
    }

    ui->pathPushButton->setIcon(style()->standardPixmap(QStyle::SP_DialogOpenButton));
    ui->newGamePushButton->setIcon(style()->standardPixmap(QStyle::SP_FileIcon));
    ui->upSavePushButton->setIcon(style()->standardPixmap(QStyle::SP_ArrowUp));
    ui->downSavePushButton->setIcon(style()->standardPixmap(QStyle::SP_ArrowDown));
    ui->deleteSavePushButton->setIcon(style()->standardPixmap(QStyle::SP_TrashIcon));

    /*QRegExp *fileValidator = new QRegExp("(^<>:\\/\\\\|\\*\\?\")");
    ui->savestateListView->setV*/

    if (m_settings->contains("lastSaveStateDir"))
        gamesFolder = m_settings->value("lastSaveStateDir").toString();
    else
    {
        gamesFolder = QFileDialog::getExistingDirectory(this, tr("Choose Savestatedir"),
                                                        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), QFileDialog::ShowDirsOnly);
    }
    handleStuff.setSaveStateDir(gamesFolder);
    ui->pathLineEdit->setText(gamesFolder);

    ui->categoryTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->savestateListView->setContextMenuPolicy(Qt::CustomContextMenu);
    saveStateModel = new QStandardItemModel();
    repStateModel = new QStandardItemModel();
    ui->savestateListView->setModel(saveStateModel);
    ui->categoryTreeView->setModel(repStateModel);
    newSaveInserted = NULL;
    usb2snes = new USB2snes();
    handleStuff.setUsb2snes(usb2snes);
    ui->usb2snesStatut->setUsb2snes(usb2snes);

    createMenus();

    connect(ui->categoryTreeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(categoryListShowContextMenu(QPoint)));
    connect(saveStateModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(saveStateItemChanged(QStandardItem*)));
    //connect(saveStateModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(onSaveStateModelDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(ui->savestateListView->itemDelegate(), SIGNAL(commitData(QWidget*)), this, SLOT(onSaveStateDelegateDataCommited(QWidget*)));
    connect(ui->usb2snesStatut, SIGNAL(readyForSaveState()), this, SLOT(onReadyForSaveState()));
    connect(ui->usb2snesStatut, SIGNAL(unReadyForSaveState()), this, SLOT(onUnReadyForSaveState()));
    loadGames();

    usb2snes->connect();
}

void Savestate2snesw::loadGames()
{
    sDebug() << "Loading games";
    QStringList games = handleStuff.loadGames();
    if (games.size() != 0)
    {
        ui->gameComboBox->clear();
        foreach(QString game, games)
        {
            ui->gameComboBox->addItem(game);
        }
        ui->gameComboBox->model()->sort(0);
        if (m_settings->contains("lastGameLoaded"))
        {
            int index = ui->gameComboBox->findText(m_settings->value("lastGameLoaded").toString());
            if (index != -1)
            {
              ui->gameComboBox->setCurrentIndex(index);
            }
        } else {
            ui->gameComboBox->setCurrentIndex(0);
        }
    } else {
        ui->statusBar->showMessage("No game found into : " + gamesFolder);
    }
}



Savestate2snesw::~Savestate2snesw()
{
    delete ui;
}

void Savestate2snesw::saveListShowContextMenu(QPoint point)
{
    saveStateMenu->exec(ui->savestateListView->mapToGlobal(point));
}

void Savestate2snesw::categoryListShowContextMenu(QPoint point)
{
    indexCatUnderMenu = ui->categoryTreeView->indexAt(point);
    sDebug() << "Menu triggered" << indexCatUnderMenu.isValid();
    if (indexCatUnderMenu.isValid()) {
        QStandardItem* item = repStateModel->itemFromIndex(indexCatUnderMenu);
        categoryMenu->addSection(item->text());
        categoryMenu->addAction(ui->actionAddSubCategory);
        categoryMenu->addAction(ui->actionRemoveCategory);
    } else {
        categoryMenu->removeAction(ui->actionAddSubCategory);
        categoryMenu->removeAction(ui->actionRemoveCategory);
    }
    categoryMenu->exec(ui->categoryTreeView->mapToGlobal(point));
}

void Savestate2snesw::createMenus()
{
    saveStateMenu = new QMenu();
    categoryMenu = new QMenu();
    saveStateMenu->addAction(ui->actionRemoveSavestate);

    categoryMenu->addAction(ui->actionRemoveCategory);
    categoryMenu->addAction(ui->actionAddSubCategory);
    categoryMenu->addAction(ui->actionAddCategory);
}


void Savestate2snesw::on_actionRemoveCategory_triggered()
{
    sDebug() << "Removing a category";
    if (handleStuff.removeCategory(repStateModel->itemFromIndex(indexCatUnderMenu)->data(MyRolePath).toString()))
        repStateModel->removeRow(indexCatUnderMenu.row(), indexCatUnderMenu.parent());
}

void Savestate2snesw::on_actionAddCategory_triggered()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Enter a name for the new category"), tr("Category name:"), QLineEdit::Normal, tr("New Category"), &ok);
    if (ok)
    {
        sDebug() << "Adding a category";
        QStandardItem* parent = repStateModel->invisibleRootItem();
        if (indexCatUnderMenu.isValid())
        {
            QStandardItem* curItem = repStateModel->itemFromIndex(indexCatUnderMenu);
            if (curItem->parent() != NULL)
                parent = curItem->parent();
        }
        QString parentPath;
        if (parent != repStateModel->invisibleRootItem())
            parentPath = parent->data(MyRolePath).toString();
        Category* newCat = handleStuff.addCategory(text, parentPath);
        if (newCat != NULL)
        {
            QStandardItem*  newItem = new QStandardItem(newCat->name);
            newItem->setData(newCat->path, MyRolePath);
            parent->appendRow(newItem);
        } else {
                QMessageBox::warning(this, tr("Error adding a category"), QString(tr("Something failed while trying to add the category : %1")).arg(text));
        }
     }
}

void Savestate2snesw::on_actionAddSubCategory_triggered()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Enter a name for the new category"), tr("Category name:"), QLineEdit::Normal, tr("New Category"), &ok);
    if (ok)
    {
        sDebug() << "Adding a sub category";
        QStandardItem* curItem = repStateModel->itemFromIndex(indexCatUnderMenu);
        Category* newCat = handleStuff.addCategory(text, curItem->data(MyRolePath).toString());
        if (newCat == NULL)
        {
            QMessageBox::warning(this, tr("Error adding a sub category"), QString(tr("Something failed while trying to add the sub category : %1")).arg(text));
            return ;
        }
        QStandardItem* newItem = new QStandardItem(text);
        newItem->setData(newCat->path, MyRolePath);
        curItem->appendRow(newItem);
        ui->categoryTreeView->setExpanded(curItem->index(), true);
    }
}

void Savestate2snesw::on_newGamePushButton_clicked()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Enter a new game"), tr("Game name:"), QLineEdit::Normal, tr("New game"), &ok);
    if (ok)
    {
        if (handleStuff.addGame(text))
        {
            ui->gameComboBox->addItem(text);
        } else {
            qCCritical(log_MainUI()) << "Can't add a new game";
        }
    }
}

void createChildItems(QVector<Category*> cats, QStandardItem *parent)
{
    foreach (Category* cat, cats)
    {
        QStandardItem *newItem = new QStandardItem(cat->name);
        newItem->setData(cat->path, MyRolePath);
        parent->appendRow(newItem);
        createChildItems(cat->children, newItem);
    }
}

void Savestate2snesw::on_gameComboBox_currentIndexChanged(const QString &arg1)
{
    if (arg1.isEmpty())
        return;
    sDebug() << "Selected game changed";
    repStateModel->clear();
    saveStateModel->clear();
    QVector<Category*> categories = handleStuff.loadCategories(arg1);
    if (categories.isEmpty())
        return;
    m_settings->setValue("lastGameLoaded", arg1);
    foreach(Category* cat, categories)
    {
        QStandardItem *newItem = new QStandardItem(cat->name);
        newItem->setData(cat->path, MyRolePath);
        repStateModel->invisibleRootItem()->appendRow(newItem);
        createChildItems(cat->children, newItem);
    }
    ui->categoryTreeView->expandAll();
}

void    Savestate2snesw::newSaveState(bool triggerSave)
{
    sDebug() << "New Savestate, trigger : " << triggerSave;
    QStandardItem*  newSaveItem = new QStandardItem(tr("New Savestate"));
    QString name = newSaveItem->text();
    while (!saveStateModel->findItems(name).isEmpty())
        name += "_";
    newSaveItem->setText(name);
    if (handleStuff.addSaveState(newSaveItem->text(), triggerSave))
    {
        saveStateModel->invisibleRootItem()->appendRow(newSaveItem);
        ui->savestateListView->setCurrentIndex(newSaveItem->index());
        qDebug() << newSaveItem->isEditable();
        ui->savestateListView->edit(newSaveItem->index());
        newSaveInserted = newSaveItem;
    } else {
        delete newSaveItem;
        QMessageBox::warning(this, tr("New savestate error"), QString(tr("Something failed when trying to save the new savestate : %1")).arg(newSaveItem->text()));
    }
}

void Savestate2snesw::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("windowState", saveState());
    m_settings->setValue("windowGeometry", saveGeometry());
    m_settings->setValue("lastSaveStateDir", gamesFolder);
}

void Savestate2snesw::on_addSaveStatePushButton_clicked()
{
    if (ui->categoryTreeView->currentIndex().isValid())
        newSaveState(true);
}

void Savestate2snesw::on_saveSaveStatePushButton_clicked()
{
    if (ui->categoryTreeView->currentIndex().isValid())
        newSaveState(false);
}


void Savestate2snesw::on_loadStatePushButton_clicked()
{
    QModelIndex cur = ui->savestateListView->currentIndex();
    if (cur.isValid())
    {
        sDebug() << "Loading savestate" << saveStateModel->itemFromIndex(cur)->text();
        handleStuff.loadSaveState(saveStateModel->itemFromIndex(cur)->text());
    }
}


void Savestate2snesw::saveStateItemChanged(QStandardItem *item)
{
    static bool avoid_loop = false;
    sDebug() << item->text() << "renamed.";
    QString name = item->text();
    bool b = false;
    while (saveStateModel->findItems(name).size() > 1)
    {
        b = true;
        name += "_";
    }
    if (b && ! avoid_loop)
    {
      sDebug() << "Name collision";
      avoid_loop = true;
      item->setText(name);
    }
    avoid_loop = false;
    handleStuff.renameSaveState(item->row(), item->text());
}

void    Savestate2snesw::setStateTitle(QStandardItem* cat)
{
    QString title = ui->gameComboBox->currentText();
    QString catStr;
    while (cat != NULL)
    {
        catStr = cat->text() + " " + catStr;
        qDebug() << catStr;
        cat = cat->parent();
    }
    ui->savestateTitleLabel->setText(title + " - " + catStr);
}

void Savestate2snesw::on_categoryTreeView_clicked(const QModelIndex &index)
{
    QStandardItem* cat = repStateModel->itemFromIndex(index);
    sDebug() << "Category " << cat->text() << " selected";
    setStateTitle(cat);
    saveStateModel->clear();
    QStringList saveList = handleStuff.loadSaveStates(cat->data(MyRolePath).toString());
    foreach (QString save, saveList)
    {
        saveStateModel->invisibleRootItem()->appendRow(new QStandardItem(save));
    }
}


void Savestate2snesw::onSaveStateDelegateDataCommited(QWidget *e)
{
    sDebug() << "Item edited" << saveStateModel->itemFromIndex(ui->savestateListView->currentIndex())->text();
}


void Savestate2snesw::onReadyForSaveState()
{
    sDebug() << "Ready for savestate";
    ui->statusBar->showMessage(tr("USB2Snes is ready for savestates."));
    ui->addSaveStatePushButton->setEnabled(true);
    ui->loadStatePushButton->setEnabled(true);
    ui->saveSaveStatePushButton->setEnabled(true);
    ui->editShortcutButton->setEnabled(true);
    if (handleStuff.gameInfos().saveShortcut != 0)
    {
        handleStuff.setShortcutSave(handleStuff.gameInfos().saveShortcut);
        handleStuff.setShortcutLoad(handleStuff.gameInfos().loadShortcut);
        ui->usb2snesStatut->refreshShortcuts();
    }
}

void Savestate2snesw::onUnReadyForSaveState()
{
    sDebug() << "Unready for savestate";
    ui->statusBar->showMessage(tr("USB2Snes is not ready for savestates."));
    ui->addSaveStatePushButton->setEnabled(false);
    ui->loadStatePushButton->setEnabled(false);
    ui->saveSaveStatePushButton->setEnabled(false);
    ui->editShortcutButton->setEnabled(false);
}


void Savestate2snesw::on_upSavePushButton_clicked()
{
    if (!ui->savestateListView->currentIndex().isValid())
        return;
    sDebug() << "item move up" << ui->savestateListView->currentIndex();
    int row = ui->savestateListView->currentIndex().row();
    if (row == 0)
        return ;
    QList<QStandardItem*> lItem = saveStateModel->takeRow(row);
    saveStateModel->insertRow(row - 1, lItem.at(0));
    ui->savestateListView->setCurrentIndex(saveStateModel->indexFromItem(lItem.at(0)));
    handleStuff.changeStateOrder(row, row - 1);
}

void Savestate2snesw::on_downSavePushButton_clicked()
{
    if (!ui->savestateListView->currentIndex().isValid())
        return;
    sDebug() << "item move down" << ui->savestateListView->currentIndex();
    int row = ui->savestateListView->currentIndex().row();
    if (row == saveStateModel->rowCount() - 1)
        return ;
    QList<QStandardItem*> lItem = saveStateModel->takeRow(row);
    saveStateModel->insertRow(row + 1, lItem.at(0));
    ui->savestateListView->setCurrentIndex(saveStateModel->indexFromItem(lItem.at(0)));
    handleStuff.changeStateOrder(row, row + 1);
}


void Savestate2snesw::on_deleteSavePushButton_clicked()
{
    if (!ui->savestateListView->currentIndex().isValid())
        return;
    sDebug() << "deleting  " << ui->savestateListView->currentIndex();
    int row = ui->savestateListView->currentIndex().row();
    if (handleStuff.deleteSaveState(row))
    {
        QList<QStandardItem*> lItem = saveStateModel->takeRow(row);
        delete lItem.at(0);
    } else {
        QMessageBox::warning(this, tr("Removing savestate error"), QString(tr("Something failed when deleting : %1")).arg(saveStateModel->item(row)->text()));
    }
}

void Savestate2snesw::on_renameSavePushButton_clicked()
{
    if (!ui->savestateListView->currentIndex().isValid())
        return;
    ui->savestateListView->edit(ui->savestateListView->currentIndex());
}

void Savestate2snesw::on_savestateListView_doubleClicked(const QModelIndex &index)
{
    on_loadStatePushButton_clicked();
}


void Savestate2snesw::on_pathPushButton_clicked()
{
    gamesFolder = QFileDialog::getExistingDirectory(this, tr("Choose Games Directory"), gamesFolder, QFileDialog::ShowDirsOnly);
    sDebug() << "Choosen folder for savestates : " << gamesFolder;
    handleStuff.setSaveStateDir(gamesFolder);
    ui->pathLineEdit->setText(gamesFolder);
    loadGames();
}

void Savestate2snesw::on_editShortcutButton_clicked()
{
    ShortcutEditDialog  diag(this, handleStuff.shortcutSave(), handleStuff.shortcutLoad());
    if (diag.exec())
    {
        quint16 save = diag.saveShortcut();
        quint16 load = diag.loadShortcut();
        sDebug() << "Setting shortcuts s/l : " << QString::number(save, 16) << QString::number(load, 16);
        handleStuff.setShortcutSave(save);
        handleStuff.setShortcutLoad(load);
        ui->usb2snesStatut->refreshShortcuts();
        handleStuff.setGameShortCut(save, load);
    }
}
