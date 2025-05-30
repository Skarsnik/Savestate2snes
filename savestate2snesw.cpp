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

QStandardItem*   findCatItemPath(QStandardItem* item, QString toFind);

Savestate2snesw::Savestate2snesw(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Savestate2snesw)
{
    ui->setupUi(this);
    sDebug() << "Savestate2snes  " << qApp->applicationVersion();
    invalidDirRegex = QRegExp("[\\\\\\/<>\\:\\\"\\|\\?\\*\\.]");
    invalidFileRegex = QRegExp("[\\\\\\/<>\\:\\\"\\|\\?\\*\\.]");
    //qDebug() << invalidFileRegex.indexIn("\\") << invalidFileRegex.indexIn("/");

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
    ui->pathLineEdit->setText(gamesFolder);

    ui->categoryTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->savestateListView->setContextMenuPolicy(Qt::CustomContextMenu);
    saveStateModel = new QStandardItemModel();
    repStateModel = new QStandardItemModel();
    ui->savestateListView->setModel(saveStateModel);
    ui->categoryTreeView->setModel(repStateModel);
    newSaveInserted = nullptr;
    createMenus();
    turnSaveStateAction(false);

    connect(ui->categoryTreeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(categoryListShowContextMenu(QPoint)));
    connect(saveStateModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(saveStateItemChanged(QStandardItem*)));
    connect(saveStateModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(saveStateRowRemoved(QModelIndex, int, int)));
    connect(saveStateModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(saveStateRowInserted(QModelIndex, int, int)));
    connect(saveStateModel, SIGNAL(modelReset()), this, SLOT(saveStateModelReset()));
    //connect(saveStateModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(onSaveStateModelDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(ui->savestateListView->itemDelegate(), SIGNAL(commitData(QWidget*)), this, SLOT(onSaveStateDelegateDataCommited(QWidget*)));
    connect(ui->consoleSwitcher, SIGNAL(readyForSaveState()), this, SLOT(onReadyForSaveState()));
    connect(ui->consoleSwitcher, SIGNAL(unReadyForSaveState()), this, SLOT(onUnReadyForSaveState()));
    connect(ui->consoleSwitcher, SIGNAL(modeChanged(ConsoleSwitcher::Mode)), this, SLOT(onModeChanged(ConsoleSwitcher::Mode)));
    connect(ui->consoleSwitcher, &ConsoleSwitcher::readyForSaveState, ui->trainingTimer, &TrainingTimer::saveStateReady);
    connect(ui->consoleSwitcher, &ConsoleSwitcher::unReadyForSaveState, ui->trainingTimer, &TrainingTimer::saveStateUnready);
    setWindowTitle(qApp->applicationName() + " - " + qApp->applicationVersion() + " - Multi");

    // Actions
    ui->actionMake_a_savestate->setEnabled(false);
    ui->actionReload_last_savestate->setEnabled(false);
    ui->actionSave_a_savestate->setEnabled(false);
    addAction(ui->actionReload_last_savestate);
    addAction(ui->actionMake_a_savestate);
    addAction(ui->actionSave_a_savestate);

    ui->consoleSwitcher->start();
    ui->statusBar->showMessage(ui->consoleSwitcher->unreadyString());
    ui->trainingTimer->loadPreset(qApp->applicationDirPath() + "/memorypreset.json");
    changeState(NONE);
    QTimer::singleShot(0, this, &Savestate2snesw::init);
}

void Savestate2snesw::init()
{
    onModeChanged(ui->consoleSwitcher->mode());
    handleStuff = ui->consoleSwitcher->getHandle();
    ui->consoleSwitcher->start();
    ui->statusBar->showMessage(ui->consoleSwitcher->unreadyString());
    if (m_settings->contains("lastCategoryLoaded"))
    {
        sDebug() << "Attempting to load the last categoryloaded" << m_settings->value("lastCategoryLoaded").toString();
        QStandardItem* repRoot = repStateModel->invisibleRootItem();
        QStandardItem* catFound = nullptr;

        if (repRoot != nullptr)
        {
            if (repRoot->hasChildren())
            {
                for (unsigned int i = 0; i < repRoot->rowCount(); i++)
                {
                    QStandardItem *child = repRoot->child(i);
                    if (child->data(MyRolePath).toString() == m_settings->value("lastCategoryLoaded").toString())
                    {
                        catFound = child;
                        break;
                    }
                    catFound = findCatItemPath(child, m_settings->value("lastCategoryLoaded").toString());
                }
            }
        }
        if (catFound != nullptr)
        {
            sDebug() << "Category found " << catFound->text();
            ui->categoryTreeView->expand(repStateModel->indexFromItem(catFound));
            ui->categoryTreeView->setCurrentIndex(catFound->index());
            loadCategory(catFound);
        }
    }
}

