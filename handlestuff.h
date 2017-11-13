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
    bool    addSaveState(QString name);
    bool    removeCategory(QStandardItem* category);

private:
    QDir                                    saveDirectory;
    QStringList                             games;
    QMap<QString, QStandardItem*>           categories;
    QMap<QString, QStandardItem*>           categoriesByPath;
    QMap<QString, QStringList>              saveStates;
    QString                                 gameLoaded;
    QStandardItem*                          catLoaded;

    void findCategory(QStandardItem *parent, QDir dir);
    QStringList getCacheOrderList(QString file, QString dirPath);
    void        writeCacheOrderFile(QString file, QString dirPath);
};

#endif // HANDLESTUFF_H
