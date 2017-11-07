#include <QDebug>

#include "handlestuff.h"

HandleStuff::HandleStuff()
{

}

QStringList HandleStuff::loadGames()
{
    QFileInfoList listDir = saveDirectory.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    qDebug() << "Loading games" << listDir.size();
    foreach(QFileInfo fi, listDir)
    {
        games << fi.baseName();
        qDebug() << fi.baseName();
    }
    return games;
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


QStandardItem *HandleStuff::loadCategories(QString game)
{
    qDebug() << "Loading category for " << game;
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
    return categories[game];
}

bool HandleStuff::addGame(QString newGame)
{
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
    QStandardItem*  item = categoriesByPath[category->data(MyRolePath).toString()];
    if (!saveStates.contains(item))
    {
        QDir dir(item->data(MyRolePath).toString());
        QFileInfoList fil = dir.entryInfoList(QDir::Files);
        foreach(QFileInfo fi, fil)
        {
            saveStates[item] << fi.baseName();
        }
    }
    return saveStates[item];
}

bool HandleStuff::addSaveState(QStandardItem *category, QString name)
{
    QStandardItem*  item = categoriesByPath[category->data(MyRolePath).toString()];
    return true;
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
