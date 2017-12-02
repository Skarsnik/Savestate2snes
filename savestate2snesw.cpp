#include <QApplication>
#include <QInputDialog>
#include <QDebug>
#include "savestate2snesw.h"
#include "ui_savestate2snesw.h"
#include <QtSerialPort/QSerialPortInfo>

Savestate2snesw::Savestate2snesw(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Savestate2snesw)
{
    ui->setupUi(this);
    ui->pathPushButton->setIcon(style()->standardPixmap(QStyle::SP_DialogOpenButton));
    ui->newGamePushButton->setIcon(style()->standardPixmap(QStyle::SP_FileIcon));
    handleStuff.setSaveStateDir("D:\\Project\\Savestate2snes\\savestate");
    ui->pathLineEdit->setText("D:\\Project\\Savestate2snes\\savestate");

    ui->categoryTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->savestateListView->setContextMenuPolicy(Qt::CustomContextMenu);
    saveStateModel = new QStandardItemModel();
    repStateModel = new QStandardItemModel();
    ui->savestateListView->setModel(saveStateModel);
    ui->categoryTreeView->setModel(repStateModel);
    newSaveInserted = NULL;
    usb2snes = new USB2snes();

    createMenus();

    connect(ui->categoryTreeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(categoryListShowContextMenu(QPoint)));
    connect(saveStateModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(saveStateItemChanged(QStandardItem*)));
    //connect(saveStateModel, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), SLOT(onSaveStateModelDataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(ui->savestateListView->itemDelegate(), SIGNAL(commitData(QWidget*)), this, SLOT(onSaveStateDelegateDataCommited(QWidget*)));
    connect(usb2snes, SIGNAL(stateChanged()), this, SLOT(usb2snesStateChanged()));

    loadGames();

    usb2snes->connect();
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

void Savestate2snesw::loadGames()
{
    QStringList games = handleStuff.loadGames();
    if (games.size() != 0)
    {
        foreach(QString game, games)
        {
            ui->gameComboBox->addItem(game);
        }
        ui->gameComboBox->model()->sort(0);
        ui->gameComboBox->setCurrentIndex(0);
    }
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
    repStateModel->clear();
    QStandardItem* root = handleStuff.loadCategories(arg1);
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

void Savestate2snesw::on_addSaveStatePushButton_clicked()
{
    QStandardItem*  newSaveItem = new QStandardItem(tr("New Savestate"));
    saveStateModel->invisibleRootItem()->appendRow(newSaveItem);
    ui->savestateListView->setCurrentIndex(newSaveItem->index());
    qDebug() << newSaveItem->isEditable();
    ui->savestateListView->edit(newSaveItem->index());
    newSaveInserted = newSaveItem;
}

void Savestate2snesw::saveStateItemChanged(QStandardItem *item)
{
    if (newSaveInserted != NULL && item == newSaveInserted)
    {
        //handleStuff.addSaveState(item->text());
        qDebug() << item->text() << "renamed?";
        handleStuff.addSaveState(item->text());
        newSaveInserted = NULL;
    }
}

void Savestate2snesw::on_categoryTreeView_clicked(const QModelIndex &index)
{
    QStandardItem* cat = repStateModel->itemFromIndex(index);
    ui->savestateTitleLabel->setText(ui->gameComboBox->currentText() + " - " + cat->text());
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

void Savestate2snesw::usb2snesStateChanged()
{
    if (usb2snes->state() == USB2snes::Ready)
    {
        /*usb2snes->getAddress(0x7FC0, 21);
        usb2snes->getAddress(0xFFC0, 21);*/
        //usb2snes->getAddress(0xC0FFD5, 2);
        QString text = usb2snes->getRomName();
        qDebug() << text;
        ui->romNameLabel->setText(text);
        usb2snes->setAddress(0xFFC0, QByteArray("IL FAIT BEAU ICI"));
    }
}
