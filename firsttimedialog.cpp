#include "firsttimedialog.h"
#include "ui_firsttimedialog.h"

#include <QFileDialog>
#include <QStandardPaths>
#include <QStyle>

FirstTimeDialog::FirstTimeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FirstTimeDialog)
{
    ui->setupUi(this);
    savePath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + "Savestates";
    ui->directoryPathLabel->setText(savePath);
    ui->pushButton->setIcon(style()->standardPixmap(QStyle::SP_DialogOpenButton));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    pathChanged = false;
}

FirstTimeDialog::~FirstTimeDialog()
{
    delete ui;
}

void FirstTimeDialog::on_pushButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Savestates directory"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), QFileDialog::ShowDirsOnly);
    if (dir.isEmpty())
        return;
    pathChanged = true;
    savePath = dir;
    ui->directoryPathLabel->setText(dir);
}

void FirstTimeDialog::on_buttonBox_accepted()
{
    if (!pathChanged)
        QDir("/").mkdir(savePath);
}

void FirstTimeDialog::on_versionsCheckBox_toggled(bool checked)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(checked);
}
