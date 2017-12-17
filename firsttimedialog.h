#ifndef FIRSTTIMEDIALOG_H
#define FIRSTTIMEDIALOG_H

#include <QDialog>

namespace Ui {
class FirstTimeDialog;
}

class FirstTimeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FirstTimeDialog(QWidget *parent = 0);
    ~FirstTimeDialog();
    QString savePath;

private slots:
    void on_pushButton_clicked();

    void on_buttonBox_accepted();

    void on_versionsCheckBox_toggled(bool checked);

private:
    Ui::FirstTimeDialog *ui;
    bool    pathChanged;
};

#endif // FIRSTTIMEDIALOG_H
