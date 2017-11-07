#ifndef HANDLESTUFF_H
#define HANDLESTUFF_H

#include <QString>
#include <QStandardItem>
#include <QDir>

#define MyRolePath Qt::UserRole + 1

class HandleStuff
{
public:
    HandleStuff();

    QStringList loadGames();
    QStandardItem*  loadCategories(QString game);
    bool    addGame(QString newGame);
    void    setSaveStateDir(QString dir);
    bool    addCategory(QStandardItem* newCategory, QStandardItem* parent);
    bool    addSubCategory(QStandardItem* newCategory, QStandardItem* parent);
    QStringList    loadSaveStates(QStandardItem* category);
    bool    addSaveState(QStandardItem  *category, QString name);
    bool    removeCategory(QStandardItem* category);

private:
    QDir                                    saveDirectory;
    QStringList                             games;
    QMap<QString, QStandardItem*>           categories;
    QMap<QString, QStandardItem*>           categoriesByPath;
    QMap<QStandardItem*, QStringList>       saveStates;
    QString                                 gameLoaded;

    void findCategory(QStandardItem *parent, QDir dir);
};

#endif // HANDLESTUFF_H
