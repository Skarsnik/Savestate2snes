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
#include "savestate2snesw.h"
#include "ui_savestate2snesw.h"
#include <QtSerialPort/QSerialPortInfo>

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
    qDebug() << "Menu triggered" << indexCatUnderMenu.isValid();
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
     repStateModel->removeRow(indexCatUnderMenu.row(), indexCatUnderMenu.parent());
}

void Savestate2snesw::on_actionAddCategory_triggered()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Enter a name for the new category"), tr("Category name:"), QLineEdit::Normal, tr("New Category"), &ok);
    if (ok)
    {
        QStandardItem* parent = repStateModel->invisibleRootItem();
        if (indexCatUnderMenu.isValid())
        {
            QStandardItem* curItem = repStateModel->itemFromIndex(indexCatUnderMenu);
            if (curItem->parent() != NULL)
                parent = curItem->parent();
         }
        handleStuff.addCategory(new QStandardItem(text), parent);
    }
}

void Savestate2snesw::on_actionAddSubCategory_triggered()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Enter a name for the new category"), tr("Category name:"), QLineEdit::Normal, tr("New Category"), &ok);
    if (ok)
    {
        QStandardItem* curItem = repStateModel->itemFromIndex(indexCatUnderMenu);
        if (!handleStuff.addSubCategory(new QStandardItem(text), curItem))
            qCritical() << "Can't add a new sub category";
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
            qCritical() << "Can't add a new game";
        }
    }
}

void deepClone(QStandardItem* tocpy, QStandardItem *cpy)
{
    for (unsigned int i = 0; i < tocpy->rowCount(); i++)
    {
        QStandardItem* childcpy = tocpy->child(i)->clone();
        cpy->appendRow(childcpy);
        if (tocpy->child(i)->rowCount() != 0)
            deepClone(tocpy->child(i), childcpy);
    }
}

void Savestate2snesw::on_gameComboBox_currentIndexChanged(const QString &arg1)
{
    if (arg1.isEmpty())
        return;
    repStateModel->clear();
    saveStateModel->clear();
    QStandardItem* root = handleStuff.loadCategories(arg1);
    m_settings->setValue("lastGameLoaded", arg1);
    qDebug() << root;
    for (unsigned int i = 0; i < root->rowCount(); i++)
    {
        qDebug() << root->child(i)->text() << root->child(i);
        QStandardItem* childcpy = root->child(i)->clone();
        repStateModel->invisibleRootItem()->appendRow(childcpy);
        qDebug() << "child cpy" << childcpy->parent();
        deepClone(root->child(i), childcpy);
    }
    ui->categoryTreeView->expandAll();
}

void    Savestate2snesw::newSaveState(bool triggerSave)
{
    QStandardItem*  newSaveItem = new QStandardItem(tr("New Savestate"));
    QString name = newSaveItem->text();
    while (!saveStateModel->findItems(name).isEmpty())
        name += "_";
    newSaveItem->setText(name);
    saveStateModel->invisibleRootItem()->appendRow(newSaveItem);
    ui->savestateListView->setCurrentIndex(newSaveItem->index());
    qDebug() << newSaveItem->isEditable();
    handleStuff.addSaveState(newSaveItem->text(), triggerSave);
    ui->savestateListView->edit(newSaveItem->index());
    newSaveInserted = newSaveItem;
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
        handleStuff.loadSaveState(saveStateModel->itemFromIndex(cur)->text());
}


void Savestate2snesw::saveStateItemChanged(QStandardItem *item)
{
    static bool avoid_loop = false;
    qDebug() << item->text() << "renamed.";
    QString name = item->text();
    bool b = false;
    while (saveStateModel->findItems(name).size() > 1)
    {
        b = true;
        name += "_";
    }
    if (b && ! avoid_loop)
    {
      qDebug() << "Name collision";
      avoid_loop = true;
      item->setText(name);
    }
    avoid_loop = false;
    handleStuff.renameSaveState(item);
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
    setStateTitle(cat);
    saveStateModel->clear();
    QStringList saveList = handleStuff.loadSaveStates(cat);
    foreach (QString save, saveList)
    {
        saveStateModel->invisibleRootItem()->appendRow(new QStandardItem(save));
    }
}


void Savestate2snesw::onSaveStateDelegateDataCommited(QWidget *e)
{
    qDebug() << "Item edited" << saveStateModel->itemFromIndex(ui->savestateListView->currentIndex())->text();
}


void Savestate2snesw::onReadyForSaveState()
{
    ui->statusBar->showMessage(tr("USB2Snes is ready for savestates."));
    ui->addSaveStatePushButton->setEnabled(true);
    ui->loadStatePushButton->setEnabled(true);
    ui->saveSaveStatePushButton->setEnabled(true);
}

void Savestate2snesw::onUnReadyForSaveState()
{
    ui->statusBar->showMessage(tr("USB2Snes is not ready for savestates."));
    ui->addSaveStatePushButton->setEnabled(false);
    ui->loadStatePushButton->setEnabled(false);
    ui->saveSaveStatePushButton->setEnabled(false);
}


void Savestate2snesw::on_upSavePushButton_clicked()
{
    if (!ui->savestateListView->currentIndex().isValid())
        return;
    qDebug() << "item move up" << ui->savestateListView->currentIndex();
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
    qDebug() << "item move down" << ui->savestateListView->currentIndex();
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
    qDebug() << "deleting  " << ui->savestateListView->currentIndex();
    int row = ui->savestateListView->currentIndex().row();
    QList<QStandardItem*> lItem = saveStateModel->takeRow(row);
    delete lItem.at(0);
    handleStuff.deleteSaveState(row);
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
    qDebug() << "Choosen file : " << gamesFolder;
    handleStuff.setSaveStateDir(gamesFolder);
    ui->pathLineEdit->setText(gamesFolder);
    loadGames();
}