QStandardItem*   findCatItemPath(QStandardItem* item, QString toFind)
{
    if (item->hasChildren())
    {
        for (unsigned int i = 0; i < item->rowCount(); i++)
        {
            QStandardItem *child = item->child(i);
            if (child->data(MyRolePath).toString() == toFind)
            {
                return child;
            }
            return  findCatItemPath(child, toFind);
        }
    }
    return nullptr;
}


void Savestate2snesw::loadGames()
{
    sDebug() << "Loading games";
    QStringList games = handleStuff->loadGames();
    QString lastLoaded;
    ui->gameComboBox->clear();
    if (games.size() != 0)
    {
        if (m_settings->contains("lastGameLoaded"))
        {
             sDebug() << "LastGameLoaded was : " << m_settings->value("lastGameLoaded").toString();
            lastLoaded = m_settings->value("lastGameLoaded").toString();
        }
        foreach(QString game, games)
        {
            QIcon ico = handleStuff->getGameIcon(game);
            if (ico.isNull())
                ui->gameComboBox->addItem(game);
            else
                ui->gameComboBox->addItem(ico, game);
        }
        ui->gameComboBox->model()->sort(0);
        if (!lastLoaded.isEmpty())
        {

            int index = ui->gameComboBox->findText(lastLoaded);
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

void Savestate2snesw::turnSaveStateAction(bool on)
{
    foreach(QAbstractButton* but, ui->saveActionButtonGroup->buttons())
    {
        but->setEnabled(on);
    }
}



Savestate2snesw::~Savestate2snesw()
{
    delete ui;
}

void Savestate2snesw::onModeChanged(ConsoleSwitcher::Mode mode)
{
    sDebug() << "Mode changed" << mode;
    handleStuff = ui->consoleSwitcher->getHandle();
    ui->trainingTimer->setHandler(handleStuff);
    if (mode == ConsoleSwitcher::SNESClassic)
    {
        QString snescdir = gamesFolder + "/SNESClassic";
        if (!QFileInfo::exists(snescdir))
        {
            QDir newDir(gamesFolder);
            newDir.mkdir("SNESClassic");
        }
        handleStuff->setSaveStateDir(snescdir);
    }
    if (mode == ConsoleSwitcher::USB2Snes)
        handleStuff->setSaveStateDir(gamesFolder);
    if (mode == ConsoleSwitcher::NWAccess)
    {
        QString nwadir = gamesFolder + "/NWAccess";
        if (!QFileInfo::exists(nwadir))
        {
            QDir newDir(gamesFolder);
            newDir.mkdir("NWAccess");
        }
        handleStuff->setSaveStateDir(nwadir);
    }
    saveStateModel->clear();
    repStateModel->clear();
    loadGames();
    ui->savestateListView->setHandleStuff(handleStuff);
    connect(handleStuff, &HandleStuff::addSaveStateFinished, this, &Savestate2snesw::onSaveStateFinished, Qt::UniqueConnection);
    connect(handleStuff, &HandleStuff::loadSaveStateFinished, this, &Savestate2snesw::onLoadStateFinished, Qt::UniqueConnection);
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
    categoryMenu->addAction(ui->actionCategorySetIcon);
}


void Savestate2snesw::on_actionRemoveCategory_triggered()
{

    QStandardItem *item = repStateModel->itemFromIndex(indexCatUnderMenu);
    sDebug() << "Removing a category " << item->text();
    if (item->rowCount() > 0)
    {
        if (QMessageBox::question(this, tr("Removing a category"),
                                  QString(tr("You are about to remove the %1 category that include subcategories. Do you really want to proceed?")).arg(item->text())) != QMessageBox::Yes)
            return ;
    }
    if (handleStuff->removeCategory(item->data(MyRolePath).toString()))
    {
        repStateModel->removeRow(indexCatUnderMenu.row(), indexCatUnderMenu.parent());
        if (indexCatUnderMenu == ui->categoryTreeView->currentIndex())
        {
            saveStateModel->clear();
            turnSaveStateAction(false);
        }
    }
}

void Savestate2snesw::on_actionAddCategory_triggered()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("Enter a name for the new category"), tr("Category name:"), QLineEdit::Normal, tr("New Category"), &ok);
    if (ok)
    {
        while (invalidDirRegex.indexIn(text) != -1)
        {
            QMessageBox::information(this, tr("Error with category name"), tr("You entered an invalid character for a category name (< > : \" \/ \\ | ? * .)"));
            text = QInputDialog::getText(this, tr("Enter a name for the new category"), tr("Category name:"), QLineEdit::Normal, tr("New Category"), &ok);
            if (!ok)
                return ;
        }
        sDebug() << "Adding a category" << text;
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
        Category* newCat = handleStuff->addCategory(text, parentPath);
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
    QString text = QInputDialog::getText(this, tr("Enter a name for the new sub category"), tr("Category name:"), QLineEdit::Normal, tr("New Category"), &ok);
    if (ok)
    {
        while (invalidDirRegex.indexIn(text) != -1)
        {
            QMessageBox::information(this, tr("Error with category name"), tr("You entered an invalid character for a sub category name (< > : \" \/ \\ | ? * .)"));
            text = QInputDialog::getText(this, tr("Enter a name for the new category"), tr("Category name:"), QLineEdit::Normal, tr("New Category"), &ok);
            if (!ok)
                return ;
        }
        sDebug() << "Adding a sub category";
        QStandardItem* curItem = repStateModel->itemFromIndex(indexCatUnderMenu);
        Category* newCat = handleStuff->addCategory(text, curItem->data(MyRolePath).toString());
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
        if (handleStuff->addGame(text))
        {
            ui->gameComboBox->addItem(text);
            ui->gameComboBox->setCurrentText(text);
        } else {
            qFatal("Can't add a new game");
        }
    }
}

void createChildItems(QVector<Category*> cats, QStandardItem *parent)
{
    foreach (Category* cat, cats)
    {
        QStandardItem *newItem = new QStandardItem(cat->name);
        if (!cat->icon.isNull())
            newItem->setIcon(cat->icon);
        newItem->setData(cat->path, MyRolePath);
        parent->appendRow(newItem);
        createChildItems(cat->children, newItem);
    }
}

void Savestate2snesw::on_gameComboBox_currentIndexChanged(const QString &arg1)
{
    if (arg1.isEmpty())
        return;
    sDebug() << "Selected game changed" << arg1;
    repStateModel->clear();
    saveStateModel->clear();
    QVector<Category*> categories = handleStuff->loadCategories(arg1);
    if (categories.isEmpty())
        return;
    m_settings->setValue("lastGameLoaded", arg1);
    sDebug() << "lastGameLoaded value is now : " << m_settings->value("lastGameLoaded").toString();
    foreach(Category* cat, categories)
    {
        QStandardItem *newItem = new QStandardItem(cat->name);
        if (!cat->icon.isNull())
            newItem->setIcon(cat->icon);
        newItem->setData(cat->path, MyRolePath);
        repStateModel->invisibleRootItem()->appendRow(newItem);
        createChildItems(cat->children, newItem);
    }
    ui->categoryTreeView->expandAll();
}

void    Savestate2snesw::newSaveState(bool triggerSave)
{
    sDebug() << "New Savestate, trigger : " << triggerSave;
    if (state != READY)
        return ;
    changeState(SAVING_SAVESTATE);
    newSaveStateNameRequested = tr("New Savestate");
    while (!saveStateModel->findItems(newSaveStateNameRequested).isEmpty())
        newSaveStateNameRequested += "_";
    handleStuff->addSaveState(newSaveStateNameRequested, triggerSave);
}

void Savestate2snesw::onSaveStateFinished(bool success)
{
    if (success)
    {
        QStandardItem*  newSaveItem = new QStandardItem(newSaveStateNameRequested);
        QString name = newSaveItem->text();
        saveStateModel->invisibleRootItem()->appendRow(newSaveItem);
        ui->savestateListView->setCurrentIndex(newSaveItem->index());
        sDebug() << newSaveItem->isEditable();
        ui->savestateListView->edit(newSaveItem->index());
        newSaveInserted = newSaveItem;
    } else {
        QMessageBox::warning(this, tr("New savestate error"), QString(tr("Something failed when trying to save the new savestate : %1")).arg(newSaveStateNameRequested));
    }
    changeState(READY);
}

void Savestate2snesw::onLoadStateFinished(bool success)
{
    changeState(READY);
}


void Savestate2snesw::changeState(State m_state)
{
    sDebug() << "State changed" << m_state;
    state = m_state;
}

void Savestate2snesw::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    //usb2snes->close();
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
    if (cur.isValid() && state == READY)
    {
        sDebug() << "Loading savestate" << saveStateModel->itemFromIndex(cur)->text();
        changeState(LOADING_SAVESTATE);
        handleStuff->loadSaveState(saveStateModel->itemFromIndex(cur)->text());
    }
}


void Savestate2snesw::saveStateItemChanged(QStandardItem *item)
{
    static bool avoid_loop = false;
    sDebug() << item->text() << "renamed.";
    QString name = item->text();
    int posInvalid = invalidFileRegex.indexIn(name);
    if (posInvalid != -1)
    {
        QMessageBox::warning(this, tr("Invalid character"), tr("The savestate name contains an invalid character (< > : \" / \\ | ? *). <br/> It get replaced by _"));
        while (posInvalid != -1)
        {
            name[posInvalid] = '_';
            posInvalid = invalidFileRegex.indexIn(name);
        }
        item->setText(name);
        name = item->text();
    }
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
    handleStuff->renameSaveState(item->row(), item->text());
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
    loadCategory(cat);
}

void    Savestate2snesw::loadCategory(QStandardItem* cat)
{
    sDebug() << "Category " << cat->text() << " selected";
    m_settings->setValue("lastCategoryLoaded", cat->data(MyRolePath).toString());
    setStateTitle(cat);
    saveStateModel->clear();
    QStringList saveList = handleStuff->loadSaveStates(cat->data(MyRolePath).toString());
    foreach (QString save, saveList)
    {
        saveStateModel->invisibleRootItem()->appendRow(new QStandardItem(save));
    }
    uiReadyForSavestate = true;
    if (backendSaveStateReady == true && state != READY)
        onReadyForSaveState();
}

void Savestate2snesw::onSaveStateDelegateDataCommited(QWidget *e)
{
    Q_UNUSED(e)
    sDebug() << "Item edited" << saveStateModel->itemFromIndex(ui->savestateListView->currentIndex())->text();
}

void Savestate2snesw::saveStateRowRemoved(QModelIndex p, int begin, int last)
{
    Q_UNUSED(p) Q_UNUSED(begin) Q_UNUSED(last)
    if (saveStateModel->rowCount() == 0)
        turnSaveStateAction(false);
}

void Savestate2snesw::saveStateRowInserted(QModelIndex p, int begin, int last)
{
    Q_UNUSED(p) Q_UNUSED(begin) Q_UNUSED(last)
    if (!ui->renameSavePushButton->isEnabled())
            turnSaveStateAction(true);
}

void Savestate2snesw::saveStateModelReset()
{
    turnSaveStateAction(false);
}


void Savestate2snesw::onReadyForSaveState()
{
    backendSaveStateReady = true;
    if (uiReadyForSavestate == false)
        return ;
    sDebug() << "Ready for savestate";
    ui->actionMake_a_savestate->setEnabled(true);
    ui->actionReload_last_savestate->setEnabled(true);
    ui->actionSave_a_savestate->setEnabled(true);
    ui->statusBar->showMessage(ui->consoleSwitcher->readyString());
    ui->addSaveStatePushButton->setEnabled(true);
    ui->loadStatePushButton->setEnabled(true);
    ui->saveSaveStatePushButton->setEnabled(true);
    changeState(READY);
    if (handleStuff->hasShortcutsEdit())
    {
        ui->editShortcutButton->setEnabled(true);
        if (handleStuff->gameInfos().saveShortcut != 0)
        {
            handleStuff->setShortcutSave(handleStuff->gameInfos().saveShortcut);
            handleStuff->setShortcutLoad(handleStuff->gameInfos().loadShortcut);
            ui->consoleSwitcher->refreshShortcuts();
        }
    }
    ui->trainingTimer->setEnabled(true);
    handleStuff->savestateReady();
}

void Savestate2snesw::onUnReadyForSaveState()
{
    backendSaveStateReady = false;
    sDebug() << "Unready for savestate";
    ui->actionMake_a_savestate->setEnabled(false);
    ui->actionReload_last_savestate->setEnabled(false);
    ui->actionSave_a_savestate->setEnabled(false);
    ui->statusBar->showMessage(ui->consoleSwitcher->unreadyString());
    ui->addSaveStatePushButton->setEnabled(false);
    ui->loadStatePushButton->setEnabled(false);
    ui->saveSaveStatePushButton->setEnabled(false);
    if (handleStuff->hasShortcutsEdit())
        ui->editShortcutButton->setEnabled(false);
    ui->trainingTimer->setEnabled(false);
    handleStuff->savestateUnready();
    changeState(NONE);
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
    handleStuff->changeStateOrder(row, row - 1);
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
    handleStuff->changeStateOrder(row, row + 1);
}


void Savestate2snesw::on_deleteSavePushButton_clicked()
{
    if (!ui->savestateListView->currentIndex().isValid())
        return;
    sDebug() << "deleting  " << ui->savestateListView->currentIndex();
    int row = ui->savestateListView->currentIndex().row();
    if (handleStuff->deleteSaveState(row))
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
    Q_UNUSED(index)
    if (ui->loadStatePushButton->isEnabled())
        on_loadStatePushButton_clicked();
}


void Savestate2snesw::on_pathPushButton_clicked()
{
    gamesFolder = QFileDialog::getExistingDirectory(this, tr("Choose Games Directory"), gamesFolder, QFileDialog::ShowDirsOnly);
    if (gamesFolder.isEmpty())
        return ;
    sDebug() << "Choosen folder for savestates : " << gamesFolder;
    handleStuff->setSaveStateDir(gamesFolder);
    ui->pathLineEdit->setText(gamesFolder);
    loadGames();
}

void Savestate2snesw::on_editShortcutButton_clicked()
{
    if (!handleStuff->hasShortcutsEdit())
        return ;
    ShortcutEditDialog  diag(this, handleStuff->shortcutSave(), handleStuff->shortcutLoad());
    if (diag.exec())
    {
        quint16 save = diag.saveShortcut();
        quint16 load = diag.loadShortcut();
        sDebug() << "Setting shortcuts s/l : " << QString::number(save, 16) << QString::number(load, 16);
        handleStuff->setShortcutSave(save);
        handleStuff->setShortcutLoad(load);
        ui->consoleSwitcher->refreshShortcuts();
        handleStuff->setGameShortCut(save, load);
    }
}

void Savestate2snesw::on_actionReload_last_savestate_triggered()
{
    if (ui->loadStatePushButton->isEnabled())
        on_loadStatePushButton_clicked();
}

void Savestate2snesw::on_actionMake_a_savestate_triggered()
{
    if (ui->addSaveStatePushButton->isEnabled())
        on_addSaveStatePushButton_clicked();
}

void Savestate2snesw::on_actionSave_a_savestate_triggered()
{
    if (ui->saveSaveStatePushButton->isEnabled())
        on_saveSaveStatePushButton_clicked();
}

void Savestate2snesw::on_actionCategorySetIcon_triggered()
{
    const   QString& catName = repStateModel->itemFromIndex(ui->categoryTreeView->currentIndex())->text();
    const   QString& catPath = repStateModel->itemFromIndex(ui->categoryTreeView->currentIndex())->data(MyRolePath).toString();
    QString fileName = QFileDialog::getOpenFileName(this, QString(tr("Select an icon for %1")).arg(catName), QString(), tr("Image Files (*.png *.jpg *.bmp)"));
    if (fileName.isEmpty())
        return;
    QFileInfo fi(fileName);
    QString newName = catPath + "/icon." + fi.suffix();
    sDebug() << "Setting icon for cat : " << newName;
    QFile::copy(fileName, newName);
    handleStuff->setCategoryIcon(catPath, newName);
    repStateModel->itemFromIndex(ui->categoryTreeView->currentIndex())->setIcon(QIcon(newName));
}
