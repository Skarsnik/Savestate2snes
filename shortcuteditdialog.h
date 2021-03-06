#ifndef SHORTCUTEDITDIALOG_H
#define SHORTCUTEDITDIALOG_H

#include <QAbstractButton>
#include <QDialog>

namespace Ui {
class ShortcutEditDialog;
}

class ShortcutEditDialog : public QDialog
{
    Q_OBJECT


public:
    explicit ShortcutEditDialog(QWidget *parent = 0);
    ShortcutEditDialog(QWidget *parent, quint16 save, quint16 load);
    ~ShortcutEditDialog();
    quint16    saveShortcut();
    quint16    loadShortcut();


private slots:
    void    onControllerGroupToggled(int id, bool checked);
    void    onChooseGroupToggled(QAbstractButton*button, bool toggled);

    void on_saveResetButton_clicked();

    void on_loadResetButton_clicked();

private:
    Ui::ShortcutEditDialog *ui;

    quint16 m_saveShortcut;
    quint16 m_loadShortcut;

    void    setControllerButtonStatus(quint16 shortcut);
    void setLabels();
};

#endif // SHORTCUTEDITDIALOG_H
